#ifndef EC_LINUX_TRANSPORT_H
#define EC_LINUX_TRANSPORT_H

#include "ec_transport.h"

/* Borrow a bound SOCK_RAW or connected SOCK_DGRAM fd; does not close it.
 * fd and the pointed-to integer must remain valid for the endpoint lifetime.
 * SOCK_DGRAM socketpairs are only local test transport, not an EtherCAT NIC.
 */
void ec_linux_transport_bind(ec_transport *transport, int *fd, size_t max_frame);

#endif
