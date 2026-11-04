#!/usr/bin/env bash
# Rootless tests. Socketpairs carry synthetic Ethernet bytes, never NIC traffic.
set -euo pipefail
repo=$(cd -- "$(dirname -- "$0")/../.." && pwd)
soem=${1:?Usage: bash run.sh /path/to/pinned/SOEM [new-output-directory]}
if [ "$#" -ge 2 ]; then
    mkdir -- "$2"
    output=$(cd -- "$2" && pwd)
else
    output=$(mktemp -d /tmp/ethercat-05b.XXXXXX)
fi
echo "Output: $output"
python3 "$repo/tests/05b-linux-transport/prepare_soem.py" "$soem" "$output/soem" | tee "$output/prepare.txt"
cmake -S "$repo/tests/05b-linux-transport" -B "$output/build" \
    -DSOEM_SOURCE="$output/soem" -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_SANITIZERS="${ENABLE_SANITIZERS:-OFF}" | tee "$output/configure.txt"
cmake --build "$output/build" --target test_transport test_soem_transport simple_ng -j 2 \
    2>&1 | tee "$output/build.txt"
ctest --test-dir "$output/build" --output-on-failure -V 2>&1 | tee "$output/tests.txt"
echo 'PASS: 5B local transport and adapted SOEM synthetic-response checks'
echo 'NOT TESTED: AF_PACKET/veth, NIC/slave exchange, seL4/sDDF, hardware timing'
