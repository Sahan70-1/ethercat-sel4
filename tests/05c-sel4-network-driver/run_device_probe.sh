#!/usr/bin/env bash
# Build a read-only device probe and check its actual seL4/QEMU output.
set -euo pipefail
if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo 'Usage: bash run_device_probe.sh /path/to/microkit-sdk-2.4.0 [new-output-directory]' >&2
    exit 2
fi
repo=$(cd -- "$(dirname -- "$0")/../.." && pwd)
source_dir="$repo/tests/05c-sel4-network-driver"
sdk=$(cd -- "$1" && pwd)
board_dir="$sdk/board/qemu_virt_aarch64/debug"
mode=${PROBE_MODE:-modern}
case "$mode" in
    modern|absent|legacy) ;;
    *) echo 'FAIL: PROBE_MODE must be modern, absent or legacy' >&2; exit 2 ;;
esac
if [ "$(cat "$sdk/VERSION")" != '2.4.0' ]; then
    echo 'FAIL: this probe requires Microkit SDK 2.4.0' >&2
    exit 1
fi
for tool in clang ld.lld qemu-system-aarch64 timeout sha256sum grep; do
    command -v "$tool" >/dev/null || { echo "FAIL: missing tool: $tool" >&2; exit 1; }
done
test -x "$sdk/bin/microkit"
test -f "$board_dir/lib/libmicrokit.a"
test -f "$board_dir/include/microkit.h"
if [ "$#" -eq 2 ]; then
    mkdir -- "$2"
    output=$(cd -- "$2" && pwd)
else
    output=$(mktemp -d /tmp/ethercat-05c-probe.XXXXXX)
fi
export MICROKIT_SDK="$sdk"
{
    printf 'Output: %s\nSDK: %s\nSDK version: %s\n' "$output" "$sdk" "$(cat "$sdk/VERSION")"
    printf 'Project HEAD: %s\nPROBE_MODE: %s\n' "$(git -C "$repo" rev-parse HEAD)" "$mode"
    uname -sm
    clang --version | head -n 1
    ld.lld --version
    qemu-system-aarch64 --version | head -n 1
    sha256sum "$0" "$source_dir/device_probe.c" "$source_dir/device_probe.system"
} | tee "$output/environment.txt"

clang --target=aarch64-none-elf -std=gnu11 -mcpu=cortex-a53 -mstrict-align \
    -nostdlib -ffreestanding -fno-stack-protector -fno-pic -g \
    -Wall -Wextra -Werror -I"$board_dir/include" \
    -c "$source_dir/device_probe.c" -o "$output/device_probe.o" \
    2>&1 | tee "$output/build.txt"
ld.lld -L"$board_dir/lib" "$output/device_probe.o" -lmicrokit -Tmicrokit.ld \
    -o "$output/device_probe.elf" 2>&1 | tee -a "$output/build.txt"
"$sdk/bin/microkit" "$source_dir/device_probe.system" \
    --search-path "$output" --board qemu_virt_aarch64 --config debug \
    -o "$output/loader.img" -r "$output/report.txt" \
    2>&1 | tee -a "$output/build.txt"
sha256sum "$output/loader.img" | tee -a "$output/environment.txt"

legacy=false
if [ "$mode" = legacy ]; then legacy=true; fi
device_args=()
if [ "$mode" != absent ]; then
    # Isolated in-process hub: no host network, TAP, socket or slave connection.
    device_args=(-netdev hubport,id=probe,hubid=0
        -device virtio-net-device,bus=virtio-mmio-bus.0,netdev=probe,mac=02:00:00:00:05:0c)
fi
qemu_command=(qemu-system-aarch64
    -machine virt-8.2,virtualization=on -cpu cortex-a53 -accel tcg -m size=2G
    -display none -monitor none -serial "file:$output/qemu-boot.log" -nic none
    -global "virtio-mmio.force-legacy=$legacy" "${device_args[@]}"
    -device "loader,file=$output/loader.img,addr=0x70000000,cpu-num=0"
    -d guest_errors,unimp,cpu_reset -D "$output/qemu-debug.log")
printf '%q ' timeout --foreground --signal=TERM --kill-after=3s 30s \
    "${qemu_command[@]}" > "$output/qemu-command.txt"
printf '\n' >> "$output/qemu-command.txt"

echo "Booting device probe ($mode) for up to 30 seconds; output follows automatically."
set +e
timeout --foreground --signal=TERM --kill-after=3s 30s "${qemu_command[@]}" \
    </dev/null > "$output/qemu-stderr.log" 2>&1
boot_status=$?
set -e
if [ -f "$output/qemu-boot.log" ]; then cat "$output/qemu-boot.log"; fi
cat "$output/qemu-stderr.log"
printf 'QEMU timeout command exit status: %s\n' "$boot_status" | tee "$output/result.txt"
fail() { echo "FAIL: $*" | tee -a "$output/result.txt" >&2; exit 1; }
[ "$boot_status" -eq 124 ] || fail 'expected bounded timeout of the idle probe PD'
[ -f "$output/qemu-boot.log" ] || fail 'QEMU did not create the UART log'
if grep -Eqi 'FAIL:|MON\|ERROR|Kernel panic|assertion.*failed' \
    "$output/qemu-boot.log" "$output/qemu-stderr.log"; then
    fail 'device probe or guest reported an error; inspect the logs above'
fi
for marker in 'Booting all finished, dropped to user space' \
    'Microkit Monitor started!' 'DEVICE PROBE: starting (read-only MMIO)' \
    'PASS: modern VirtIO-net MMIO device identified'; do
    grep -Fq "$marker" "$output/qemu-boot.log" || fail "missing marker: $marker"
done
echo 'PASS: seL4 PD identified the modern VirtIO network device through read-only MMIO' | tee -a "$output/result.txt"
echo 'NOT TESTED: device initialization, queues, IRQs, Ethernet TX/RX or SOEM' | tee -a "$output/result.txt"
