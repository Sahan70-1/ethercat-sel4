#include <stdint.h>
#include <microkit.h>
#include "shared_protocol.h"

/* Filled with the mapped address by Microkit. */
uintptr_t shared_buffer_vaddr;

static uint32_t current_test = 0;

static const uint32_t test_sizes[TEST_COUNT] = {
    64, 512, 1518
};

static void send_test(void)
{
    volatile shared_packet_t *packet =
        (volatile shared_packet_t *)shared_buffer_vaddr;

    uint32_t sequence = current_test + 1;
    uint32_t length = test_sizes[current_test];

    packet->sequence = sequence;
    packet->length = length;

    for (uint32_t i = 0; i < length; i++) {
        packet->payload[i] = expected_byte(sequence, i);
    }

    /* Publish data before notifying the receiver. */
    __atomic_thread_fence(__ATOMIC_RELEASE);

    if (length == 64) {
        microkit_dbg_puts("MASTER: sending 64 bytes\n");
    } else if (length == 512) {
        microkit_dbg_puts("MASTER: sending 512 bytes\n");
    } else {
        microkit_dbg_puts("MASTER: sending 1518 bytes\n");
    }

    microkit_notify(CHANNEL_ID);
}

void init(void)
{
    microkit_dbg_puts("MASTER: initialized\n");
    send_test();
}

void notified(microkit_channel ch)
{
    if (ch != CHANNEL_ID) {
        microkit_dbg_puts("MASTER: unexpected channel\n");
        return;
    }

    if (current_test >= TEST_COUNT) {
        microkit_dbg_puts("MASTER: unexpected ACK\n");
        return;
    }

    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    microkit_dbg_puts("MASTER: received ACK\n");

    current_test++;

    if (current_test < TEST_COUNT) {
        send_test();
    } else {
        microkit_dbg_puts(
            "MASTER: ALL 3 TRANSFERS ACKNOWLEDGED\n"
        );
    }
}
