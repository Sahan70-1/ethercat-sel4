# Experiment 04 — Notification-Driven Shared-Memory Latency

## 1. Objective

Measure the round-trip latency of notification-driven
shared-memory communication between two seL4 Microkit
Protection Domains (PDs).

The experiment evaluates communication overhead relevant
to the seL4 component of our EtherCAT research project.

It does not measure EtherCAT network latency.

## 2. Experimental Environment

- Host: Apple Silicon MacBook Air M1
- Guest: Ubuntu ARM64 running through OrbStack
- Microkit SDK: 2.4.0
- Board: qemu_virt_aarch64
- Configuration: debug
- Target CPU: Cortex-A53
- Compiler optimization: -O2 (benchmark)
- Virtual machine: QEMU ARM64
- Shared-memory region: 4096 bytes
- Timer: ARM64 CNTPCT_EL0
- Counter frequency: 62,500,000 Hz
- Counter resolution: 16 ns per tick

The same software and QEMU configuration was used
across all three benchmark executions.

## 3. Communication Architecture

The system contains two Protection Domains:

### Master PD

- Writes a packet into shared memory.
- Publishes packet contents before notification.
- Records a start timestamp.
- Notifies the Driver.
- Receives an acknowledgment notification.
- Records the end timestamp.

### Driver PD

- Receives the request notification.
- Reads the shared-memory packet.
- Validates the sequence number and payload length.
- Validates every payload byte.
- Sends an acknowledgment notification.

Shared-memory permissions:

- Master: read/write
- Driver: read-only

Both PDs communicate over Microkit channel 1.

The protocol permits only one outstanding request.
The Master sends the next request after receiving
the previous acknowledgment.

This avoids overlapping transfers and notification
coalescing in the intended benchmark workload.

## 4. Payload Format

Each shared-memory packet contains:

- 32-bit sequence number
- 32-bit payload length
- Payload buffer of up to 1518 bytes

The payload contents are generated using a deterministic
byte pattern and verified by the Driver.

Payload sizes tested:

- 64 bytes
- 512 bytes
- 1518 bytes

These sizes represent shared-memory application payloads,
not actual Ethernet frames transmitted over a network.

## 5. Functional Verification

The initial functional test independently verified
all three payload sizes.

Results:

- 64 bytes: PASS
- 512 bytes: PASS
- 1518 bytes: PASS
- All three acknowledgments received

Evidence:

results/sel4/04-notification-latency/functional-verification.log

## 6. Benchmark Methodology

The benchmark uses a separate implementation located
in the benchmark/ directory.

The earlier pilot implementation is preserved in
benchmark/pilot/.

For each payload size:

- 1000 warm-up round trips
- 10000 measured round trips

Each independent QEMU execution therefore includes:

- 3000 warm-up round trips
- 30000 measured round trips
- 1000 timestamp baseline measurements

Three independent QEMU executions were recorded.

Total measured communication samples: 90000
Total timestamp baseline samples: 3000

### Measurement Boundaries

The Master prepares and publishes the payload before
recording the start timestamp.

The timed interval begins immediately before the
Master issues the request notification.

The Driver then validates the shared-memory packet
and sends an acknowledgment.

The interval ends when the Master's notification
handler reads the end timestamp.

Therefore, the measured round-trip interval includes:

- Request notification processing
- Protection Domain scheduling and switching
- Driver-side payload validation
- Acknowledgment notification processing
- Timestamp-read overhead

It excludes the Master's initial payload preparation
and debug output.

No normal debug printing occurs inside the timed
communication path.

The timestamp baseline is measured separately.
Baseline values are not subtracted from reported latency.

### Timer Conversion

Latency in microseconds is computed as:

latency_us = elapsed_ticks * 1000000 / 62500000

The P99 statistic uses the nearest-rank method.

## 7. Three-Run Results

All latency measurements below are in microseconds.

| Run | Payload | Min | Mean | Median | P99 | Max |
|-----|---------|-----|------|--------|-----|-----|
| 01 | 64 B | 21.456 | 35.072 | 32.400 | 69.792 | 98.912 |
| 01 | 512 B | 23.008 | 37.700 | 36.208 | 70.672 | 142.512 |
| 01 | 1518 B | 25.664 | 38.677 | 35.536 | 68.160 | 97.424 |
| 02 | 64 B | 21.872 | 37.094 | 35.968 | 70.672 | 97.712 |
| 02 | 512 B | 23.040 | 36.796 | 33.136 | 71.216 | 94.784 |
| 02 | 1518 B | 25.920 | 39.858 | 35.376 | 75.760 | 1087.856 |
| 03 | 64 B | 21.584 | 36.263 | 35.376 | 69.776 | 110.640 |
| 03 | 512 B | 22.800 | 36.143 | 34.496 | 67.696 | 91.616 |
| 03 | 1518 B | 25.664 | 37.777 | 35.168 | 62.064 | 356.464 |

### Timestamp Baseline

| Run | Median (us) | P99 (us) | Max (us) |
|-----|-------------|----------|----------|
| 01 | 0.080 | 0.128 | 5.664 |
| 02 | 0.080 | 0.128 | 5.200 |
| 03 | 0.088 | 0.128 | 5.744 |

## 8. Long-Tail Latency

Number of samples exceeding selected thresholds:

| Run | Payload | >50 us | >100 us | >200 us |
|-----|---------|--------|---------|---------|
| 01 | 64 B | 511 | 0 | 0 |
| 01 | 512 B | 867 | 1 | 0 |
| 01 | 1518 B | 1644 | 0 | 0 |
| 02 | 64 B | 760 | 0 | 0 |
| 02 | 512 B | 916 | 0 | 0 |
| 02 | 1518 B | 1745 | 16 | 14 |
| 03 | 64 B | 641 | 1 | 0 |
| 03 | 512 B | 585 | 0 | 0 |
| 03 | 1518 B | 1366 | 1 | 1 |

The largest observed latency was 1087.856 us in
Run 02 with the 1518-byte payload.

The measurements do not establish whether this
outlier originated from QEMU execution, host scheduling,
guest scheduling, or another source of interference.

## 9. Interpretation

Median round-trip latency across the nine tested
run/payload combinations ranged from:

32.400 us to 36.208 us.

P99 latency ranged from:

62.064 us to 75.760 us.

Larger payloads increased the Driver's validation work,
but median latency did not increase monotonically
across every run.

The existence of occasional high-latency samples is
important for subsequent real-time evaluation.

These measurements do not establish a worst-case
execution-time bound or a deadline guarantee.

## 10. Relationship to Experiment 03

Experiment 03 measured synchronous PPC round trips.

Its observed median range across three QEMU runs was:

24.920 us to 28.080 us.

Experiment 04 measured notification-driven shared-memory
round trips with Driver-side payload validation.

These experiments have different work inside the timed
intervals and cannot establish which communication
primitive is intrinsically faster.

A future controlled comparison should equalize
the workloads and isolate individual overheads.

## 11. Reproduction

Run commands from the repository root unless noted.

Microkit SDK:

~/sel4-research/tools/microkit-sdk-2.4.0

### Functional Test

cd tests/sel4/04-notification-latency

make MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0"

make run MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0"

Exit QEMU using Ctrl+A followed by X.

### Full Benchmark

From the repository root:

cd tests/sel4/04-notification-latency/benchmark

make MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0"

make run MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0"

The full benchmark prints the baseline and
communication samples after all measurements complete.

### Three-Run Analysis

From the repository root:

python3 tests/sel4/04-notification-latency/compare_runs.py

The analysis script verifies sample completeness,
timer frequency, and benchmark completion before
calculating statistics.

## 12. Evidence Files

Stored under:

results/sel4/04-notification-latency/

- functional-verification.log
- pilot-benchmark.log
- full-run01-raw.log
- full-run02-raw.log
- full-run03-raw.log
- run01-statistics.txt
- three-run-analysis.txt
- three-run-comparison.csv

## 13. Limitations

- Experiments were conducted under QEMU, not hardware.
- The Microkit configuration was debug.
- Host scheduling may influence timing results.
- The workload was executed on a virtual CPU.
- Only three independent executions were conducted.
- Payload sizes were tested in a fixed order.
- Driver payload validation is included in latency.
- No network devices or Ethernet frames were tested.
- No SOEM integration was measured.
- No EtherCAT cycle or deadline was enforced.
- No hard real-time latency guarantee was demonstrated.

The measurements are exploratory observations and
must not be interpreted as worst-case timing guarantees.

## 14. Next Work

- Preserve SHA-256 checksums of experimental evidence.
- Compare results with synchronous PPC measurements.
- Separate notification overhead from payload validation.
- Investigate scheduling interference and latency outliers.
- Evaluate periodic timing and network communication.
- Repeat measurements on appropriate physical hardware.
