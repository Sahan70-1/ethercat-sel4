#define _POSIX_C_SOURCE 200809L
#include "soem/soem.h"
#include "ec_transport.h"
#include "ec_linux_transport.h"
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Internal SOEM entry point, intentionally exercised against real nicdrv.c. */
int ecx_inframe(ecx_portt *port, uint8 idx, int stacknumber);

typedef struct {
    unsigned char bytes[4][EC_BUFSIZE + 1];
    size_t length[4];
    unsigned head, tail, sends, waits;
    ec_io_result tx_result;
    unsigned respond_on_send; /* deterministic retry trigger; 0 = no response */
    unsigned short response_wkc;
} fake_link;

static void queue_frame(fake_link *mock, const void *frame, size_t length)
{
    assert(mock->tail < 4 && length <= EC_BUFSIZE + 1);
    memcpy(mock->bytes[mock->tail], frame, length);
    mock->length[mock->tail++] = length;
}

static ec_io_result fake_send(void *context, const void *frame, size_t length)
{
    fake_link *mock = context;
    mock->sends++;
    if (mock->tx_result != EC_IO_OK) return mock->tx_result;
    if (mock->respond_on_send && mock->sends == mock->respond_on_send) {
        unsigned char reply[EC_BUFSIZE];
        memcpy(reply, frame, length);
        reply[length - 2] = mock->response_wkc & 0xff;
        reply[length - 1] = mock->response_wkc >> 8;
        queue_frame(mock, reply, length);
    }
    return EC_IO_OK;
}

static ec_io_result fake_receive(void *context, void *frame, size_t capacity, size_t *length)
{
    fake_link *mock = context;
    if (mock->head == mock->tail) return EC_IO_AGAIN;
    size_t size = mock->length[mock->head];
    unsigned i = mock->head++;
    if (size > capacity) return EC_IO_INVALID;
    memcpy(frame, mock->bytes[i], size);
    *length = size;
    return EC_IO_OK;
}

static ec_io_result fake_now(void *context, uint64_t *time)
{
    (void)context;
    struct timespec ts;
    assert(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
    *time = (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
    return EC_IO_OK;
}

static ec_io_result fake_wait(void *context, uint64_t deadline)
{
    fake_link *mock = context;
    mock->waits++;
    if (mock->head < mock->tail) return EC_IO_OK;
    struct timespec ts = { .tv_sec = (time_t)(deadline / 1000000),
                           .tv_nsec = (long)(deadline % 1000000) * 1000 };
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
    return EC_IO_TIMEOUT;
}

static const ec_transport_ops fake_ops = { fake_send, fake_receive, fake_now, fake_wait };
static ecx_portt port;
static fake_link mock;

static void start(void)
{
    memset(&port, 0, sizeof(port));
    memset(&mock, 0, sizeof(mock));
    port.stack.txbuf = &port.txbuf;
    port.stack.txbuflength = &port.txbuflength;
    port.stack.tempbuf = &port.tempinbuf;
    port.stack.rxbuf = &port.rxbuf;
    port.stack.rxbufstat = &port.rxbufstat;
    port.stack.rxsa = &port.rxsa;
    port.stack.transport = (ec_transport){ &fake_ops, &mock, EC_BUFSIZE };
    assert(pthread_mutex_init(&port.getindex_mutex, NULL) == 0);
    assert(pthread_mutex_init(&port.tx_mutex, NULL) == 0);
    assert(pthread_mutex_init(&port.rx_mutex, NULL) == 0);
}

static void stop(void)
{
    assert(pthread_mutex_destroy(&port.getindex_mutex) == 0);
    assert(pthread_mutex_destroy(&port.tx_mutex) == 0);
    assert(pthread_mutex_destroy(&port.rx_mutex) == 0);
}

static uint8 request(void)
{
    uint8 idx = ecx_getindex(&port);
    uint16 data = 0;
    ec_setupheader(port.txbuf[idx]);
    ecx_setupdatagram(&port, port.txbuf[idx], EC_CMD_BRD, idx, 0, 0x0130, sizeof(data), &data);
    return idx;
}

int main(void)
{
    start();
    uint8 idx = request();
    assert(ecx_inframe(&port, idx, 0) == EC_NOFRAME);
    mock.respond_on_send = 1;
    mock.response_wkc = 0;
    assert(ecx_srconfirm(&port, idx, 10000) == 0);
    assert(mock.sends == 1 && port.rxbufstat[idx] == EC_BUF_COMPLETE);
    assert(mock.waits > 0);
    stop();
    puts("PASS actual SOEM accepts mock WKC=0 without retry; empty RX is NOFRAME");

    start();
    idx = request();
    mock.respond_on_send = 2;
    mock.response_wkc = 2;
    assert(ecx_srconfirm(&port, idx, 1000000) == 2);
    assert(mock.sends == 2 && mock.waits > 0);
    stop();
    puts("PASS actual SOEM retries unanswered request then accepts mock response");

    start();
    uint8 first = request(), second = request();
    assert(ecx_outframe(&port, first, 0) > 0);
    assert(ecx_outframe(&port, second, 0) > 0);
    unsigned char reply[EC_BUFSIZE];
    memcpy(reply, port.txbuf[second], port.txbuflength[second]);
    reply[port.txbuflength[second] - 2] = 2;
    queue_frame(&mock, reply, port.txbuflength[second]);
    assert(ecx_inframe(&port, first, 0) == EC_OTHERFRAME);
    assert(port.rxbufstat[second] == EC_BUF_RCVD);
    assert(ecx_inframe(&port, second, 0) == 2);
    memcpy(reply, port.txbuf[first], port.txbuflength[first]);
    reply[port.txbuflength[first] - 2] = 1;
    queue_frame(&mock, reply, port.txbuflength[first]);
    assert(ecx_inframe(&port, first, 0) == 1);
    stop();
    puts("PASS actual SOEM buffers out-of-order mock indexes");

    start();
    idx = request();
    assert(ecx_outframe(&port, idx, 0) > 0);
    memcpy(reply, port.txbuf[idx], port.txbuflength[idx]);
    queue_frame(&mock, reply, 13);
    assert(ecx_inframe(&port, idx, 0) == EC_NOFRAME);
    queue_frame(&mock, reply, (size_t)port.txbuflength[idx] - 1);
    assert(ecx_inframe(&port, idx, 0) == EC_OTHERFRAME);
    assert(port.rxbufstat[idx] == EC_BUF_TX);
    reply[14] = 0xff; reply[15] = 0x1f;
    queue_frame(&mock, reply, port.txbuflength[idx]);
    assert(ecx_inframe(&port, idx, 0) == EC_OTHERFRAME);
    assert(port.rxbufstat[idx] == EC_BUF_TX);
    stop();
    puts("PASS truncated RX and invalid WKC offset rejected before completion");

    start();
    idx = request();
    memcpy(reply, port.txbuf[idx], port.txbuflength[idx]);
    reply[17] = EC_MAXBUF - 1; /* no outstanding request for this index */
    queue_frame(&mock, reply, port.txbuflength[idx]);
    assert(ecx_inframe(&port, idx, 0) == EC_OTHERFRAME);
    assert(port.rxbufstat[EC_MAXBUF - 1] == EC_BUF_EMPTY);
    reply[12] = 0x08; reply[13] = 0x00;
    queue_frame(&mock, reply, port.txbuflength[idx]);
    assert(ecx_inframe(&port, idx, 0) == EC_OTHERFRAME);
    stop();
    puts("PASS unexpected index and non-EtherCAT frame not completed");

    start();
    idx = request();
    mock.tx_result = EC_IO_AGAIN;
    assert(ecx_outframe(&port, idx, 0) == -1);
    assert(port.rxbufstat[idx] == EC_BUF_EMPTY);
    mock.tx_result = EC_IO_ERROR;
    assert(ecx_outframe(&port, idx, 0) == -1);
    mock.tx_result = EC_IO_OK;
    assert(ecx_srconfirm(&port, idx, 1000) == EC_NOFRAME);
    assert(mock.sends >= 3); /* two failed sends plus at least one timeout attempt */
    stop();
    puts("PASS TX backpressure/error mapping and no-response deadline");

    start();
    for (int i = 0; i < 64; i++) {
        mock.head = mock.tail = mock.sends = 0;
        mock.respond_on_send = 1;
        mock.response_wkc = (unsigned short)(i + 1);
        idx = request();
        assert(ecx_srconfirm(&port, idx, 10000) == i + 1);
        ecx_setbufstat(&port, idx, EC_BUF_EMPTY);
    }
    stop();
    puts("PASS 64 SOEM index allocate/receive/release cycles");
    start();
    assert(ecx_outframe(&port, EC_MAXBUF, 0) == -1);
    assert(ecx_inframe(&port, EC_MAXBUF, 0) == EC_NOFRAME);
    assert(ecx_setupnic(&port, "unused", 1) == 0);
    stop();
    puts("PASS invalid caller index and unsupported secondary setup rejected");

    start();
    int fd[2];
    assert(socketpair(AF_UNIX, SOCK_DGRAM, 0, fd) == 0);
    ec_linux_transport_bind(&port.stack.transport, &fd[0], EC_BUFSIZE);
    idx = request();
    assert(ecx_outframe(&port, idx, 0) == port.txbuflength[idx]);
    ssize_t count = recv(fd[1], reply, sizeof(reply), MSG_DONTWAIT);
    assert(count == port.txbuflength[idx]);
    assert(memcmp(reply, port.txbuf[idx], (size_t)count) == 0);
    reply[count - 2] = 2;
    assert(send(fd[1], reply, (size_t)count, 0) == count);
    assert(ecx_waitinframe(&port, idx, 100000) == 2);
    close(fd[0]); close(fd[1]);
    stop();
    puts("PASS actual SOEM through Linux socketpair TX/RX/wait with synthetic peer");
    puts("All responses are synthetic; no real slaves or network NIC tested");
    return 0;
}
