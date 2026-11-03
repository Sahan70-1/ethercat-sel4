#include <microkit.h>

#define CHANNEL_ID 1
#define REQUEST_LABEL 0x10
#define RESPONSE_LABEL 0x20
#define ERROR_LABEL 0xFF

#define REQUEST_VALUE 0x1234
#define RESPONSE_VALUE 0x1235

void init(void)
{
    microkit_dbg_puts("DRIVER: benchmark ready\n");
}

microkit_msginfo protected(microkit_channel ch,
                           microkit_msginfo msginfo)
{
    if (ch != CHANNEL_ID ||
        microkit_msginfo_get_label(msginfo) != REQUEST_LABEL ||
        microkit_msginfo_get_count(msginfo) != 1 ||
        microkit_mr_get(0) != REQUEST_VALUE) {
        return microkit_msginfo_new(ERROR_LABEL, 0);
    }

    microkit_mr_set(0, RESPONSE_VALUE);
    return microkit_msginfo_new(RESPONSE_LABEL, 1);
}

void notified(microkit_channel ch)
{
    (void)ch;
}
