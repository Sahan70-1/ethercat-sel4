# Experiment 01: Protection Domain Communication

**Project:** EtherCAT Implementation on seL4 Microkernels with Real-Time and Performance Guarantees

**Group:** 19
**Institution:** University of Peradeniya, Sri Lanka
**Experiment date:** 2026-11-03
**Status:** Functional test passed in QEMU

## 1. Objective

Verify bidirectional asynchronous notification communication
between two isolated seL4 Microkit Protection Domains.

- Master PD: represents the future EtherCAT master.
- Driver PD: represents the future Ethernet driver.

Neither PD runs SOEM or a real Ethernet driver yet.

## 2. Test Environment

| Component | Configuration |
|---|---|
| Host computer | Apple Silicon MacBook Air M1 |
| Host OS | macOS 26.4.1 |
| Linux | OrbStack Ubuntu 24.04.5 ARM64 |
| Microkit SDK | 2.4.0 |
| QEMU | 8.2.2 |
| GNU Make | 4.3 |
| AArch64 GCC | 13.3.0 |
| Board | qemu_virt_aarch64 |
| Emulated CPU | Cortex-A53 |
| Configuration | debug |
| QEMU RAM | 2 GiB |

## 3. Architecture

    Master PD (priority 150)
             |
       Microkit Channel
             |
    Driver PD (priority 149)

Both PDs use local channel ID 1.

The system configuration is defined in `system.system`.

## 4. Communication Procedure

1. Master PD initializes.
2. Master sends a request notification using microkit_notify().
3. Driver receives the event in notified().
4. Driver sends an acknowledgement notification.
5. Master receives the acknowledgement in notified().

The acknowledgement is an application-level notification,
not an Ethernet frame or hardware interrupt acknowledgement.

## 5. Source Files

- master.c: Master PD application
- driver.c: Driver PD application
- system.system: Protection Domains and channel configuration
- Makefile: Standalone Microkit build and QEMU execution
- .gitignore: Excludes generated build files

## 6. Build and Run

Prerequisites:

- Microkit SDK 2.4.0 for Linux ARM64
- AArch64 GCC and Binutils
- GNU Make
- QEMU AArch64

Build:

    make MICROKIT_SDK=/path/to/microkit-sdk-2.4.0

Run:

    make run MICROKIT_SDK=/path/to/microkit-sdk-2.4.0

Exit QEMU using Ctrl+A followed by X.

The Makefile includes an assembler-search workaround
for the tested Ubuntu ARM64 toolchain.

## 7. Observed Results

The Microkit build completed successfully.

Build output:

    Root objects: 52
    CapDL specification footprint: 7.57 KiB
    Initial task memory: 384 KiB
    Loader image size: 2.63 MiB

The QEMU test produced:

    MON|INFO: Microkit Monitor started!
    MASTER: initialized
    MASTER: sending REQUEST notification
    DRIVER: initialized
    DRIVER: received REQUEST from master
    DRIVER: sending ACK notification
    MASTER: received ACK from driver

The notification request and acknowledgement were
both observed.

Recorded console output:

`../../../results/sel4/01-pd-communication/qemu-notifications.log`

The automated log capture used a 15-second timeout
because the Microkit system continues running.

## 8. Interpretation

Bidirectional notification communication between two
isolated Microkit Protection Domains was functionally
verified in the selected QEMU environment.

This test demonstrates one successful request and
acknowledgement sequence.

## 9. Limitations

- Only one notification round trip was observed.
- Notifications do not carry Ethernet frame payloads.
- Notifications may coalesce when sent repeatedly.
- No shared memory was used.
- No protected procedure calls were tested.
- No IPC latency or jitter measurements were made.
- No real Ethernet driver or EtherCAT slave was used.
- No physical real-time guarantees were established.
- The test ran under QEMU, not physical target hardware.

## 10. Reproducibility

Microkit SDK version: 2.4.0
SDK archive SHA-256:

1ec1006235995a7d100a2b08b2b60cac264b8685208659fa7829342dd37e9a67

This identifies the downloaded SDK archive; its hash
was not independently verified against an official
checksum manifest.

Microkit reference:
https://docs.sel4.systems/projects/microkit/

## 11. Next Experiment

Experiment 02: Shared Memory Communication

Extend the two-PD architecture to transfer data
through a shared memory region.

Test buffer sizes of 64, 512 and 1518 bytes, verify
data integrity, and use notifications for synchronization.
