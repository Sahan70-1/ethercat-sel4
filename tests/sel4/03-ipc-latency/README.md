# Experiment 03 — Synchronous IPC Round-Trip Latency

## 1. Objective

Measure the synchronous protected procedure call (PPC)
round-trip latency between two seL4 Microkit Protection
Domains (PDs).

This experiment evaluates inter-PD communication overhead
for the seL4 component of the EtherCAT research project.

It does not measure EtherCAT network transmission latency.

## 2. Experimental Environment

- Host: Apple Silicon MacBook Air M1
- Linux environment: Ubuntu 24.04 ARM64 via OrbStack
- Microkit SDK: 2.4.0
- Microkit board: qemu_virt_aarch64
- Microkit configuration: debug
- Emulator: QEMU AArch64, Cortex-A53
- QEMU configuration: virtualization enabled
- Active virtual CPUs: 1
- Compiler: AArch64 GCC with -O2 optimization
- Timing source: ARM CNTPCT_EL0 physical counter
- Measured counter frequency: 62,500,000 Hz
- Nominal counter tick duration: 16 ns

## 3. Architecture

Two Protection Domains communicate over a synchronous
Microkit PPC channel.

Master PD:
- Sends the PPC request through channel 1.
- Measures elapsed counter ticks around microkit_ppcall().
- Validates the returned message outside the timed interval.
- Stores measured samples in memory.

Driver PD:
- Implements the protected() entry point.
- Checks the request label, message count and payload.
- Returns the expected response.
- Performs no console printing in the PPC handler.

Communication path:

Master PD -> synchronous PPC -> Driver PD -> Master PD

The timed interval includes the PPC call, Driver processing,
return path and timestamp instructions.

It excludes request-register setup, response validation and
console printing.

## 4. Functional Verification

The separate functional test confirmed:
- Master and Driver initialization.
- Successful synchronous PPC invocation.
- Correct request and response validation.

An initial virtual counter read using CNTVCT_EL0
caused a vCPU fault in this configuration.

The physical counter CNTPCT_EL0 was subsequently
tested successfully.

## 5. Benchmark Methodology

Each independent benchmark execution performs:

- 1,000 PPC warm-up calls.
- 10,000 timed PPC round trips.
- 1,000 timestamp-baseline measurements.
- In-memory storage of measurement samples.
- CSV output after measurements finish.

Three independent QEMU executions were performed.

Total dataset:
- 30,000 PPC latency measurements.
- 3,000 timestamp-baseline measurements.

Latency conversion:

latency_us = ticks * 1,000,000 / counter_frequency_hz

P99 uses the nearest-rank percentile method.

No fixed timestamp overhead is subtracted from PPC results.

## 6. Measured PPC Results

All latency values are in microseconds.

| Metric | Run 01 | Run 02 | Run 03 |
|---|---:|---:|---:|
| Minimum | 17.040 | 17.840 | 17.456 |
| Mean | 27.545 | 29.059 | 27.597 |
| Median | 24.920 | 28.080 | 26.832 |
| P99 | 52.752 | 61.248 | 52.336 |
| Maximum | 201.168 | 416.832 | 153.920 |

## 7. Latency Tail

Number of PPC measurements exceeding each threshold:

| Threshold | Run 01 | Run 02 | Run 03 |
|---|---:|---:|---:|
| Above 50 us | 133 | 230 | 133 |
| Above 100 us | 2 | 19 | 1 |
| Above 200 us | 1 | 3 | 0 |

Run 02 showed higher median, P99 and maximum latency
than Runs 01 and 03.

## 8. Timestamp Baseline

Baseline median: 0.096 us in all three runs.
Baseline P99: 0.128 us in all three runs.

Maximum timestamp-baseline intervals:
- Run 01: 6.448 us
- Run 02: 6.784 us
- Run 03: 179.376 us

Occasional large baseline intervals indicate that
measurement execution can experience delays independently
of PPC operations.

Their exact cause was not established.

## 9. Reproduction

From the repository root on Ubuntu ARM64:

    cd tests/sel4/03-ipc-latency/benchmark

    export MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0"

    make clean MICROKIT_SDK="$MICROKIT_SDK"
    make MICROKIT_SDK="$MICROKIT_SDK"

To run QEMU interactively:

    make run MICROKIT_SDK="$MICROKIT_SDK"

To capture a fresh run from the repository root:

    timeout 180s make -C tests/sel4/03-ipc-latency/benchmark run \
      MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0" \
      > results/sel4/03-ipc-latency/ppc-reproduction.log 2>&1

QEMU may remain running after BENCH: PASS - completed.
A timeout exit status of 124 does not by itself indicate
benchmark failure or success.

To analyze the three archived runs:

    python3 tests/sel4/03-ipc-latency/compare_runs.py

## 10. Evidence

The results/sel4/03-ipc-latency directory contains:

- ppc-functional.log
- timer-verification.log
- ppc-benchmark-raw.log (Run 01)
- ppc-run02-raw.log
- ppc-run03-raw.log
- ppc-all-runs.csv
- ppc-runs-comparison.csv
- three-run-analysis.txt
- ppc-run01-sha256.txt

The checksum manifest records hashes for the selected
raw logs, derived data and benchmark source files.

## 11. Interpretation and Limitations

The experiment demonstrates successful synchronous IPC
measurement between two seL4 Microkit Protection Domains
under QEMU AArch64.

Median PPC round-trip latency ranged from 24.920 to
28.080 us across the three executions.

Occasional higher-latency samples occurred, including a
maximum of 416.832 us in Run 02.

Important limitations:

- QEMU timing is not equivalent to physical hardware timing.
- Only one active virtual CPU was used.
- The experiment uses a debug Microkit configuration.
- macOS host scheduling and emulator execution may affect timing.
- Three executions do not establish a worst-case latency bound.
- No real-time deadline guarantee was demonstrated.
- No EtherCAT frames or network packets were transmitted.
- No SOEM integration was measured.

These results are exploratory IPC measurements and should
not be interpreted as guaranteed worst-case execution times.

## 12. Next Work

Investigate notification and shared-memory communication
latency, compare IPC mechanisms, and subsequently evaluate
network communication and real-time behavior.
