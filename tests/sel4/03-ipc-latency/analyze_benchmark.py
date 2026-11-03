from pathlib import Path
import csv
import math
import statistics

ROOT = Path(__file__).resolve().parents[3]
RESULTS = ROOT / "results/sel4/03-ipc-latency"

log = RESULTS / "ppc-benchmark-raw.log"
alternate = RESULTS / r"ppc-benchmark-raw\.log"

# Correct an accidental backslash in the filename.
if not log.exists() and alternate.exists():
    alternate.rename(log)
    print("Corrected raw log filename.")

if not log.exists():
    raise SystemExit("ERROR: Raw benchmark log not found")

lines = [
    line.strip()
    for line in log.read_text(errors="replace").splitlines()
]

if "BENCH: PASS - completed" not in lines:
    raise SystemExit("ERROR: Benchmark did not report completion")

if any("BENCH: FAIL" in line or "MON|ERROR" in line
       for line in lines):
    raise SystemExit("ERROR: Benchmark or Microkit fault found")

frequency_lines = [
    line for line in lines
    if line.startswith("BENCH: frequency_hz=")
]

if len(frequency_lines) != 1:
    raise SystemExit("ERROR: Invalid frequency metadata")

frequency = int(frequency_lines[0].split("=")[1])

if frequency <= 0:
    raise SystemExit("ERROR: Invalid counter frequency")

start = lines.index("CSV_BEGIN")
end = lines.index("CSV_END", start + 1)

rows = list(csv.reader(lines[start + 1:end]))

if not rows or rows[0] != ["type", "index", "ticks"]:
    raise SystemExit("ERROR: Invalid CSV header")

samples = {"ppc": [], "baseline": []}
clean_rows = []

for row in rows[1:]:
    if len(row) != 3:
        raise SystemExit(f"ERROR: Invalid row: {row}")

    kind, index_text, ticks_text = row

    if kind not in samples:
        raise SystemExit(f"ERROR: Unknown sample type: {kind}")

    index = int(index_text)
    ticks = int(ticks_text)

    if index != len(samples[kind]) or ticks < 0:
        raise SystemExit("ERROR: Invalid index or tick value")

    samples[kind].append(ticks)
    clean_rows.append((kind, index, ticks))

if len(samples["ppc"]) != 10000:
    raise SystemExit("ERROR: Expected 10000 PPC samples")

if len(samples["baseline"]) != 1000:
    raise SystemExit("ERROR: Expected 1000 baseline samples")

csv_path = RESULTS / "ppc-samples.csv"

with csv_path.open("w", newline="") as f:
    writer = csv.writer(f, lineterminator="\n")
    writer.writerow(["type", "index", "ticks"])
    writer.writerows(clean_rows)

def show_statistics(name, values):
    ordered = sorted(values)
    p99_index = math.ceil(0.99 * len(ordered)) - 1

    metrics = {
        "Minimum": ordered[0],
        "Mean": statistics.mean(values),
        "Median": statistics.median(values),
        "P99": ordered[p99_index],
        "Maximum": ordered[-1],
    }

    print(f"\n===== {name} =====")

    for label, ticks in metrics.items():
        microseconds = ticks * 1_000_000 / frequency
        print(f"{label:8}: {ticks:12.3f} ticks"
              f" | {microseconds:12.3f} us")

print("===== DATA VALIDATION PASSED =====")
print("Counter frequency:", frequency, "Hz")
print("PPC samples:", len(samples["ppc"]))
print("Baseline samples:", len(samples["baseline"]))
print("CSV saved:", csv_path)

show_statistics("PPC Round-Trip", samples["ppc"])
show_statistics("Timestamp Baseline", samples["baseline"])

if min(samples["ppc"]) == 0 or min(samples["baseline"]) == 0:
    print("\nWARNING: Zero-tick samples detected; investigate.")

print("\nNote: Baseline overhead has NOT been subtracted.")
print("Results describe exploratory QEMU measurements.")
