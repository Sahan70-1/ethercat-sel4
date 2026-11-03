# Experiment 02: Shared Memory Communication

**Research Project:** EtherCAT Implementation on seL4 Microkernels with Real-Time and Performance Guarantees

**Group:** 19
**Institution:** University of Peradeniya, Sri Lanka
**Experiment Date:** 2026-11-03
**Status:** Functional tests passed in QEMU

## 1. Objective

Verify data transfer between two isolated seL4 Microkit
Protection Domains using shared memory and notifications.

Test payload sizes:
- 64 bytes
- 512 bytes
- 1518 bytes

Verify data integrity and acknowledgement for each transfer.

## 2. Experimental Environment

| Component | Configuration |
|---|---|
| Host | MacBook Air M1, Apple Silicon |
| macOS | 26.4.1 |
| Linux | OrbStack Ubuntu 24.04.5 ARM64 |
| Microkit SDK | 2.4.0 |
| Board | qemu_virt_aarch64 |
| Configuration | debug |
| QEMU | 8.2.2 |
| Emulated CPU | Cortex-A53 |
| Virtual CPUs | 1 (QEMU default) |
| RAM | 2 GiB |
| GCC | AArch64 GCC 13.3.0 |
| GNU Make | 4.3 |

## 3. Architecture

    Master PD (priority 150)
      Read/Write access
             |
       Shared Memory
          4096 B
             |
       Read-only access
    Driver PD (priority 149)

The two domains also communicate through a Microkit
notification channel with local channel ID 1.

The shared memory region is configured in system.system.

## 4. Shared Data Structure

The shared packet contains:

- 32-bit sequence number
- 32-bit payload length
- Payload buffer of 1518 bytes

Payload bytes follow a deterministic pattern defined
by expected_byte() in shared_protocol.h.

The Driver independently checks each payload byte.

## 5. Communication Procedure

1. Master writes a sequence number and payload length.
2. Master writes the payload into shared memory.
3. Master sends a notification to Driver.
4. Driver reads the shared data.
5. Driver verifies sequence, length and every payload byte.
6. Driver sends an ACK notification only after validation.
7. Master receives ACK and starts the next test.

One buffer is reused sequentially.

## 6. Source Files

- master.c: Writes test payloads and handles ACKs
- driver.c: Reads and validates payloads
- shared_protocol.h: Shared packet format and data pattern
- system.system: Memory mappings and notification channel
- Makefile: Compilation and QEMU execution
- .gitignore: Excludes generated build files

## 7. Build and Run

Prerequisites:
- Microkit SDK 2.4.0 for Linux ARM64
- AArch64 GCC and Binutils
- GNU Make
- QEMU ARM64

Build:

    make MICROKIT_SDK=/path/to/microkit-sdk-2.4.0

Run:

    make run MICROKIT_SDK=/path/to/microkit-sdk-2.4.0

Exit QEMU using Ctrl+A followed by X.

The Makefile includes the assembler-search workaround
required by the tested Ubuntu toolchain.

## 8. Actual Results

Microkit build completed successfully.

Build statistics:

    Root objects: 55
    CapDL specification footprint: 8.03 KiB
    Initial task memory: 388 KiB
    Loader image size: 2.63 MiB

Observed application output:

    MASTER: sending 64 bytes
    DRIVER: PASS - 64 bytes
    MASTER: received ACK

    MASTER: sending 512 bytes
    DRIVER: PASS - 512 bytes
    MASTER: received ACK

    MASTER: sending 1518 bytes
    DRIVER: PASS - 1518 bytes
    MASTER: received ACK

    MASTER: ALL 3 TRANSFERS ACKNOWLEDGED

All three payload-integrity checks passed in the
observed QEMU execution.

The result was also reproduced during log capture.

Raw log:

../../../results/sel4/02-shared-memory/qemu-shared-memory.log

## 9. Interpretation

The experiment demonstrates successful data exchange
through a shared memory region mapped into two isolated
Microkit Protection Domains.

The Master used read-write access.
The Driver used a read-only mapping.

Notifications were used for request and acknowledgement
synchronization.

## 10. Limitations

- Only three sequential transfers were tested per run.
- No throughput or latency measurements were performed.
- No physical Ethernet hardware was used.
- No actual EtherCAT frames were transmitted.
- The test used a single emulated CPU.
- No concurrent multicore stress testing was performed.
- No negative test of memory access permissions was run.
- Volatile accesses and fences in the test implementation
  are not a complete general-purpose multicore protocol.
- Notifications can coalesce under some scheduling
  conditions; one notification is not a message counter.
- QEMU measurements cannot establish physical real-time
  performance guarantees.

## 11. Reproducibility

Microkit SDK: 2.4.0

Downloaded SDK archive SHA-256:

1ec1006235995a7d100a2b08b2b60cac264b8685208659fa7829342dd37e9a67

The recorded hash has not been independently verified
against an official published checksum.

Reference:
https://docs.sel4.systems/projects/microkit/

## 12. Next Experiment

Experiment 03: IPC Round-Trip Latency

Investigate synchronous protected procedure calls
between Protection Domains and measure communication
latency using a suitable timer.
