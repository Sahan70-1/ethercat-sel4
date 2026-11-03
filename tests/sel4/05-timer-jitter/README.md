# Experiment 05 — Periodic Timer Jitter and Real-Time Behavior

## 1. Research objective

This experiment investigates periodic ARM timer interrupt delivery
to a seL4 Microkit Protection Domain (PD).

The primary target is a 1 ms period (1 kHz), relevant to
cyclic industrial communication workloads such as EtherCAT.

The experiment measures:

- Observed intervals between timer callbacks
- Lateness relative to programmed timer deadlines
- Skipped scheduled deadlines
- Variation across independent QEMU boots
- Sensitivity to QEMU's virtual-clock configuration

This experiment does not transmit EtherCAT frames.

## 2. Experimental environment

- Development host: Apple MacBook Air M1
- Guest environment: OrbStack Ubuntu 24.04 ARM64
- Microkit SDK: 2.4.0
- Microkit board: qemu_virt_aarch64
- Microkit configuration: debug
- QEMU: 8.2.2
- Emulated processor: Cortex-A53
- QEMU machine: virt,virtualization=on
- Interrupt controller: ARM GICv2-compatible
- Interrupt source: ARM generic physical timer, IRQ 30
- Physical counter: CNTPCT_EL0
- Timer comparison register: CNTP_CVAL_EL0
- Timer control register: CNTP_CTL_EL0
- Timer frequency: 62,500,000 Hz

The Ubuntu environment does not expose /dev/kvm.
QEMU therefore uses software emulation (TCG).

## 3. Hardware timer verification

The initial one-shot test is implemented in:

- timer.c
- system.system
- Makefile

The PD programs a one-shot physical timer and handles
the interrupt through Microkit channel 0.

The system configuration assigns:

    <irq irq="30" id="0" trigger="level" />

The successful test printed:

    TIMER: initialized
    TIMER: one-shot armed
    TIMER: PASS - physical timer interrupt received

Evidence:

    results/sel4/05-timer-jitter/one-shot-test.log

This verifies that the physical timer can be programmed and
that its interrupt reaches the PD in this QEMU configuration.

## 4. Timer benchmark architecture

The benchmark uses one Protection Domain.

Its timer interrupt is delivered to notified(channel).

The benchmark reads CNTPCT_EL0 when the callback executes.

A timer comparison value is programmed using CNTP_CVAL_EL0.

The requested interval is:

    62,500 counter ticks = 1 ms

The main benchmark uses CVAL-only rearming, followed by
Microkit interrupt acknowledgment.

The PD scheduling configuration is:

    budget="10000"
    period="10000"

Both scheduling parameters are expressed in microseconds.

The benchmark uses absolute timer deadlines, rather than
scheduling the next timer relative to callback completion.

## 5. Measurement methodology

Each full run contains:

- 1,000 warmup timer callbacks
- 10,000 measured timer callbacks
- 1 ms requested timer period
- ARM physical counter timestamps
- No serial output during the timed measurement loop

The measured data is stored in static arrays.

The results are printed after measurement completion.

Each CSV sample contains:

    sample,interval_ticks,lateness_ticks,skipped_periods

Definitions:

interval_ticks:
    Counter difference between successive callbacks.

lateness_ticks:
    Counter difference between the callback timestamp
    and the scheduled timer deadline, clamped to zero.

skipped_periods:
    Number of complete scheduled timer periods advanced
    past when calculating the next deadline.

The skipped-period count is a software-derived metric.
It is not an independent count of lost hardware interrupts.

Conversion:

    microseconds = ticks / 62.5

P99 statistics use the nearest-rank method.

Measured callback interval is not the same as timer
interrupt service time or network-cycle execution time.

## 6. Pilot experiments

Seven pilot configurations were evaluated.

Each pilot used 10 warmup callbacks and 50 measured samples.

| Configuration | Requested period | Mean observed interval (us) | Skipped deadlines |
|---|---:|---:|---:|
| Default PD budget | 1 ms | 3000.369 | 100 |
| 10 ms PD budget | 1 ms | 3000.598 | 100 |
| 10 ms PD budget | 2 ms | 3002.415 | 25 |
| 10 ms PD budget | 3 ms | 3001.119 | 0 |
| 10 ms PD budget | 5 ms | 4971.430 | 0 |
| CVAL-only rearming | 1 ms | 3000.873 | 100 |
| Instruction-counted TCG | 1 ms | 1000.000 | 0 |

All seven pilots completed and validated.

The 2 ms pilot showed a strictly alternating skipped-period
pattern of 0,1,0,1 across 50 callbacks.

Changing the PD budget and removing timer disable/enable
operations did not eliminate the approximately 3 ms cadence.

The pilot dataset is saved in:

    results/sel4/05-timer-jitter/pilot-comparison.csv

Pilot implementations are preserved in separate directories.

## 7. Full benchmark configurations

The same compiled benchmark loader image was used for both
QEMU timing modes.

Mode A: Default TCG

    qemu-system-aarch64
    -machine virt,virtualization=on
    -cpu cortex-a53

Mode B: Instruction-counted TCG

    qemu-system-aarch64
    -accel tcg,thread=single
    -icount shift=3,sleep=off
    -machine virt,virtualization=on
    -cpu cortex-a53

Both use the same timer source, IRQ and scheduling parameters.

The instruction-counted mode advances guest virtual time
according to QEMU's selected instruction-counting model.

It is not calibrated physical execution time.

Three independent QEMU boots were captured per mode.

Total measured callbacks:

    3 default runs x 10,000 = 30,000
    3 icount runs x 10,000 = 30,000

    Total = 60,000 measured callbacks

Warmup callbacks are excluded from measured statistics.

## 8. Full benchmark — Default TCG

| Metric | Run 01 | Run 02 | Run 03 |
|---|---:|---:|---:|
| Samples | 10000 | 10000 | 10000 |
| Mean interval (us) | 2945.101 | 2962.040 | 2860.400 |
| Median interval (us) | 2999.920 | 2999.872 | 2999.824 |
| P99 interval (us) | 3020.096 | 3085.872 | 3021.024 |
| Maximum interval (us) | 5023.440 | 10247.584 | 7456.352 |
| Skipped deadlines | 19451 | 19621 | 18604 |

Combined default-TCG statistics:

- Samples: 30,000
- Mean callback interval: 2922.514 us
- Median callback interval: 2999.856 us
- P99 callback interval: 3038.112 us
- Maximum callback interval: 10247.584 us
- Mean deadline lateness: 2378.624 us
- P99 deadline lateness: 2642.416 us
- Maximum deadline lateness: 9962.608 us
- Measured skipped deadlines: 57,676
- Callbacks with at least one skipped deadline: 28,821

The approximately 3 ms median interval appeared consistently
across all three independent runs.

However, significant tail-latency outliers also occurred.

## 9. Full benchmark — Instruction-counted TCG

All three runs produced identical measured statistics:

- Samples per run: 10,000
- Minimum interval: 1000.000 virtual us
- Mean interval: 1000.000 virtual us
- Median interval: 1000.000 virtual us
- P99 interval: 1000.000 virtual us
- Maximum interval: 1000.000 virtual us
- Deadline lateness: 4.768 virtual us
- Skipped deadlines: 0

Combined instruction-counted statistics:

- Samples: 30,000
- Mean callback interval: 1000.000 virtual us
- Median callback interval: 1000.000 virtual us
- P99 callback interval: 1000.000 virtual us
- Maximum callback interval: 1000.000 virtual us
- Mean deadline lateness: 4.768 virtual us
- Skipped deadlines: 0

These deterministic measurements are properties of the selected
QEMU virtual-time model.

They are not physical jitter or hardware WCET measurements.

## 10. Interpretation

The timer interrupt mechanism was functionally verified.

Default TCG showed approximately 3 ms median callback
intervals under a requested 1 ms periodic workload.

Instruction-counted TCG produced exactly 1 ms virtual
intervals using the same benchmark loader.

Therefore, changing QEMU's timing model materially changes
the observed timing results.

The comparison does not isolate every possible source of
delay within QEMU, seL4 or the guest interrupt path.

The default-TCG 3 ms cadence must not be described as an
intrinsic minimum timer period of seL4.

Likewise, the instruction-counted results must not be
presented as proof of hard real-time performance.

## 11. Limitations

- QEMU TCG is not cycle-accurate ARM hardware.
- The virtual timer model depends on QEMU configuration.
- Host scheduling and virtualization may influence results.
- Instruction-counted virtual time is artificial.
- Only the QEMU ARM64 target was measured.
- No hardware-based worst-case latency bound was established.
- A measured maximum is not a formal WCET guarantee.
- Debug Microkit configuration was used.
- No EtherCAT frames were transmitted.
- No Ethernet driver or network stack was benchmarked.
- No real EtherCAT slave cycle was measured.
- Timer-callback measurements do not establish end-to-end
  EtherCAT cycle deadlines.

Physical-hardware experiments are needed before making
claims about real-world 1 kHz deadline guarantees.

## 12. Building

Run from the repository root inside Ubuntu OrbStack.

Set the SDK path:

    export MICROKIT_SDK="$HOME/sel4-research/tools/microkit-sdk-2.4.0"

Build:

    make -C tests/sel4/05-timer-jitter/benchmark \
      MICROKIT_SDK="$MICROKIT_SDK"

Generated image:

    tests/sel4/05-timer-jitter/benchmark/build/loader.img

Build products are excluded from Git.

## 13. Reproducing the benchmark

The launcher is:

    tests/sel4/05-timer-jitter/run_full_benchmark.py

To inspect a command without starting QEMU:

    python3 tests/sel4/05-timer-jitter/run_full_benchmark.py \
      --mode default --run 1 --dry-run

    python3 tests/sel4/05-timer-jitter/run_full_benchmark.py \
      --mode icount --run 1 --dry-run

For a fresh experiment using an unused output location,
the corresponding command is:

    python3 tests/sel4/05-timer-jitter/run_full_benchmark.py \
      --mode default --run 1

or:

    python3 tests/sel4/05-timer-jitter/run_full_benchmark.py \
      --mode icount --run 1

The launcher intentionally refuses to overwrite existing logs.

To repeat archived run numbers, use a separate fresh checkout
or an independently archived results directory.

Never overwrite the original raw evidence.

The launcher stops QEMU after detecting the final CSV marker.

## 14. Statistical analysis

Analysis script:

    tests/sel4/05-timer-jitter/analyze_full_runs.py

The script validates:

- Completion markers
- Exactly 10,000 samples per run
- Sequential sample IDs
- Positive callback intervals
- Timer period and early-interrupt indicators

It generates:

    results/sel4/05-timer-jitter/six-run-comparison.csv

    results/sel4/05-timer-jitter/six-run-analysis.txt

Analysis outputs are protected against accidental overwrite.

To rerun the analysis, work with a fresh results copy that
does not already contain generated analysis outputs.

## 15. Evidence organization

Source code:

    tests/sel4/05-timer-jitter/

Recorded evidence:

    results/sel4/05-timer-jitter/

Evidence includes:

- Successful one-shot timer verification
- Seven pilot raw logs and pilot comparison
- Six full benchmark raw logs
- Six-run comparison CSV
- Six-run statistical analysis

A SHA-256 manifest is generated after documentation is finalized.

## 16. Research conclusion

This experiment demonstrates functional ARM physical timer
interrupt handling by a seL4 Microkit Protection Domain.

It also demonstrates that the observed timing depends
strongly on the QEMU clock model used for evaluation.

Default TCG did not meet the requested 1 ms callback cadence
in these measurements.

Instruction-counted TCG produced deterministic 1 ms
virtual intervals, but those results cannot establish
real-hardware real-time guarantees.

The correct conclusion is a virtualized timing-model
sensitivity finding, not a hardware determinism claim.

Further physical-hardware testing is required for
end-to-end EtherCAT timing guarantees.
