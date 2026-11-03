# Experiment 00: seL4 Microkit Boot Verification

**Project:** EtherCAT Implementation on seL4 Microkernels with Real-Time and Performance Guarantees
**Group:** 19
**Institution:** University of Peradeniya, Sri Lanka
**Experiment date:** 2026-11-03
**Status:** Verified on QEMU ARM64

## 1. Objective

Set up a seL4 Microkit development environment and verify
that a basic Protection Domain can execute inside QEMU.

This is an environment-verification milestone, not a
real-time performance experiment.

## 2. Experimental Environment

| Component | Configuration |
|---|---|
| Host computer | Apple Silicon MacBook Air M1 |
| Host operating system | macOS 26.4.1 |
| Linux environment | OrbStack, Ubuntu 24.04.5 ARM64 |
| Microkit SDK | 2.4.0 |
| Microkit board | qemu_virt_aarch64 |
| Microkit configuration | debug |
| GNU Make | 4.3 |
| AArch64 GCC | 13.3.0 |
| GNU Binutils | 2.42 |
| QEMU | 8.2.2 |
| Emulated CPU | Cortex-A53 |
| QEMU RAM | 2 GiB |

## 3. Application Architecture

One Protection Domain called `serial_server` runs under
seL4 Microkit.

The domain accesses QEMU's emulated PL011 UART using a
memory-mapped device region.

Physical UART address: 0x09000000

Mapped virtual address: 0x04000000

The program prints:

    SERIAL SERVER: starting

## 4. Source Files

- `serial_server.c`: UART-based serial server application
- `system.system`: Microkit system description
- `Makefile`: Standalone build and QEMU execution
- `.gitignore`: Excludes generated build artifacts

The serial server is adapted from the official seL4
Microkit Wordle tutorial. The unused `printf.h`
dependency was removed for this standalone experiment.

Tutorial source:
https://trustworthy.systems/Downloads/microkit_tutorial/

## 5. Build Instructions

Prerequisites:

- Ubuntu ARM64
- GNU Make
- AArch64 GCC and Binutils
- QEMU ARM64
- Microkit SDK 2.4.0

Set the Microkit SDK path:

    export MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0"

Build:

    make MICROKIT_SDK="$MICROKIT_SDK"

Expected generated artifacts:

- `build/serial_server.o`
- `build/serial_server.elf`
- `build/loader.img`
- `build/report.txt`

Run:

    make run MICROKIT_SDK="$MICROKIT_SDK"

Exit QEMU using Ctrl+A followed by X.

The Makefile includes a GCC assembler-search workaround
used in the tested Ubuntu ARM64 environment.

## 6. Verification Results

The standalone repository version compiled successfully
and booted in QEMU.

Observed output:

    Booting all finished, dropped to user space
    MON|INFO: Microkit Monitor started!
    SERIAL SERVER: starting

Microkit build output:

    Root objects: 36
    CapDL specification footprint: 5.28 KiB
    Initial task memory: 368 KiB
    Loader image size: 2.62 MiB

Full captured boot log:

`../../../results/sel4/00-microkit-hello-world/qemu-boot.log`

The log was captured with a 15-second timeout because
the serial-server application runs continuously.
Timeout termination is not considered an application
failure when successful startup has been observed.

## 7. Observations and Limitations

- seL4 booted successfully in QEMU ARM64.
- The Microkit monitor started.
- The serial-server Protection Domain executed.
- UART output was observed.
- A GIC target-ID inference warning appeared during boot.
- The warning did not prevent this application from starting.
- Inter-domain communication has not yet been tested.
- No EtherCAT master or slave was involved.
- No timing measurements or real-time guarantees were established.
- QEMU results cannot establish physical real-time guarantees.

## 8. Reproducibility

Downloaded Microkit SDK archive SHA-256:

1ec1006235995a7d100a2b08b2b60cac264b8685208659fa7829342dd37e9a67

Downloaded tutorial archive SHA-256:

b76da23c1191e5a4eac734ed30ae5b03a3e41107ec03bad1ccd973251cb6d374

These hashes identify the downloaded archives; they
were not independently authenticated against a published
checksum manifest.

## 9. Next Experiment

Create Master and Driver Protection Domains and verify
communication using Microkit channels and notifications.
