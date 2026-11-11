#!/usr/bin/env bash
# Rebuild the original UART boot demo using Clang, without changing its files.
set -euo pipefail
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: bash run_boot.sh /path/to/microkit-sdk-2.4.0 [new-output-directory]' >&2
    exit 2
fi
repo=$(cd -- "$(dirname -- "$0")/../.." && pwd)
source_dir="$repo/tests/sel4/00-microkit-hello-world"
sdk=$(cd -- "$1" && pwd)
board_dir="$sdk/board/qemu_virt_aarch64/debug"
if [ "$(cat "$sdk/VERSION")" != '2.4.0' ]; then
    echo 'FAIL: this boot check requires Microkit SDK 2.4.0' >&2
    exit 1
fi
for tool in clang ld.lld qemu-system-aarch64 timeout sha256sum grep; do
    if ! command -v "$tool" >/dev/null; then
        echo "FAIL: missing tool: $tool" >&2
        exit 1
    fi
done
test -x "$sdk/bin/microkit"
test -f "$board_dir/lib/libmicrokit.a"
test -f "$board_dir/elf/sel4.elf"
test -f "$board_dir/include/microkit.h"
if [ "$#" -eq 2 ]; then
    mkdir -- "$2"
    output=$(cd -- "$2" && pwd)
else
    output=$(mktemp -d /tmp/ethercat-05c-boot.XXXXXX)
fi
export MICROKIT_SDK="$sdk"
{
    printf 'Output: %s\n' "$output"
    printf 'SDK: %s\n' "$sdk"
    printf 'SDK version: %s\n' "$(cat "$sdk/VERSION")"
    printf 'Project HEAD: %s\n' "$(git -C "$repo" rev-parse HEAD)"
    uname -sm
    clang --version | head -n 1
    ld.lld --version
    qemu-system-aarch64 --version | head -n 1
    sha256sum "$0" "$source_dir/serial_server.c" "$source_dir/system.system"
} | tee "$output/environment.txt"

clang --target=aarch64-none-elf -std=gnu11 -mcpu=cortex-a53 -mstrict-align \
    -nostdlib -ffreestanding -fno-stack-protector -fno-pic -g \
    -Wall -Wextra -Werror -Wno-unused-parameter \
    -I"$board_dir/include" -c "$source_dir/serial_server.c" \
    -o "$output/serial_server.o" 2>&1 | tee "$output/build.txt"
ld.lld -L"$board_dir/lib" "$output/serial_server.o" \
    -lmicrokit -Tmicrokit.ld -o "$output/serial_server.elf" \
    2>&1 | tee -a "$output/build.txt"
"$sdk/bin/microkit" "$source_dir/system.system" \
    --search-path "$output" --board qemu_virt_aarch64 --config debug \
    -o "$output/loader.img" -r "$output/report.txt" \
    2>&1 | tee -a "$output/build.txt"

echo 'Booting in QEMU TCG for up to 15 seconds; it will stop automatically.'
set +e
timeout --signal=TERM --kill-after=3s 15s qemu-system-aarch64 \
    -machine virt,virtualization=on -cpu cortex-a53 -accel tcg \
    -m size=2G -nographic -serial mon:stdio -nic none \
    -device "loader,file=$output/loader.img,addr=0x70000000,cpu-num=0" \
    > "$output/qemu-boot.log" 2>&1
boot_status=$?
set -e
cat "$output/qemu-boot.log"
printf 'QEMU timeout command exit status: %s\n' "$boot_status" | tee "$output/result.txt"
if [ "$boot_status" -ne 124 ]; then
    echo 'FAIL: expected bounded timeout of the continuously running UART demo' >&2
    exit 1
fi
for marker in 'Booting all finished, dropped to user space' \
    'Microkit Monitor started!' 'SERIAL SERVER: starting'; do
    if ! grep -Fq "$marker" "$output/qemu-boot.log"; then
        echo "FAIL: missing boot marker: $marker" >&2
        exit 1
    fi
done
if grep -Eqi 'MON\|ERROR|Kernel panic|assertion.*failed' "$output/qemu-boot.log"; then
    echo 'FAIL: boot log contains a monitor error, kernel panic or assertion failure' >&2
    exit 1
fi
echo 'PASS: seL4 boot, Microkit monitor and original UART PD startup observed' | tee -a "$output/result.txt"
echo 'NOT TESTED: network driver, Ethernet TX/RX, SOEM, slaves or hardware timing' | tee -a "$output/result.txt"
