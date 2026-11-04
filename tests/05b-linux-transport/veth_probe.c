/* One synthetic BRD exchange through the adapted SOEM library.
 * This is a transport probe, not slave discovery or an EtherCAT slave model.
 */
#include "soem/soem.h"
#include "ec_linux_transport.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ecx_portt port;

/* A supplied local datagram fd lets the same probe run in rootless tests.
 * Production veth use goes through the real ecx_setupnic path instead. */
static int setup_test_fd(const char *argument)
{
    char *end;
    errno = 0;
    long fd = strtol(argument, &end, 10);
    if (errno || end == argument || *end || fd < 0 || fd > INT_MAX) return 0;
    port.sockhandle = (int)fd;
    port.stack.sock = &port.sockhandle;
    port.stack.txbuf = &port.txbuf;
    port.stack.txbuflength = &port.txbuflength;
    port.stack.tempbuf = &port.tempinbuf;
    port.stack.rxbuf = &port.rxbuf;
    port.stack.rxbufstat = &port.rxbufstat;
    port.stack.rxsa = &port.rxsa;
    if (pthread_mutex_init(&port.getindex_mutex, NULL)) return 0;
    if (pthread_mutex_init(&port.tx_mutex, NULL)) return 0;
    if (pthread_mutex_init(&port.rx_mutex, NULL)) return 0;
    ec_linux_transport_bind(&port.stack.transport, &port.sockhandle, EC_BUFSIZE);
    return 1;
}

int main(int argc, char **argv)
{
    int ready;
    if (argc == 3 && strcmp(argv[1], "--test-fd") == 0) {
        ready = setup_test_fd(argv[2]);
    } else if (argc == 2 && argv[1][0] != '-') {
        ready = ecx_setupnic(&port, argv[1], 0);
    } else {
        fprintf(stderr, "Usage: veth_probe INTERFACE (or --test-fd FD for local tests)\n");
        return 2;
    }
    if (!ready) {
        fprintf(stderr, "FAIL: cannot initialize probe endpoint\n");
        return 1;
    }

    uint8 index = ecx_getindex(&port);
    uint16 data = 0;
    ec_setupheader(port.txbuf[index]);
    ecx_setupdatagram(&port, port.txbuf[index], EC_CMD_BRD, index,
                     0, 0x0130, sizeof(data), &data);
    printf("PROBE: synthetic BRD test, index=%u, frame_bytes=%d\n",
           index, port.txbuflength[index]);
    fflush(stdout);
    int wkc = ecx_srconfirm(&port, index, 500000);
    int valid = wkc == 2 && port.rxbufstat[index] == EC_BUF_COMPLETE &&
                port.rxbuf[index][EC_HEADERSIZE] == 2 &&
                port.rxbuf[index][EC_HEADERSIZE + 1] == 0;
    printf("PROBE: received WKC=%d (expected synthetic value: 2)\n", wkc);
    if (valid) puts("PASS: matched response with synthetic payload 0x0002 and WKC=2");
    else puts("FAIL: expected matching synthetic response was not received");
    puts("This is not slave discovery or a real EtherCAT slave response.");
    ecx_setbufstat(&port, index, EC_BUF_EMPTY);
    ecx_closenic(&port);
    pthread_mutex_destroy(&port.getindex_mutex);
    pthread_mutex_destroy(&port.tx_mutex);
    pthread_mutex_destroy(&port.rx_mutex);
    return valid ? 0 : 1;
}
