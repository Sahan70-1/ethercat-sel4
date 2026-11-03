from pathlib import Path
from statistics import mean, median
from math import ceil
import csv

RESULTS = Path("results/sel4/04-notification-latency")
RUNS = ("run01", "run02", "run03")
SIZES = (64, 512, 1518)
EXPECTED_SAMPLES = 10000
EXPECTED_BASELINE = 1000
FREQUENCY = 62500000

def find_log(run):
    expected = f"full-{run}-raw.log"
    matches = [
        p for p in RESULTS.iterdir()
        if p.name.replace("\\", "") == expected
    ]
    if len(matches) != 1:
        raise RuntimeError(f"Log not uniquely found: {expected}")
    return matches[0]

def parse_run(run):
    path = find_log(run)

    samples = {size: {} for size in SIZES}
    baseline = {}
    frequencies = []
    passes = 0

    for line in path.read_text(errors="replace").splitlines():
        line = line.strip()

        if "BENCH: FAIL" in line:
            raise RuntimeError(f"{run}: benchmark failure")

        if line.startswith("BENCH: FREQ_HZ,"):
            frequencies.append(int(line.split(",")[1]))

        if line == "BENCH: PASS - 30000 notification round trips":
            passes += 1

        if line.startswith("BENCH: CSV,"):
            parts = line.split(",")

            if parts[1] == "size_bytes":
                continue

            if len(parts) != 4:
                raise RuntimeError(f"{run}: malformed CSV")

            size, index, ticks = map(int, parts[1:])

            if size not in samples:
                raise RuntimeError(f"{run}: unexpected payload size")

            if index in samples[size]:
                raise RuntimeError(f"{run}: duplicate sample")

            samples[size][index] = ticks

        if line.startswith("BENCH: BASELINE,"):
            parts = line.split(",")

            if parts[1] == "index":
                continue

            if len(parts) != 3:
                raise RuntimeError(f"{run}: malformed baseline")

            index, ticks = map(int, parts[1:])

            if index in baseline:
                raise RuntimeError(f"{run}: duplicate baseline")

            baseline[index] = ticks

    if passes != 1 or frequencies != [FREQUENCY]:
        raise RuntimeError(f"{run}: invalid status or frequency")

    for size in SIZES:
        if sorted(samples[size]) != list(range(EXPECTED_SAMPLES)):
            raise RuntimeError(f"{run}: incomplete {size}-byte samples")

    if sorted(baseline) != list(range(EXPECTED_BASELINE)):
        raise RuntimeError(f"{run}: incomplete baseline")

    return samples, baseline

def us(ticks):
    return ticks * 1000000 / FREQUENCY

def nearest_rank(values, percentile):
    ordered = sorted(values)
    return ordered[ceil(len(ordered) * percentile) - 1]

def calculate(values):
    return {
        "min_us": us(min(values)),
        "mean_us": us(mean(values)),
        "median_us": us(median(values)),
        "p99_us": us(nearest_rank(values, 0.99)),
        "max_us": us(max(values)),
        "over50": sum(us(t) > 50 for t in values),
        "over100": sum(us(t) > 100 for t in values),
        "over200": sum(us(t) > 200 for t in values),
    }

rows = []

print("EXPERIMENT 04 - THREE-RUN COMPARISON")
print("Counter frequency: 62500000 Hz")
print("Timer resolution: 16 ns per tick")
print("P99: nearest-rank method")
print("Baseline subtraction: none")
print("Warm-up: 1000 transfers per payload per run")
print("Measured: 10000 transfers per payload per run")

for run in RUNS:
    samples, baseline = parse_run(run)

    print(f"\n===== {run.upper()} =====")
    print("Validation: PASS")

    b = calculate(list(baseline.values()))
    print(
        f"Baseline: median={b['median_us']:.3f} us, "
        f"P99={b['p99_us']:.3f} us, "
        f"max={b['max_us']:.3f} us"
    )

    for size in SIZES:
        stats = calculate(list(samples[size].values()))

        print(
            f"{size:4d} bytes | "
            f"min={stats['min_us']:.3f} | "
            f"mean={stats['mean_us']:.3f} | "
            f"median={stats['median_us']:.3f} | "
            f"P99={stats['p99_us']:.3f} | "
            f"max={stats['max_us']:.3f} us"
        )

        print(
            f"  Tail counts: "
            f">50us={stats['over50']}, "
            f">100us={stats['over100']}, "
            f">200us={stats['over200']}"
        )

        rows.append({
            "run": run,
            "payload_bytes": size,
            "sample_count": EXPECTED_SAMPLES,
            **stats
        })

output = RESULTS / "three-run-comparison.csv"

fields = [
    "run", "payload_bytes", "sample_count",
    "min_us", "mean_us", "median_us",
    "p99_us", "max_us",
    "over50", "over100", "over200"
]

with output.open("w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=fields)
    writer.writeheader()

    for row in rows:
        formatted = row.copy()
        for key in (
            "min_us", "mean_us", "median_us",
            "p99_us", "max_us"
        ):
            formatted[key] = f"{row[key]:.3f}"
        writer.writerow(formatted)

print("\n===== FINAL VALIDATION =====")
print("Independent runs:", len(RUNS))
print("Total timed samples:", len(RUNS) * len(SIZES) * EXPECTED_SAMPLES)
print("Total baseline samples:", len(RUNS) * EXPECTED_BASELINE)
print("CSV:", output)
print("ANALYSIS: PASS - all three runs verified")
