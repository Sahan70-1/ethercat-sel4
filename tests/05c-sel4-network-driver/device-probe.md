# Experiment 5C: identify the virtual network device from seL4

The basic UART boot check has passed on student WSL. This next check builds a
new, small seL4 protection domain (PD) that reads QEMU's network-device
identification registers. It establishes device access before initialization
and packet handling. It does not send or receive Ethernet frames.

QEMU supplies the emulated network card, called **VirtIO-net**. The guest still
needs driver software to operate it. This probe is new project code; it does
not compile or link sDDF, SOEM, Linux sockets, or an existing NIC driver.
The full driver implementation choice remains documented in the
[5C plan](../../docs/experiment-05c-sel4-network-driver.md#driver-choice-what-is-known-and-what-remains-open).

## What each file does

| File | Purpose |
|---|---|
| [device_probe.c](device_probe.c) | Runs inside seL4; reads four aligned 32-bit registers and checks the device identity |
| [device_probe.system](device_probe.system) | Gives that PD read-only, uncached access to the device-register page |
| [run_device_probe.sh](run_device_probe.sh) | Compiles the C code, builds the Microkit image, attaches the virtual card and checks actual QEMU output |

The original UART experiment and `run_boot.sh` remain unchanged. Probe build
outputs go into a new `/tmp/ethercat-05c-probe.*` directory on each run.

## Run in the existing WSL terminal

Use a clean checkout on the 5C branch. If Git reports a conflict or local
changes preventing an update, retain the message rather than discarding work.

```sh
(
  set -e
  cd ~/FYP/ethercat-sel4
  git switch experiment-05c-sel4-network-driver
  git pull --ff-only origin experiment-05c-sel4-network-driver
  export MICROKIT_SDK="$HOME/tools/microkit-sdk-2.4.0"
  set +e
  bash tests/05c-sel4-network-driver/run_device_probe.sh "$MICROKIT_SDK"
  echo "Probe runner exit status: $?"
)
```

Wait about 30 seconds for the saved console output. Expected observations:

| Register | Expected value | Meaning |
|---|---|---|
| Magic | `0x74726976` | Identifies a VirtIO MMIO transport |
| Version | `0x00000002` | Modern MMIO interface |
| Device ID | `0x00000001` | Network device |
| Vendor ID | `0x554d4551` in tested QEMU | Reported for context, not used as a portable pass condition |

The guest should print `PASS: modern VirtIO-net MMIO device identified`, and
the host runner should exit **0**. QEMU's internal timeout status **124** is
expected because the probe PD waits after initialization; the runner also
requires kernel/monitor/probe startup and device-identification markers and
rejects guest error messages. Send the full output and final runner status.

The warning that hub 0 is not connected to the host network is expected:
the virtual card is connected only to an isolated QEMU hub for this probe.
No root access, TAP interface, network socket or physical slave is needed.

## Configuration and limits

- SDK: Microkit 2.4.0, `qemu_virt_aarch64`, `debug`; compiler target AArch64
  little endian with GNU C11, warnings treated as errors.
- QEMU machine: `virt-8.2,virtualization=on`, Cortex-A53, TCG, 2 GiB RAM.
  The versioned machine name avoids silently selecting a newer machine layout.
- Card: `virtio-net-device`, explicitly attached to `virtio-mmio-bus.0`;
  `virtio-mmio.force-legacy=false` selects modern interface version 2.
- QEMU `info qtree` confirmed bus 0 at physical `0x0a000000`, length `0x200`.
  The system maps a 4 KiB device page at virtual `0x04000000`, with `perms="r"`
  and `cached="false"`. That page contains eight transport slots; the probe
  reads only offsets `0x000`, `0x004`, `0x008`, `0x00c` of slot 0. The mapping
  does not establish isolation between slots sharing that page.
- Output uses Microkit's debug console. No network IRQ, DMA buffer, queue,
  notification channel or timer is configured. No device register is written.

Artifacts: `environment.txt`, `build.txt`, the object/ELF/image, `report.txt`,
`qemu-command.txt`, `qemu-boot.log`, `qemu-stderr.log`, `qemu-debug.log` and
`result.txt`. An optional second runner argument selects a new, nonexistent
output directory. Source/configuration/runner/image hashes identify each run.

## Negative controls

These are optional validation cases, not the student's normal next command.
The same guest source/image is tested with altered QEMU device configuration.
Each runner is expected to exit **1**, even though seL4 itself boots:

```sh
(
  set +e
  PROBE_MODE=absent bash tests/05c-sel4-network-driver/run_device_probe.sh "$MICROKIT_SDK"
  echo "Absent-card runner exit status: $? (expected: 1)"
  PROBE_MODE=legacy bash tests/05c-sel4-network-driver/run_device_probe.sh "$MICROKIT_SDK"
  echo "Legacy-interface runner exit status: $? (expected: 1)"
)
```

An empty slot has Device ID 0, which is valid in VirtIO. The guest skips it
without a device error; the host test fails because the required network-card
PASS marker is absent. The legacy case has Version 1 and is rejected by this
modern-interface probe. These cases guard against treating boot alone as
evidence that the intended network device was found.

## Reference and next step

The register layout and aligned-access rules are from VirtIO 1.2, section
4.2.2; network Device ID 1 is in section 5.1.1. Inspected primary source:
[OASIS VirtIO specification, v1.2-cs01](https://github.com/oasis-tcs/virtio-spec/blob/e230375adde48ff39f734ea2783ef7b9d2ba346a/content.tex).
Register facts were also cross-checked against Linux's `virtio_mmio.h` and the
previously inspected sDDF headers. No upstream driver implementation is copied
into this probe.

[Cloud results and negative controls](../../results/05c-sel4-network-driver/device-probe.md)
are separate from the pending student WSL probe result. After this check,
the next implementation step is device reset and feature negotiation, followed
by TX/RX queues, buffer ownership and interrupt handling. Raw-frame tests must
pass before claiming a functioning Ethernet driver or integrating SOEM.
