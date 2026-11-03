#include <stdint.h>
#include <microkit.h>
#include "shared_protocol.h"

/* Filled with the mapped address by Microkit. */
uintptr_t shared_buffer_vaddr;

static uint32_t expected_test = 0;

static const uint32_t test_sizes[TEST_COUNT] = {
    64, 512, 1518
};

void init(void)
{
    microkit_dbg_puts("DRIVER: initialized\n");
}

void notified(microkit_channel ch)
{
    if (ch != CHANNEL_ID) {
        microkit_dbg_puts("DRIVER: unexpected channel\n");
        return;
    }

    if (expected_test >= TEST_COUNT) {
        microkit_dbg_puts("DRIVER: unexpected request\n");
        return;
    }

    /* Observe data published before notification. */
    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    volatile const shared_packet_t *packet =
        (volatile const shared_packet_t *)shared_buffer_vaddr;

    uint32_t sequence = expected_test + 1;
    uint32_t length = test_sizes[expected_test];

    if (packet->sequence != sequence ||
        packet->length != length) {
        microkit_dbg_puts(
            "DRIVER: FAIL - incorrect sequence or length\n"
        );
        return;
    }

    for (uint32_t i = 0; i < length; i++) {
        if (packet->payload[i] != expected_byte(sequence, i)) {
            microkit_dbg_puts(
                "DRIVER: FAIL - payload mismatch\n"
            );
            return;
        }
    }

    if (length == 64) {
        microkit_dbg_puts("DRIVER: PASS - 64 bytes\n");
    } else if (length == 512) {
        microkit_dbg_puts("DRIVER: PASS - 512 bytes\n");
    } else {
        microkit_dbg_puts("DRIVER: PASS - 1518 bytes\n");
    }

    expected_test++;

    /* Complete validation before acknowledging. */
    __atomic_thread_fence(__ATOMIC_RELEASE);
    microkit_notify(CHANNEL_ID);
}
