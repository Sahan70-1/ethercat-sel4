#!/usr/bin/env python3

"""Run Experiment 05 in a fresh QEMU boot and preserve raw output."""

import argparse
from pathlib import Path
import shlex
import subprocess
import time

parser = argparse.ArgumentParser(
    description="Experiment 05 periodic timer benchmark launcher"
)
parser.add_argument(
    "--mode", required=True, choices=("default", "icount")
)
parser.add_argument(
    "--run", required=True, type=int, choices=(1, 2, 3)
)
parser.add_argument(
    "--dry-run", action="store_true",
    help="Show QEMU command without starting it"
)

args = parser.parse_args()

repo = Path(__file__).resolve().parents[3]
image = repo / "tests/sel4/05-timer-jitter/benchmark/build/loader.img"
results = repo / "results/sel4/05-timer-jitter"

log = results / f"full-{args.mode}-run{args.run:02d}-raw.log"

if not image.is_file():
    raise SystemExit("ERROR: Benchmark loader image missing.")

command = ["qemu-system-aarch64"]

if args.mode == "icount":
    command += [
        "-accel", "tcg,thread=single",
        "-icount", "shift=3,sleep=off",
    ]

command += [
    "-machine", "virt,virtualization=on",
    "-cpu", "cortex-a53",
    "-serial", "mon:stdio",
    "-device", f"loader,file={image},addr=0x70000000,cpu-num=0",
    "-m", "size=2G",
    "-nographic",
]

print("===== EXPERIMENT 05 BENCHMARK =====", flush=True)
print("Mode:", args.mode, flush=True)
print("Run:", args.run, flush=True)
print("QEMU command:", shlex.join(command), flush=True)
print("Output:", log, flush=True)

if args.dry_run:
    print("DRY RUN: No QEMU process started.")
    raise SystemExit(0)

results.mkdir(parents=True, exist_ok=True)

if any(
    p.name.replace("\\", "") == log.name
    for p in results.iterdir()
):
    raise SystemExit("ERROR: Raw log already exists. Not overwriting.")

marker = b"BENCH05_CSV_END"
found = False
offset = 0
recent = b""
start = time.monotonic()

with log.open("xb") as output:
    process = subprocess.Popen(
        command,
        stdout=output,
        stderr=subprocess.STDOUT,
    )

    try:
        while time.monotonic() - start < 240:
            time.sleep(1)
            output.flush()

            with log.open("rb") as reader:
                reader.seek(offset)
                chunk = reader.read()
                offset = reader.tell()

            if marker in recent + chunk:
                found = True
                break

            recent = (recent + chunk)[-256:]

            if process.poll() is not None:
                break

    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()

text = log.read_text(errors="replace")

complete = (
    found
    and "BENCH05: COMPLETE" in text
    and "BENCH05_CSV_BEGIN" in text
    and "BENCH05_CSV_END" in text
)

print("===== CAPTURE STATUS =====")
print("Benchmark complete:", "BENCH05: COMPLETE" in text)
print("CSV end detected:", found)
print("QEMU stopped:", process.poll() is not None)
print("Log size:", log.stat().st_size, "bytes")
print("CAPTURE:", "PASS" if complete else "INCOMPLETE")

if not complete:
    raise SystemExit(1)
