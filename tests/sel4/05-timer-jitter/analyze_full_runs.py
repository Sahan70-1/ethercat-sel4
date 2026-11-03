from pathlib import Path
from statistics import mean, median
from math import ceil
import csv
import re

RESULTS = Path("results/sel4/05-timer-jitter")
CSV_PATH = RESULTS / "six-run-comparison.csv"
REPORT_PATH = RESULTS / "six-run-analysis.txt"

if CSV_PATH.exists() or REPORT_PATH.exists():
    raise SystemExit("ERROR: Analysis outputs already exist.")

FREQUENCY = 62500000
EXPECTED_SAMPLES = 10000
EXPECTED_PERIOD_TICKS = 62500

def locate(expected):
    matches = [
        p for p in RESULTS.iterdir()
        if p.is_file() and p.name.replace("\\", "") == expected
    ]
    if len(matches) != 1:
        raise ValueError(f"Expected exactly one log: {expected}")
    return matches[0]

def p99(values):
    ordered = sorted(values)
    return ordered[ceil(len(ordered) * 0.99) - 1]

def to_us(ticks):
    return ticks * 1000000 / FREQUENCY

def statistics(ticks):
    values = [to_us(x) for x in ticks]
    return {
        "min": min(values),
        "mean": mean(values),
        "median": median(values),
        "p99": p99(values),
        "max": max(values),
    }

def load(mode, run):
    expected = f"full-{mode}-run{run:02d}-raw.log"
    path = locate(expected)
    text = path.read_text(errors="replace")

    required = [
        "BENCH05: COMPLETE",
        "BENCH05_CSV_BEGIN",
        "BENCH05_CSV_END",
        f"BENCH05: period_ticks={EXPECTED_PERIOD_TICKS}",
        "BENCH05: early_interrupts=0",
    ]

    if any(marker not in text for marker in required):
        raise ValueError(f"Incomplete or invalid log: {expected}")

    match = re.search(
        r"BENCH05: skipped_periods=(\d+)", text
    )
    if not match:
        raise ValueError(f"Missing skipped-period total: {expected}")

    reported_skips = int(match.group(1))

    data = text.split("BENCH05_CSV_BEGIN", 1)[1]
    data = data.split("BENCH05_CSV_END", 1)[0]

    rows = []
    for line in data.splitlines():
        match = re.fullmatch(
            r"\s*(\d+),(\d+),(\d+),(\d+)\s*", line
        )
        if match:
            rows.append(tuple(map(int, match.groups())))

    valid = (
        len(rows) == EXPECTED_SAMPLES
        and [r[0] for r in rows]
            == list(range(1, EXPECTED_SAMPLES + 1))
        and all(r[1] > 0 for r in rows)
    )

    if not valid:
        raise ValueError(f"Sample validation failed: {expected}")

    intervals = [r[1] for r in rows]
    lateness = [r[2] for r in rows]
    skipped = [r[3] for r in rows]

    measured_skips = sum(skipped)
    warmup_skips = reported_skips - measured_skips

    if warmup_skips < 0:
        raise ValueError(f"Skipped-period count mismatch: {expected}")

    return {
        "source": expected,
        "intervals": intervals,
        "lateness": lateness,
        "skipped": skipped,
        "warmup_skips": warmup_skips,
    }

datasets = {}
records = []
lines = [
    "EXPERIMENT 05 - PERIODIC TIMER JITTER",
    "Six-run benchmark statistical analysis",
    "",
    "Board: qemu_virt_aarch64",
    "Requested timer period: 1000 us",
    "Counter frequency: 62500000 Hz",
    "Warmup callbacks per run: 1000",
    "Measured callbacks per run: 10000",
    "P99 calculation: nearest rank",
    "",
]

for mode in ("default", "icount"):
    for run in (1, 2, 3):
        dataset = load(mode, run)
        datasets[(mode, run)] = dataset

        interval = statistics(dataset["intervals"])
        lateness = statistics(dataset["lateness"])

        skips = sum(dataset["skipped"])
        callbacks_with_skips = sum(
            value > 0 for value in dataset["skipped"]
        )

        record = {
            "clock_mode": mode,
            "run": run,
            "samples": EXPECTED_SAMPLES,
            "interval_min_us": round(interval["min"], 3),
            "interval_mean_us": round(interval["mean"], 3),
            "interval_median_us": round(interval["median"], 3),
            "interval_p99_us": round(interval["p99"], 3),
            "interval_max_us": round(interval["max"], 3),
            "lateness_min_us": round(lateness["min"], 3),
            "lateness_mean_us": round(lateness["mean"], 3),
            "lateness_median_us": round(lateness["median"], 3),
            "lateness_p99_us": round(lateness["p99"], 3),
            "lateness_max_us": round(lateness["max"], 3),
            "measured_skipped_deadlines": skips,
            "callbacks_with_skips": callbacks_with_skips,
            "warmup_skipped_deadlines": dataset["warmup_skips"],
            "source_log": dataset["source"],
        }
        records.append(record)

        lines.extend([
            f"{mode.upper()} RUN {run:02d}",
            f"  Samples: {EXPECTED_SAMPLES}",
            f"  Interval mean: {interval['mean']:.3f} us",
            f"  Interval median: {interval['median']:.3f} us",
            f"  Interval P99: {interval['p99']:.3f} us",
            f"  Interval maximum: {interval['max']:.3f} us",
            f"  Lateness mean: {lateness['mean']:.3f} us",
            f"  Lateness P99: {lateness['p99']:.3f} us",
            f"  Lateness maximum: {lateness['max']:.3f} us",
            f"  Measured skipped deadlines: {skips}",
            f"  Callbacks with skips: {callbacks_with_skips}",
            f"  Warmup skipped deadlines: {dataset['warmup_skips']}",
            "",
        ])

for mode in ("default", "icount"):
    group = [datasets[(mode, run)] for run in (1, 2, 3)]

    intervals = [
        value for d in group for value in d["intervals"]
    ]
    lateness = [
        value for d in group for value in d["lateness"]
    ]
    skipped = [
        value for d in group for value in d["skipped"]
    ]

    interval_stats = statistics(intervals)
    lateness_stats = statistics(lateness)

    lines.extend([
        f"COMBINED {mode.upper()} RESULTS",
        f"  Samples: {len(intervals)}",
        f"  Mean interval: {interval_stats['mean']:.3f} us",
        f"  Median interval: {interval_stats['median']:.3f} us",
        f"  P99 interval: {interval_stats['p99']:.3f} us",
        f"  Maximum interval: {interval_stats['max']:.3f} us",
        f"  Mean lateness: {lateness_stats['mean']:.3f} us",
        f"  P99 lateness: {lateness_stats['p99']:.3f} us",
        f"  Maximum lateness: {lateness_stats['max']:.3f} us",
        f"  Measured skipped deadlines: {sum(skipped)}",
        f"  Callbacks with skips: {sum(x > 0 for x in skipped)}",
        "",
    ])

lines.extend([
    "INTERPRETATION AND LIMITATIONS",
    "Default TCG uses the normal QEMU timing model.",
    "Instruction-counted TCG uses -icount shift=3,sleep=off.",
    "Instruction-counted time is artificial virtual time.",
    "The two timing models must be reported separately.",
    "Skipped deadlines are calculated by benchmark software.",
    "The benchmark does not measure EtherCAT network traffic.",
    "QEMU results do not establish physical WCET guarantees.",
    "A measured maximum is not a proven worst-case bound.",
])

with CSV_PATH.open("x", newline="") as output:
    writer = csv.DictWriter(
        output, fieldnames=list(records[0].keys())
    )
    writer.writeheader()
    writer.writerows(records)

REPORT_PATH.write_text("\n".join(lines) + "\n")

print("===== EXPERIMENT 05 ANALYSIS =====")
for record in records:
    print(
        f"{record['clock_mode']} run{record['run']:02d}: "
        f"mean={record['interval_mean_us']:.3f} us, "
        f"p99={record['interval_p99_us']:.3f} us, "
        f"skips={record['measured_skipped_deadlines']}"
    )

print()
print("Validated runs:", len(records))
print("Validated samples:", sum(r["samples"] for r in records))
print("Comparison CSV:", CSV_PATH)
print("Analysis report:", REPORT_PATH)
print("ANALYSIS: PASS")
