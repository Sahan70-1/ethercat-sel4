#ifndef EC_TRANSPORT_H
#define EC_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>

/* Full Ethernet frames, excluding FCS. No EtherCAT parsing in this layer. */
typedef enum {
    EC_IO_OK, EC_IO_AGAIN, EC_IO_TIMEOUT, EC_IO_INVALID, EC_IO_ERROR
} ec_io_result;

typedef struct {
    ec_io_result (*send)(void *context, const void *frame, size_t length);
    ec_io_result (*receive)(void *context, void *frame, size_t capacity, size_t *length);
    ec_io_result (*now_us)(void *context, uint64_t *time);
    ec_io_result (*wait_until)(void *context, uint64_t deadline_us);
} ec_transport_ops;

typedef struct {
    const ec_transport_ops *ops;
    void *context;
    size_t max_frame;
} ec_transport;

/* Single caller per endpoint. Calls retain no caller-owned frame pointer.
 * send OK means accepted/copied, not transmitted on wire; AGAIN means retry.
 * receive consumes one frame, including invalid/truncated frames. On failure
 * length is zero and output bytes must not be parsed. Invalid length is not
 * no data. No buffers escape this copying interface.
 * now/wait use one monotonic microsecond epoch; wait may wake spuriously and
 * callers must recheck RX/deadline. Expired deadline returns TIMEOUT.
 * Resource setup/teardown belongs to the platform owner, not this interface.
 */
ec_io_result ec_transport_send(ec_transport *transport, const void *frame, size_t length);
ec_io_result ec_transport_receive(ec_transport *transport, void *frame, size_t capacity, size_t *length);
ec_io_result ec_transport_now(ec_transport *transport, uint64_t *time);
ec_io_result ec_transport_wait(ec_transport *transport, uint64_t deadline_us);

#endif
