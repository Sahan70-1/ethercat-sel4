from pathlib import Path
import csv
import math
import statistics

ROOT = Path(__file__).resolve().parents[3]
RESULTS = ROOT / "results/sel4/03-ipc-latency"

RUNS = {
    "run01": "ppc-benchmark-raw.log",
    "run02": "ppc-run02-raw.log",
    "run03": "ppc-run03-raw.log",
}

def find_log(expected):
    matches = [
        p for p in RESULTS.iterdir()
        if p.is_file()
        and p.name.replace("\\", "") == expected
    ]
    if len(matches) != 1:
        raise SystemExit(
            f"Expected exactly one log for {expected}, found {len(matches)}"
        )
    return matches[0]

def load_run(path):
    lines = [
        line.strip()
        for line in path.read_text(errors="replace").splitlines()
    ]

    if lines.count("BENCH: PASS - completed") != 1:
        raise SystemExit(f"Missing completion marker: {path}")

    if any("BENCH: FAIL" in x or "MON|ERROR" in x
           for x in lines):
        raise SystemExit(f"Fault detected: {path}")

    frequencies = [
        x for x in lines
        if x.startswith("BENCH: frequency_hz=")
    ]
    if len(frequencies) != 1:
        raise SystemExit(f"Invalid frequency metadata: {path}")

    frequency = int(frequencies[0].split("=")[1])
    if frequency <= 0:
        raise SystemExit(f"Invalid counter frequency: {path}")

    if lines.count("CSV_BEGIN") != 1 or lines.count("CSV_END") != 1:
        raise SystemExit(f"Invalid CSV markers: {path}")

    start = lines.index("CSV_BEGIN")
    end = lines.index("CSV_END")
    if end <= start:
        raise SystemExit(f"Invalid CSV marker order: {path}")

    rows = list(csv.reader(lines[start + 1:end]))
    if not rows or rows[0] != ["type", "index", "ticks"]:
        raise SystemExit(f"Invalid CSV header: {path}")

    data = {"ppc": [], "baseline": []}

    for row in rows[1:]:
        if len(row) != 3:
            raise SystemExit(f"Malformed CSV row: {row}")

        kind, index_text, ticks_text = row
        if kind not in data:
            raise SystemExit(f"Unknown sample type: {kind}")

        index = int(index_text)
        ticks = int(ticks_text)

        if index != len(data[kind]) or ticks < 0:
            raise SystemExit(f"Invalid index or ticks in {path}")

        data[kind].append(ticks)

    if len(data["ppc"]) != 10000:
        raise SystemExit(f"Wrong PPC sample count: {path}")

    if len(data["baseline"]) != 1000:
        raise SystemExit(f"Wrong baseline count: {path}")

    return frequency, data

def summary(values, frequency):
    ordered = sorted(values)
    scale = 1_000_000 / frequency

    return {
        "min_us": ordered[0] * scale,
        "mean_us": statistics.mean(values) * scale,
        "median_us": statistics.median(values) * scale,
        "p99_us": ordered[math.ceil(0.99 * len(ordered)) - 1] * scale,
        "max_us": ordered[-1] * scale,
    }

all_rows = []
summary_rows = []
frequencies = set()

for run, filename in RUNS.items():
    path = find_log(filename)
    frequency, data = load_run(path)
    frequencies.add(frequency)

    ppc = summary(data["ppc"], frequency)
    baseline = summary(data["baseline"], frequency)

    counts = {
        f"above_{threshold}us": sum(
            ticks * 1_000_000 / frequency > threshold
            for ticks in data["ppc"]
        )
        for threshold in (50, 100, 200)
    }

    summary_rows.append({
        "run": run,
        "frequency_hz": frequency,
        **ppc,
        **counts,
        "baseline_median_us": baseline["median_us"],
        "baseline_p99_us": baseline["p99_us"],
        "baseline_max_us": baseline["max_us"],
    })

    for kind in ("ppc", "baseline"):
        for index, ticks in enumerate(data[kind]):
            all_rows.append((run, kind, index, ticks))

    print(f"Validated {run}: {path.name}")
    print("  PPC:", len(data["ppc"]),
          "| Baseline:", len(data["baseline"]))

if len(frequencies) != 1:
    raise SystemExit("Counter frequency differs between runs")

combined = RESULTS / "ppc-all-runs.csv"
with combined.open("w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["run", "type", "index", "ticks"])
    writer.writerows(all_rows)

comparison = RESULTS / "ppc-runs-comparison.csv"
with comparison.open("w", newline="") as f:
    writer = csv.DictWriter(
        f, fieldnames=list(summary_rows[0].keys())
    )
    writer.writeheader()
    writer.writerows(summary_rows)

print("\n===== THREE-RUN PPC COMPARISON =====")
print(
    f"{'Run':<8} {'Min':>9} {'Mean':>9} "
    f"{'Median':>9} {'P99':>9} {'Max':>9}"
)
print("All latency values in microseconds")

for row in summary_rows:
    print(
        f"{row['run']:<8} "
        f"{row['min_us']:>9.3f} "
        f"{row['mean_us']:>9.3f} "
        f"{row['median_us']:>9.3f} "
        f"{row['p99_us']:>9.3f} "
        f"{row['max_us']:>9.3f}"
    )

print("\n===== LATENCY TAIL =====")
for row in summary_rows:
    print(
        f"{row['run']}: "
        f">50us={row['above_50us']}, "
        f">100us={row['above_100us']}, "
        f">200us={row['above_200us']}"
    )

print("\n===== TIMESTAMP BASELINE =====")
for row in summary_rows:
    print(
        f"{row['run']}: "
        f"median={row['baseline_median_us']:.3f} us, "
        f"p99={row['baseline_p99_us']:.3f} us, "
        f"max={row['baseline_max_us']:.3f} us"
    )

print("\n===== OUTPUT FILES =====")
print(comparison)
print(combined)
print("\nAll three runs validated.")
