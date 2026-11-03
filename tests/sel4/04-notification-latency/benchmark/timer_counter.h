#ifndef TIMER_COUNTER_H
#define TIMER_COUNTER_H

#include <stdint.h>

/* Read the physical counter with instruction barriers. */
static inline uint64_t timer_read(void)
{
    uint64_t value;

    __asm__ volatile(
        "isb\n\t"
        "mrs %0, cntpct_el0\n\t"
        "isb"
        : "=r"(value)
        :
        : "memory"
    );

    return value;
}

static inline uint64_t timer_frequency(void)
{
    uint64_t value;

    __asm__ volatile(
        "mrs %0, cntfrq_el0"
        : "=r"(value)
    );

    return value;
}

#endif
