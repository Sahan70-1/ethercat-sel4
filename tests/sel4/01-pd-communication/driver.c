#include <microkit.h>

#define MASTER_CHANNEL 1

void init(void)
{
    microkit_dbg_puts("DRIVER: initialized\n");
}

void notified(microkit_channel ch)
{
    if (ch == MASTER_CHANNEL) {
        microkit_dbg_puts("DRIVER: received REQUEST from master\n");

        microkit_dbg_puts("DRIVER: sending ACK notification\n");

        microkit_notify(MASTER_CHANNEL);
    } else {
        microkit_dbg_puts("DRIVER: unexpected notification\n");
    }
}
