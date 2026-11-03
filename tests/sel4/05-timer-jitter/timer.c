#include <stdint.h>
#include <microkit.h>

#define TIMER_IRQ_CHANNEL 0

static uint64_t deadline;

static uint64_t read_counter(void)
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

static uint64_t read_frequency(void)
{
    uint64_t value;
    __asm__ volatile(
        "mrs %0, cntfrq_el0"
        : "=r"(value)
    );
    return value;
}

static void timer_control(uint64_t value)
{
    __asm__ volatile(
        "msr cntp_ctl_el0, %0\n\t"
        "isb"
        :
        : "r"(value)
        : "memory"
    );
}

void init(void)
{
    microkit_dbg_puts("TIMER: initialized\n");

    uint64_t freq = read_frequency();

    if (freq != 62500000ULL) {
        microkit_dbg_puts("TIMER: FAIL - unexpected frequency\n");
        return;
    }

    timer_control(0);

    deadline = read_counter() + freq / 1000ULL;

    __asm__ volatile(
        "msr cntp_cval_el0, %0\n\t"
        "isb"
        :
        : "r"(deadline)
        : "memory"
    );

    microkit_dbg_puts("TIMER: one-shot armed\n");

    timer_control(1);
}

void notified(microkit_channel ch)
{
    if (ch != TIMER_IRQ_CHANNEL) {
        microkit_dbg_puts("TIMER: FAIL - unexpected channel\n");
        return;
    }

    uint64_t observed = read_counter();

    timer_control(0);
    microkit_irq_ack(ch);

    if (observed < deadline) {
        microkit_dbg_puts("TIMER: FAIL - early interrupt\n");
        return;
    }

    microkit_dbg_puts(
        "TIMER: PASS - physical timer interrupt received\n"
    );
}
