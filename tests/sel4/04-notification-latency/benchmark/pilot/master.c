#include <stdint.h>
#include <microkit.h>

#include "shared_protocol.h"
#include "timer_counter.h"

#define SAMPLES_PER_SIZE 5u

uintptr_t shared_buffer_vaddr;

static const uint32_t sizes[TEST_COUNT] = {
    64u, 512u, 1518u
};

static uint64_t samples[TEST_COUNT][SAMPLES_PER_SIZE];

static uint32_t size_index = 0;
static uint32_t sample_index = 0;
static uint32_t sequence = 0;
static uint64_t start_ticks = 0;
static uint8_t finished = 0;

static void print_u64(uint64_t value)
{
    char buffer[24];
    unsigned pos = sizeof(buffer) - 1;

    buffer[pos] = '\0';

    do {
        buffer[--pos] = (char)('0' + (value % 10u));
        value /= 10u;
    } while (value != 0);

    microkit_dbg_puts(&buffer[pos]);
}

static void print_results(void)
{
    microkit_dbg_puts("BENCH: FREQ_HZ,");
    print_u64(timer_frequency());
    microkit_dbg_puts("\n");

    microkit_dbg_puts(
        "BENCH: CSV,size_bytes,sample_index,ticks\n"
    );

    for (uint32_t s = 0; s < TEST_COUNT; s++) {
        for (uint32_t i = 0; i < SAMPLES_PER_SIZE; i++) {
            microkit_dbg_puts("BENCH: CSV,");
            print_u64(sizes[s]);
            microkit_dbg_puts(",");
            print_u64(i);
            microkit_dbg_puts(",");
            print_u64(samples[s][i]);
            microkit_dbg_puts("\n");
        }
    }

    microkit_dbg_puts(
        "BENCH: PASS - 15 notification round trips\n"
    );
}

static void send_next(void)
{
    volatile shared_packet_t *packet =
        (volatile shared_packet_t *)shared_buffer_vaddr;

    sequence++;

    uint32_t length = sizes[size_index];

    packet->sequence = sequence;
    packet->length = length;

    for (uint32_t i = 0; i < length; i++) {
        packet->payload[i] = expected_byte(sequence, i);
    }

    __atomic_thread_fence(__ATOMIC_RELEASE);

    /* Begin timing after payload preparation. */
    start_ticks = timer_read();

    microkit_notify(CHANNEL_ID);
}

void init(void)
{
    microkit_dbg_puts("BENCH: MASTER initialized\n");
    send_next();
}

void notified(microkit_channel ch)
{
    /* Capture the ACK timestamp immediately. */
    uint64_t end_ticks = timer_read();

    if (ch != CHANNEL_ID || finished) {
        microkit_dbg_puts(
            "BENCH: FAIL - unexpected notification\n"
        );
        return;
    }

    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    if (end_ticks < start_ticks) {
        microkit_dbg_puts(
            "BENCH: FAIL - timer went backwards\n"
        );
        finished = 1;
        return;
    }

    samples[size_index][sample_index] =
        end_ticks - start_ticks;

    sample_index++;

    if (sample_index == SAMPLES_PER_SIZE) {
        sample_index = 0;
        size_index++;
    }

    if (size_index < TEST_COUNT) {
        send_next();
    } else {
        finished = 1;
        print_results();
    }
}
