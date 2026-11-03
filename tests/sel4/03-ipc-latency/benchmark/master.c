#include <stdint.h>
#include <microkit.h>
#include "timer_counter.h"

#define CHANNEL_ID 1
#define REQUEST_LABEL 0x10
#define RESPONSE_LABEL 0x20
#define REQUEST_VALUE 0x1234
#define RESPONSE_VALUE 0x1235

#define WARMUP_COUNT 1000
#define SAMPLE_COUNT 10000
#define BASELINE_COUNT 1000

static uint64_t ppc_ticks[SAMPLE_COUNT];
static uint64_t baseline_ticks[BASELINE_COUNT];

static char *append_text(char *p, const char *s)
{
    while (*s) {
        *p++ = *s++;
    }
    return p;
}

static char *append_u64(char *p, uint64_t n)
{
    char digits[20];
    unsigned int count = 0;

    do {
        digits[count++] = '0' + (n % 10);
        n /= 10;
    } while (n != 0);

    while (count > 0) {
        *p++ = digits[--count];
    }
    return p;
}

static void print_number(uint64_t value)
{
    char buffer[32];
    char *p = append_u64(buffer, value);
    *p = '\0';
    microkit_dbg_puts(buffer);
}

static void print_csv(const char *type,
                      uint32_t index,
                      uint64_t ticks)
{
    char buffer[80];
    char *p = buffer;

    p = append_text(p, type);
    *p++ = ',';
    p = append_u64(p, index);
    *p++ = ',';
    p = append_u64(p, ticks);
    *p++ = '\n';
    *p = '\0';

    microkit_dbg_puts(buffer);
}

static int valid_reply(microkit_msginfo reply)
{
    return microkit_msginfo_get_label(reply) == RESPONSE_LABEL &&
           microkit_msginfo_get_count(reply) == 1 &&
           microkit_mr_get(0) == RESPONSE_VALUE;
}

void init(void)
{
    const microkit_msginfo request =
        microkit_msginfo_new(REQUEST_LABEL, 1);

    uint64_t frequency = timer_frequency();

    microkit_dbg_puts("BENCH: starting\n");

    if (frequency == 0) {
        microkit_dbg_puts("BENCH: FAIL - zero frequency\n");
        return;
    }

    /* Warm up without recording measurements. */
    for (uint32_t i = 0; i < WARMUP_COUNT; i++) {
        microkit_mr_set(0, REQUEST_VALUE);

        microkit_msginfo reply =
            microkit_ppcall(CHANNEL_ID, request);

        if (!valid_reply(reply)) {
            microkit_dbg_puts("BENCH: FAIL - warmup reply\n");
            return;
        }
    }

    /* Measure complete synchronous PPC round trips. */
    for (uint32_t i = 0; i < SAMPLE_COUNT; i++) {
        microkit_mr_set(0, REQUEST_VALUE);

        uint64_t start = timer_read();

        microkit_msginfo reply =
            microkit_ppcall(CHANNEL_ID, request);

        uint64_t end = timer_read();

        ppc_ticks[i] = end - start;

        if (!valid_reply(reply)) {
            microkit_dbg_puts("BENCH: FAIL - invalid reply\n");
            return;
        }
    }

    /* Characterize timestamp-read overhead separately. */
    for (uint32_t i = 0; i < BASELINE_COUNT; i++) {
        uint64_t start = timer_read();
        uint64_t end = timer_read();

        baseline_ticks[i] = end - start;
    }

    /* All console output happens after the measurements. */
    microkit_dbg_puts("BENCH: frequency_hz=");
    print_number(frequency);
    microkit_dbg_puts("\n");

    microkit_dbg_puts("BENCH: warmup=1000\n");
    microkit_dbg_puts("BENCH: samples=10000\n");
    microkit_dbg_puts("BENCH: baseline_samples=1000\n");

    microkit_dbg_puts("CSV_BEGIN\n");
    microkit_dbg_puts("type,index,ticks\n");

    for (uint32_t i = 0; i < SAMPLE_COUNT; i++) {
        print_csv("ppc", i, ppc_ticks[i]);
    }

    for (uint32_t i = 0; i < BASELINE_COUNT; i++) {
        print_csv("baseline", i, baseline_ticks[i]);
    }

    microkit_dbg_puts("CSV_END\n");
    microkit_dbg_puts("BENCH: PASS - completed\n");
}

void notified(microkit_channel ch)
{
    (void)ch;
}
