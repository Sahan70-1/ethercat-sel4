# Experiment 5C: seL4 network driver preparation

Current runnable check: rebuild and boot the original seL4 UART demo using
Clang/LLD. This verifies the host toolchain, SDK, image builder, QEMU boot and
PD startup before developing the network path. No network driver or SOEM is
included in this image.

## Source reuse

`run_boot.sh` compiles `tests/sel4/00-microkit-hello-world/serial_server.c` and
uses that experiment's `system.system` unchanged. The original source, GCC
Makefile and historical results remain in place. The new runner adds a Clang
build and bounded boot check; it does not duplicate the C program or claim
that the teammate's UART demo is our NIC driver.

## Run on student WSL

Requires the verified Microkit SDK 2.4.0, Git, Clang, LLD, QEMU AArch64 and
standard Linux shell tools. Keep the project checkout on the 5C branch.
If `git status --short` lists changes, inspect them before switching/pulling.

```sh
cd ~/FYP/ethercat-sel4
git status --short
```

From a clean checkout, update the branch:

```sh
git fetch origin
git switch experiment-05c-sel4-network-driver
git pull --ff-only origin experiment-05c-sel4-network-driver
```

Then run:

```sh
export MICROKIT_SDK="$HOME/tools/microkit-sdk-2.4.0"
bash tests/05c-sel4-network-driver/run_boot.sh "$MICROKIT_SDK"
echo "Runner exit status: $?"
```

The script cross-compiles the UART program for AArch64, links with the SDK
library, builds `loader.img` and boots it with QEMU TCG. It stops QEMU
automatically after 15 seconds, so manual QEMU exit is not required.

Expected: the final PASS line confirms the kernel reached user space, the
Microkit monitor started and the original UART PD printed its startup marker.
The overall runner should exit 0. The internal timeout status 124 is expected
because the demo normally runs continuously; timeout alone is never a pass.
The runner also requires all three boot markers and rejects monitor errors,
kernel panics and assertion-failure messages. These log checks verify this
startup exercise; they are not a general correctness or fault-isolation proof.

The first student WSL attempt built the image but failed this boot check:
timeout status 124, no QEMU text in the supplied transcript, missing first
boot marker, runner exit 1. See the [failed run](../../results/05c-sel4-network-driver/wsl-boot-failure.txt).
Use the diagnostic below before proceeding to network-driver work.

Send the printed output directory, complete terminal output and runner exit
status. Stop at an error and retain its output. No success on the student's
machine is claimed until those results are received.

## Artifacts and limitations

Each default run creates a new `/tmp/ethercat-05c-boot.*` directory. An optional
second argument chooses a new, nonexistent output directory. Outputs are
`environment.txt`, `build.txt`, `serial_server.o`, `serial_server.elf`,
`loader.img`, `report.txt`, `qemu-boot.log` and `result.txt`. Logs include tool
versions and hashes of the runner, UART source and system description.
Build outputs stay outside the checkout; no existing results are overwritten.

GNU C11 is required because seL4 headers use GNU `asm` register declarations.
Warnings are errors; the only explicit warning exception permits unused
callback parameters in the unchanged UART source.

QEMU runs with `-accel tcg` and `-nic none`. No NIC is attached and no network
backend, root access, TAP interface or slave is required. The observed GIC
target-ID inference warning also appears in the teammate's original boot;
it is retained in the logs. Its presence alone does not fail this startup check.

[Current evidence](../../results/05c-sel4-network-driver/README.md).
[5C scope and remaining network-driver work](../../docs/experiment-05c-sel4-network-driver.md).

## Diagnose the first WSL silent boot

This reuses `/tmp/ethercat-05c-boot.ujkjn9/loader.img` without rebuilding or
changing the original run. Confirm its old console log size, then capture the
UART directly to a new file, QEMU stderr separately, and limited guest-error/
CPU-reset diagnostics. QEMU has no interactive monitor or terminal input in
this attempt. The timeout increases from 15 to 30 seconds. These changes are
a diagnostic comparison, not a confirmed fix or an isolated root-cause test.

```sh
(
  set +e
  image=/tmp/ethercat-05c-boot.ujkjn9/loader.img
  if [ ! -f "$image" ]; then
    echo "The previous image is missing; send this message before rebuilding."
    exit 1
  fi
  diagnostic_dir=$(mktemp -d /tmp/ethercat-05c-diagnostic.XXXXXX)
  echo "Diagnostic directory: $diagnostic_dir"
  wc -c /tmp/ethercat-05c-boot.ujkjn9/qemu-boot.log
  sha256sum "$image"

  timeout --foreground --signal=TERM --kill-after=3s 30s qemu-system-aarch64 \
    -machine virt,virtualization=on -cpu cortex-a53 -accel tcg -m size=2G \
    -display none -monitor none -serial "file:$diagnostic_dir/uart.log" -nic none \
    -device "loader,file=$image,addr=0x70000000,cpu-num=0" \
    -d guest_errors,unimp,cpu_reset -D "$diagnostic_dir/qemu-debug.log" \
    </dev/null > "$diagnostic_dir/qemu-stderr.log" 2>&1
  diagnostic_status=$?
  echo "Diagnostic QEMU exit status: $diagnostic_status"
  wc -c "$diagnostic_dir"/*.log
  cat "$diagnostic_dir/uart.log" "$diagnostic_dir/qemu-stderr.log"
  tail -n 40 "$diagnostic_dir/qemu-debug.log"
)
```

Send the complete output. Status 124 means only that the timeout elapsed;
inspect the actual startup messages before calling boot successful. Missing
images, startup errors and empty logs are useful diagnostic results too.

Cloud comparison: the original image boots with the official Ubuntu QEMU
8.2.2 package as well as QEMU 10.0.13. This file-console diagnostic captured
all three startup markers with QEMU 8.2.2. It still uses the cloud-built image
and cloud host libraries, so it does not reproduce the entire WSL environment.
The WSL diagnostic result and root cause remain unknown.
