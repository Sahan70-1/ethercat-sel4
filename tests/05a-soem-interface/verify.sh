#!/usr/bin/env bash
# Host-only validation. Does not create interfaces or send packets.
set -euo pipefail
repo=$(cd -- "$(dirname -- "$0")/../.." && pwd)
soem=${1:?Usage: bash verify.sh /path/to/SOEM [new-output-directory]}
soem=$(cd -- "$soem" && pwd)
expected=49fc34923b973c17700592319159af22157350c9
[ "$(git -C "$soem" rev-parse HEAD)" = "$expected" ] || { echo 'Wrong SOEM revision'; exit 1; }
[ -z "$(git -C "$soem" status --porcelain --untracked-files=no)" ] || { echo 'SOEM tracked files modified'; exit 1; }
output=${2:-$(mktemp -d)}
mkdir -p "$output"
output=$(cd -- "$output" && pwd)
[ ! -e "$output/soem-build" ] || { echo 'Use a fresh output directory'; exit 1; }
cc --version | head -1
cmake --version | head -1
for demo in frame_demo ethercat_wkc_demo; do
    cc -std=c11 -Wall -Wextra -Werror "$repo/tests/01-frame-handling/$demo.c" -o "$output/$demo"
    "$output/$demo"
done
cmake -S "$soem" -B "$output/soem-build" -DSOEM_BUILD_SAMPLES=ON
cmake --build "$output/soem-build" --target soem simple_ng -j 4
# With no interface argument the sample lists adapters and intentionally exits 1.
set +e
"$output/soem-build/samples/simple_ng/simple_ng" > "$output/simple-ng-usage.txt" 2>&1
status=$?
set -e
cat "$output/simple-ng-usage.txt"
[ "$status" -eq 1 ]
grep -q 'Usage: simple_ng IFNAME1' "$output/simple-ng-usage.txt"
grep -q 'Available adapters:' "$output/simple-ng-usage.txt"
echo 'PASS: host demos, pinned SOEM build, sample usage/adapter enumeration'
echo 'NOT TESTED: raw Ethernet exchange, slaves, seL4/sDDF integration, timing'
