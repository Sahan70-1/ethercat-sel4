#define _POSIX_C_SOURCE 200809L
#include "ec_linux_transport.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static volatile sig_atomic_t signals;
static void alarm_handler(int number) { (void)number; signals++; }

int main(void)
{
    int fd[2];
    assert(socketpair(AF_UNIX, SOCK_DGRAM, 0, fd) == 0);
    ec_transport tx, rx;
    ec_linux_transport_bind(&tx, &fd[0], 1518);
    ec_linux_transport_bind(&rx, &fd[1], 1518);
    unsigned char frame[1519], received[1519];
    memset(frame, 0x53, sizeof(frame));
    frame[12] = 0x88; frame[13] = 0xa4;
    size_t length = 99;
    uint64_t now, after;

    assert(ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_AGAIN);
    assert(length == 0);
    puts("PASS empty RX is AGAIN, not a frame/WKC");

    const size_t sizes[] = {14, 60, 1518};
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        assert(ec_transport_send(&tx, frame, sizes[i]) == EC_IO_OK);
        assert(ec_transport_now(&rx, &now) == EC_IO_OK);
        assert(ec_transport_wait(&rx, now + 100000) == EC_IO_OK);
        assert(ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_OK);
        assert(length == sizes[i] && memcmp(frame, received, length) == 0);
    }
    puts("PASS full frame lengths and byte-for-byte EtherType preservation");

    assert(ec_transport_send(&tx, frame, 0) == EC_IO_INVALID);
    assert(ec_transport_send(&tx, frame, 13) == EC_IO_INVALID);
    assert(ec_transport_send(&tx, frame, sizeof(frame)) == EC_IO_INVALID);
    assert(ec_transport_send(&tx, NULL, 60) == EC_IO_INVALID);
    assert(ec_transport_receive(&rx, received, 13, &length) == EC_IO_INVALID);
    assert(ec_transport_wait(NULL, 0) == EC_IO_INVALID);
    puts("PASS invalid TX sizes and arguments rejected");

    /* Bypass TX validation to inject malformed incoming datagrams. */
    const size_t bad_sizes[] = {0, 13, 1519};
    for (size_t i = 0; i < sizeof(bad_sizes)/sizeof(bad_sizes[0]); i++) {
        assert(send(fd[0], frame, bad_sizes[i], 0) == (ssize_t)bad_sizes[i]);
        assert(ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_INVALID);
        assert(length == 0);
        assert(ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_AGAIN);
    }
    assert(ec_transport_send(&tx, frame, 60) == EC_IO_OK);
    assert(ec_transport_receive(&rx, received, 14, &length) == EC_IO_INVALID);
    assert(length == 0);
    puts("PASS zero/short/oversized/truncated RX consumed and rejected");

    for (int i = 0; i < 256; i++) {
        frame[20] = (unsigned char)i;
        unsigned char expected[60];
        memcpy(expected, frame, sizeof(expected));
        assert(ec_transport_send(&tx, frame, 60) == EC_IO_OK);
        memset(frame + 21, i, 39); /* caller can immediately reuse TX memory */
        assert(ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_OK);
        assert(length == 60 && received[20] == (unsigned char)i);
        assert(memcmp(expected, received, sizeof(expected)) == 0);
        assert(ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_AGAIN);
    }
    puts("PASS 256 buffer reuse cycles without duplicate RX");

    int buffer_size = 4096;
    assert(setsockopt(fd[0], SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) == 0);
    int accepted = 0;
    ec_io_result result;
    do {
        result = ec_transport_send(&tx, frame, 60);
        if (result == EC_IO_OK) accepted++;
        assert(accepted < 100000);
    } while (result == EC_IO_OK);
    assert(result == EC_IO_AGAIN && accepted > 0);
    int drained = 0;
    while (ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_OK) drained++;
    assert(drained == accepted);
    assert(ec_transport_send(&tx, frame, 60) == EC_IO_OK);
    assert(ec_transport_receive(&rx, received, sizeof(received), &length) == EC_IO_OK);
    puts("PASS TX backpressure and recovery after drain");

    assert(ec_transport_now(&rx, &now) == EC_IO_OK);
    assert(ec_transport_wait(&rx, now) == EC_IO_TIMEOUT);
    assert(ec_transport_wait(&rx, now + 2000) == EC_IO_TIMEOUT);
    assert(ec_transport_now(&rx, &after) == EC_IO_OK && after >= now + 2000);
    puts("PASS expired/future monotonic deadlines");

    struct sigaction action = {0}, previous;
    action.sa_handler = alarm_handler;
    sigemptyset(&action.sa_mask);
    assert(sigaction(SIGALRM, &action, &previous) == 0);
    struct itimerval alarm = { .it_value = { .tv_usec = 1000 } };
    assert(ec_transport_now(&rx, &now) == EC_IO_OK);
    assert(setitimer(ITIMER_REAL, &alarm, NULL) == 0);
    assert(ec_transport_wait(&rx, now + 20000) == EC_IO_TIMEOUT);
    assert(ec_transport_now(&rx, &after) == EC_IO_OK && after >= now + 20000);
    assert(signals > 0);
    assert(sigaction(SIGALRM, &previous, NULL) == 0);
    puts("PASS interrupted wait retains original deadline");

    close(fd[0]);
    assert(ec_transport_send(&tx, frame, 60) == EC_IO_ERROR);
    assert(ec_transport_receive(&tx, received, sizeof(received), &length) == EC_IO_ERROR);
    assert(ec_transport_now(&tx, &now) == EC_IO_OK);
    assert(ec_transport_wait(&tx, now + 1000) == EC_IO_ERROR);
    fd[0] = -1;
    assert(ec_transport_wait(&tx, now + 1000) == EC_IO_ERROR);
    close(fd[1]);
    puts("PASS closed/invalid descriptor errors");
    return 0;
}
