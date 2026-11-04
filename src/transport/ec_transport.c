#include "ec_transport.h"

static int valid(const ec_transport *t)
{
    return t && t->ops && t->max_frame >= 14;
}

ec_io_result ec_transport_send(ec_transport *t, const void *frame, size_t length)
{
    if (!valid(t) || !t->ops->send || !frame || length < 14 || length > t->max_frame)
        return EC_IO_INVALID;
    return t->ops->send(t->context, frame, length);
}

ec_io_result ec_transport_receive(ec_transport *t, void *frame, size_t capacity, size_t *length)
{
    size_t received = 0;
    if (length) *length = 0;
    if (!valid(t) || !t->ops->receive || !frame || !length || capacity < 14)
        return EC_IO_INVALID;
    if (capacity > t->max_frame) capacity = t->max_frame;
    ec_io_result result = t->ops->receive(t->context, frame, capacity, &received);
    if (result != EC_IO_OK) return result;
    if (received < 14 || received > capacity) return EC_IO_INVALID;
    *length = received;
    return EC_IO_OK;
}

ec_io_result ec_transport_now(ec_transport *t, uint64_t *time)
{
    if (!valid(t) || !t->ops->now_us || !time) return EC_IO_INVALID;
    return t->ops->now_us(t->context, time);
}

ec_io_result ec_transport_wait(ec_transport *t, uint64_t deadline_us)
{
    if (!valid(t) || !t->ops->wait_until) return EC_IO_INVALID;
    return t->ops->wait_until(t->context, deadline_us);
}
