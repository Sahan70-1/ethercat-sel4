#include <stdint.h>
#include <microkit.h>

#define TIMER_CH 0
#define WARMUP 10
#define SAMPLES 50
#define TOTAL (WARMUP + SAMPLES)

static uint64_t period_ticks;
static uint64_t deadline;
static uint64_t previous_time;
static uint64_t skipped_total;
static uint32_t interrupt_count;
static uint32_t early_count;

static uint64_t intervals[SAMPLES];
static uint64_t lateness[SAMPLES];
static uint64_t skipped[SAMPLES];

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

static void set_control(uint64_t value)
{
    __asm__ volatile(
        "msr cntp_ctl_el0, %0\n\t"
        "isb"
        :
        : "r"(value)
        : "memory"
    );
}

static void set_compare(uint64_t value)
{
    __asm__ volatile(
        "msr cntp_cval_el0, %0\n\t"
        "isb"
        :
        : "r"(value)
        : "memory"
    );
}

static void print_number(uint64_t value)
{
    char buffer[24];
    unsigned int pos = sizeof(buffer) - 1;
    buffer[pos] = '\0';

    do {
        buffer[--pos] = '0' + (value % 10);
        value /= 10;
    } while (value != 0);

    microkit_dbg_puts(&buffer[pos]);
}

static void print_results(void)
{
    microkit_dbg_puts("PILOT: COMPLETE\n");

    microkit_dbg_puts("PILOT: period_ticks=");
    print_number(period_ticks);
    microkit_dbg_puts("\n");

    microkit_dbg_puts("PILOT: skipped_periods=");
    print_number(skipped_total);
    microkit_dbg_puts("\n");

    microkit_dbg_puts("PILOT: early_interrupts=");
    print_number(early_count);
    microkit_dbg_puts("\n");

    microkit_dbg_puts("PILOT_CSV_BEGIN\n");
    microkit_dbg_puts(
        "sample,interval_ticks,lateness_ticks,skipped_periods\n"
    );

    for (uint32_t i = 0; i < SAMPLES; i++) {
        print_number(i + 1);
        microkit_dbg_puts(",");
        print_number(intervals[i]);
        microkit_dbg_puts(",");
        print_number(lateness[i]);
        microkit_dbg_puts(",");
        print_number(skipped[i]);
        microkit_dbg_puts("\n");
    }

    microkit_dbg_puts("PILOT_CSV_END\n");
}

void init(void)
{
    uint64_t frequency = read_frequency();

    microkit_dbg_puts("PILOT: initialized\n");

    if (frequency != 62500000ULL) {
        microkit_dbg_puts(
            "PILOT: FAIL - unexpected timer frequency\n"
        );
        return;
    }

    period_ticks = frequency / 1000ULL;

    set_control(0);

    deadline = read_counter() + period_ticks;
    set_compare(deadline);

    microkit_dbg_puts("PILOT: starting 1 ms timer\n");
    set_control(1);
}

void notified(microkit_channel ch)
{
    if (ch != TIMER_CH) {
        set_control(0);
        microkit_dbg_puts(
            "PILOT: FAIL - unexpected notification channel\n"
        );
        return;
    }

    uint64_t now = read_counter();

    uint64_t late = 0;
    uint64_t missed = 0;

    if (now >= deadline) {
        late = now - deadline;
        missed = late / period_ticks;
    } else {
        early_count++;
    }

    skipped_total += missed;

    if (interrupt_count >= WARMUP) {
        uint32_t index = interrupt_count - WARMUP;

        intervals[index] = now - previous_time;
        lateness[index] = late;
        skipped[index] = missed;
    }

    previous_time = now;
    interrupt_count++;

    if (interrupt_count == TOTAL) {
        set_control(0);
        microkit_irq_ack(ch);
        print_results();
        return;
    }

    deadline += (missed + 1ULL) * period_ticks;

    set_compare(deadline);

    microkit_irq_ack(ch);
}
