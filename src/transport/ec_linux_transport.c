#define _GNU_SOURCE
#include "ec_linux_transport.h"

#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <time.h>

static ec_io_result error_result(void)
{
    return errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS || errno == EINTR
        ? EC_IO_AGAIN : EC_IO_ERROR;
}

static ec_io_result send_frame(void *context, const void *frame, size_t length)
{
    ssize_t sent = send(*(int *)context, frame, length, MSG_DONTWAIT | MSG_NOSIGNAL);
    if (sent < 0) return error_result();
    return (size_t)sent == length ? EC_IO_OK : EC_IO_ERROR;
}

static ec_io_result receive_frame(void *context, void *frame, size_t capacity, size_t *length)
{
    /* MSG_TRUNC reports the full datagram length, allowing rejection of a
     * frame that would otherwise silently look like a valid truncated one. */
    ssize_t received = recv(*(int *)context, frame, capacity, MSG_DONTWAIT | MSG_TRUNC);
    if (received < 0) return error_result();
    if ((size_t)received > capacity) return EC_IO_INVALID;
    *length = (size_t)received;
    return EC_IO_OK;
}

static ec_io_result now_us(void *context, uint64_t *time)
{
    (void)context;
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts)) return EC_IO_ERROR;
    *time = (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
    return EC_IO_OK;
}

static ec_io_result wait_until(void *context, uint64_t deadline)
{
    struct pollfd fd = { .fd = *(int *)context, .events = POLLIN };
    if (fd.fd < 0) return EC_IO_ERROR;
    for (;;) {
        uint64_t now;
        if (now_us(context, &now) != EC_IO_OK) return EC_IO_ERROR;
        if (now >= deadline) return EC_IO_TIMEOUT;
        uint64_t remaining = deadline - now;
        /* Cap each poll at one second, also avoiding time_t overflow. */
        if (remaining > 1000000) remaining = 1000000;
        struct timespec timeout = { .tv_sec = (time_t)(remaining / 1000000),
                                    .tv_nsec = (long)(remaining % 1000000) * 1000 };
        int result = ppoll(&fd, 1, &timeout, NULL);
        if (result < 0) {
            if (errno == EINTR) continue; /* keep the original deadline */
            return EC_IO_ERROR;
        }
        if (result == 0) continue;
        if (fd.revents & (POLLERR | POLLHUP | POLLNVAL)) return EC_IO_ERROR;
        if (fd.revents & POLLIN) return EC_IO_OK;
    }
}

static const ec_transport_ops ops = { send_frame, receive_frame, now_us, wait_until };

void ec_linux_transport_bind(ec_transport *transport, int *fd, size_t max_frame)
{
    *transport = (ec_transport){ .ops = &ops, .context = fd, .max_frame = max_frame };
}
