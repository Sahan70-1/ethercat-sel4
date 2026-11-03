#include <microkit.h>
#include "timer_probe.h"

#define DRIVER_CHANNEL 1

#define REQUEST_LABEL  0x10
#define RESPONSE_LABEL 0x20

#define REQUEST_VALUE  0x1234
#define RESPONSE_VALUE 0x1235

void init(void)
{
    microkit_dbg_puts("MASTER: initialized\n");

    timer_probe();

    /* Place one argument in message register 0. */
    microkit_mr_set(0, REQUEST_VALUE);

    /* Message label and number of argument words. */
    microkit_msginfo request =
        microkit_msginfo_new(REQUEST_LABEL, 1);

    microkit_dbg_puts("MASTER: calling Driver PPC\n");

    /* Synchronous call: blocks until Driver replies. */
    microkit_msginfo response =
        microkit_ppcall(DRIVER_CHANNEL, request);

    microkit_dbg_puts("MASTER: PPC returned\n");

    /* Verify the reply from Driver. */
    if (microkit_msginfo_get_label(response) == RESPONSE_LABEL &&
        microkit_msginfo_get_count(response) == 1 &&
        microkit_mr_get(0) == RESPONSE_VALUE) {

        microkit_dbg_puts(
            "MASTER: PASS - PPC response verified\n"
        );

    } else {
        microkit_dbg_puts(
            "MASTER: FAIL - incorrect PPC response\n"
        );
    }
}

void notified(microkit_channel ch)
{
    (void)ch;
    microkit_dbg_puts("MASTER: unexpected notification\n");
}
