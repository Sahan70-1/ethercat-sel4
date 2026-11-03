#ifndef TIMER_PROBE_H
#define TIMER_PROBE_H

#include <stdint.h>
#include <microkit.h>

/* Read the ARM64 physical counter with an ISB. */
static inline uint64_t timer_read(void)
{
    uint64_t value;

    __asm__ volatile(
        "isb\n\t"
        "mrs %0, cntpct_el0"
        : "=r"(value)
        :
        : "memory"
    );

    return value;
}

/* Read the counter frequency in Hz. */
static inline uint64_t timer_frequency(void)
{
    uint64_t value;

    __asm__ volatile(
        "mrs %0, cntfrq_el0"
        : "=r"(value)
    );

    return value;
}

/* Print an unsigned 64-bit integer without libc. */
static void print_u64(uint64_t value)
{
    char buffer[21];
    unsigned int pos = 20;

    buffer[pos] = '\0';

    do {
        buffer[--pos] = '0' + (value % 10);
        value /= 10;
    } while (value != 0);

    microkit_dbg_puts(&buffer[pos]);
}

static void timer_probe(void)
{
    uint64_t frequency = timer_frequency();
    uint64_t start = timer_read();

    /* Small test workload to allow counter advancement. */
    for (volatile uint32_t i = 0; i < 10000; i++) {
    }

    uint64_t end = timer_read();

    microkit_dbg_puts("TIMER: frequency_hz=");
    print_u64(frequency);
    microkit_dbg_puts("\n");

    microkit_dbg_puts("TIMER: elapsed_ticks=");
    print_u64(end - start);
    microkit_dbg_puts("\n");

    if (frequency > 0 && end > start) {
        microkit_dbg_puts("TIMER: PASS\n");
    } else {
        microkit_dbg_puts("TIMER: FAIL\n");
    }
}

#endif
