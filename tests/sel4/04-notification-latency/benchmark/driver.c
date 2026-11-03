#include <stdint.h>
#include <microkit.h>

#include "shared_protocol.h"

#define WARMUP_PER_SIZE 1000u
#define SAMPLES_PER_SIZE 10000u
#define TRANSFERS_PER_SIZE (WARMUP_PER_SIZE + SAMPLES_PER_SIZE)

uintptr_t shared_buffer_vaddr;

static const uint32_t sizes[TEST_COUNT] = {
    64u, 512u, 1518u
};

static uint32_t received_count = 0;
static uint8_t failed = 0;

void init(void)
{
    microkit_dbg_puts("BENCH: DRIVER initialized\n");
}

void notified(microkit_channel ch)
{
    if (ch != CHANNEL_ID || failed ||
        received_count >= TEST_COUNT * TRANSFERS_PER_SIZE) {
        microkit_dbg_puts(
            "BENCH: FAIL - unexpected request\n"
        );
        failed = 1;
        return;
    }

    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    volatile const shared_packet_t *packet =
        (volatile const shared_packet_t *)shared_buffer_vaddr;

    uint32_t expected_sequence = received_count + 1u;
    uint32_t size_index = received_count / TRANSFERS_PER_SIZE;
    uint32_t expected_length = sizes[size_index];

    if (packet->sequence != expected_sequence ||
        packet->length != expected_length) {
        microkit_dbg_puts(
            "BENCH: FAIL - sequence or length mismatch\n"
        );
        failed = 1;
        return;
    }

    for (uint32_t i = 0; i < expected_length; i++) {
        if (packet->payload[i] !=
            expected_byte(expected_sequence, i)) {
            microkit_dbg_puts(
                "BENCH: FAIL - payload mismatch\n"
            );
            failed = 1;
            return;
        }
    }

    received_count++;

    __atomic_thread_fence(__ATOMIC_RELEASE);

    /* Acknowledge only after successful validation. */
    microkit_notify(CHANNEL_ID);
}
