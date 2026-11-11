/* Experiment 5C: read-only VirtIO MMIO identification, not a TX/RX driver.
 * Register definitions follow VirtIO 1.2, section 4.2.2 (MMIO register layout).
 * No sDDF implementation is compiled or linked into this probe.
 */
#include <stdint.h>
#include <microkit.h>

/* Patched by Microkit from device_probe.system's read-only device mapping. */
uintptr_t virtio_base_vaddr;

enum {
    REG_MAGIC = 0x000,
    REG_VERSION = 0x004,
    REG_DEVICE_ID = 0x008,
    REG_VENDOR_ID = 0x00c,
};

static uint32_t read_register(uintptr_t offset)
{
    return *(volatile const uint32_t *)(virtio_base_vaddr + offset);
}

static void print_hex(const char *name, uint32_t value)
{
    static const char digits[] = "0123456789abcdef";
    microkit_dbg_puts(name);
    microkit_dbg_puts("0x");
    for (int shift = 28; shift >= 0; shift -= 4) {
        microkit_dbg_putc(digits[(value >> shift) & 0xfu]);
    }
    microkit_dbg_puts("\n");
}

void init(void)
{
    microkit_dbg_puts("DEVICE PROBE: starting (read-only MMIO)\n");
    const uint32_t magic = read_register(REG_MAGIC);
    const uint32_t version = read_register(REG_VERSION);
    const uint32_t device_id = read_register(REG_DEVICE_ID);
    const uint32_t vendor_id = read_register(REG_VENDOR_ID);

    print_hex("Magic: ", magic);
    print_hex("Version: ", version);
    print_hex("Device ID: ", device_id);
    print_hex("Vendor ID: ", vendor_id);

    if (magic != UINT32_C(0x74726976)) {
        microkit_dbg_puts("FAIL: invalid VirtIO MMIO magic\n");
        return;
    }
    if (device_id == 0) {
        /* An empty slot is valid in VirtIO. Skip it without a device error;
         * the host runner separately requires a network-device PASS marker. */
        microkit_dbg_puts("DEVICE PROBE: empty MMIO slot; skipping\n");
        return;
    }
    if (device_id != 1) {
        microkit_dbg_puts("FAIL: attached VirtIO device is not a network card\n");
        return;
    }
    if (version != 2) {
        microkit_dbg_puts("FAIL: modern VirtIO MMIO version 2 required\n");
        return;
    }
    microkit_dbg_puts("PASS: modern VirtIO-net MMIO device identified\n");
    microkit_dbg_puts("NOT TESTED: device initialization, queues, IRQs, Ethernet TX/RX or SOEM\n");
}

void notified(microkit_channel channel)
{
    (void)channel;
    microkit_dbg_puts("FAIL: unexpected notification in read-only device probe\n");
}
