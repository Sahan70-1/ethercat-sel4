#include <microkit.h>

#define DRIVER_CHANNEL 1

void init(void)
{
    microkit_dbg_puts("MASTER: initialized\n");

    microkit_dbg_puts("MASTER: sending REQUEST notification\n");

    microkit_notify(DRIVER_CHANNEL);
}

void notified(microkit_channel ch)
{
    if (ch == DRIVER_CHANNEL) {
        microkit_dbg_puts("MASTER: received ACK from driver\n");
    } else {
        microkit_dbg_puts("MASTER: unexpected notification\n");
    }
}
