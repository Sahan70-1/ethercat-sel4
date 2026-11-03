from pathlib import Path
from statistics import mean, median
from math import ceil

RESULT_DIR = Path("results/sel4/04-notification-latency")
EXPECTED_SIZES = (64, 512, 1518)
SAMPLES_PER_SIZE = 10000
BASELINE_COUNT = 1000

# Find the log even if a filename contains an
# unintended literal backslash.
matches = [
    p for p in RESULT_DIR.iterdir()
    if p.name.replace("\\", "") == "full-run01-raw.log"
]

if len(matches) != 1:
    raise SystemExit("ERROR: Run 01 log not uniquely found")

log_file = matches[0]

samples = {size: {} for size in EXPECTED_SIZES}
baseline = {}
frequencies = set()
passed = False

for raw_line in log_file.read_text(
    errors="replace"
).splitlines():

    line = raw_line.strip()

    if "BENCH: FAIL" in line:
        raise SystemExit("ERROR: Benchmark failure in log")

    if line.startswith("BENCH: FREQ_HZ,"):
        frequencies.add(int(line.split(",")[1]))

    if "BENCH: PASS - 30000 notification round trips" in line:
        passed = True

    if line.startswith("BENCH: CSV,"):
        parts = line.split(",")

        if len(parts) != 4 or not parts[1].isdigit():
            continue

        size, index, ticks = map(int, parts[1:])

        if size not in samples or index in samples[size]:
            raise SystemExit("ERROR: Invalid or duplicate sample")

        samples[size][index] = ticks

    if line.startswith("BENCH: BASELINE,"):
        parts = line.split(",")

        if len(parts) != 3 or not parts[1].isdigit():
            continue

        index, ticks = map(int, parts[1:])

        if index in baseline:
            raise SystemExit("ERROR: Duplicate baseline sample")

        baseline[index] = ticks

if not passed or len(frequencies) != 1:
    raise SystemExit("ERROR: Invalid benchmark status/frequency")

frequency = frequencies.pop()

if frequency != 62500000:
    raise SystemExit("ERROR: Unexpected timer frequency")

for size in EXPECTED_SIZES:
    if sorted(samples[size]) != list(range(SAMPLES_PER_SIZE)):
        raise SystemExit(f"ERROR: Missing samples for {size}")

if sorted(baseline) != list(range(BASELINE_COUNT)):
    raise SystemExit("ERROR: Missing baseline samples")

def to_us(ticks):
    return ticks * 1000000 / frequency

def percentile(values, fraction):
    ordered = sorted(values)
    index = ceil(fraction * len(ordered)) - 1
    return ordered[index]

def show_stats(label, values):
    print(f"\n{label}: {len(values)} samples")
    print(f"  Minimum : {to_us(min(values)):.3f} us")
    print(f"  Mean    : {to_us(mean(values)):.3f} us")
    print(f"  Median  : {to_us(median(values)):.3f} us")
    print(f"  P99     : {to_us(percentile(values, 0.99)):.3f} us")
    print(f"  Maximum : {to_us(max(values)):.3f} us")

    for threshold in (50, 100, 200):
        count = sum(
            to_us(ticks) > threshold for ticks in values
        )
        print(f"  >{threshold} us : {count}")

print("EXPERIMENT 04 - RUN 01 ANALYSIS")
print("Log:", log_file)
print("Frequency:", frequency, "Hz")
print("P99 method: nearest rank")
print("Baseline subtraction: none")

show_stats(
    "Timestamp baseline",
    list(baseline.values())
)

for size in EXPECTED_SIZES:
    show_stats(
        f"Payload {size} bytes",
        list(samples[size].values())
    )

print("\nANALYSIS: PASS - all expected samples verified")
