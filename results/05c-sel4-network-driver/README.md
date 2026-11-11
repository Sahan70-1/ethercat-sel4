# Experiment 5C: boot and device-access evidence

Status: student WSL host tools and SDK checks passed. The original UART demo
boots in the cloud and in the student's later WSL file-console diagnostic.
The first WSL boot check failed with an empty log; that failure is retained.
The updated runner passed fresh cloud and student WSL builds/boots with QEMU
8.2.2. The cause of the initial empty log remains unconfirmed. A new
[read-only network-device probe](device-probe.md) is the next student step;
its cloud checks are recorded separately. Device initialization, network
TX/RX and SOEM execution on seL4 remain unrun. Boot and device identification
do not complete the network-driver milestone.

## Initial student WSL inventory

Provenance: output supplied by the student in chat on 2026-11-11, from the
[initial inspection commands](../../docs/experiment-05c-sel4-network-driver.md#first-student-step-inspect-wsl-without-installing-anything).
The commands were not executed remotely by Codex on the student's machine.
The shell was in `/mnt/c/Users/SAHAN`; these commands did not depend on the
current directory.

Supplied output, with shell prompts omitted:

```text
x86_64
MISSING: qemu-system-aarch64
MISSING: clang
MISSING: ld.lld
MISSING: dtc
/usr/bin/make
MICROKIT_SDK=not set
```

The host is x86-64. The intended initial guest remains AArch64, matching the
teammate's `qemu_virt_aarch64` experiments; QEMU can emulate that guest on this
host. Make is on PATH. The four missing commands were not found on PATH;
this does not search every directory for downloaded tools. No versions were
reported by this inventory.

`MICROKIT_SDK` is unset or empty. This alone does not establish that the SDK
archive/directory is absent. At this initial stage, installation, tool-version
checks and SDK discovery were still pending; the student's later output follows.

See the [next student commands](../../docs/experiment-05c-sel4-network-driver.md#next-student-step-install-host-tools-and-locate-the-sdk).
Do not interpret this inventory as a driver or EtherCAT test result.

## Student tool versions after the installation step

Provenance: later terminal output supplied by the student in chat on 2026-11-11.
The version commands were those in the
[host-tool instructions](../../docs/experiment-05c-sel4-network-driver.md#next-student-step-install-host-tools-and-locate-the-sdk).
Their supplied output was:

```text
QEMU emulator version 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.18)
Ubuntu clang version 18.1.3 (1ubuntu1)
Ubuntu LLD 18.1.3 (compatible with GNU linkers)
llvm-objcopy, compatible with GNU objcopy
Version: DTC 1.7.0
Python 3.12.3
```

These commands are now available. The displayed llvm-objcopy line identifies
the tool but does not report a numeric version. The apt installation log and
Python venv/pip checks were not part of the supplied output.

The student also ran:

```sh
find ~ -maxdepth 5 -type d -name 'microkit-sdk*' 2>/dev/null
```

It produced no output. No matching SDK directory was found within that bounded
home-directory search. SDK download/installation on WSL is the next step.

## Cloud check of the official Microkit SDK

Executed separately by Codex, not on the student's WSL computer. Downloaded:

```text
https://github.com/seL4/microkit/releases/download/2.4.0/microkit-sdk-2.4.0-linux-x86-64.tar.gz
Archive size: 72615897 bytes
SHA-256: 10bc305713965a01e52b6c7567cea4952b0cc4dde4befa2e6e72b8dce2715f6b
```

The checksum is computed from the official release download inspected here;
it is a reproducibility comparison, not a separately authenticated publisher
checksum. Extraction into a fresh workspace directory succeeded. Checked:

```text
Archive top-level directory: microkit-sdk-2.4.0
SDK VERSION: 2.4.0
PRESENT: bin/microkit
PRESENT: board/qemu_virt_aarch64/debug/elf/sel4.elf
PRESENT: board/qemu_virt_aarch64/debug/lib/libmicrokit.a
PRESENT: board/qemu_virt_aarch64/debug/include/microkit.h
```

Running the extracted `bin/microkit --help` exited 0 and listed
`qemu_virt_aarch64` among supported boards. The verified version command is
reading the SDK's `VERSION` file; no `microkit --version` option is assumed.
The archive and extracted SDK remain outside the project repository under
`/workspace/setup-ethercat/microkit-2.4.0-check/`.

These SDK-file/help checks did not boot an image or establish Microkit/sDDF
compatibility. A separate later UART boot check is recorded below.

## Student SDK verification

The student supplied further terminal output showing:

- The downloaded archive SHA-256 matches
  `10bc305713965a01e52b6c7567cea4952b0cc4dde4befa2e6e72b8dce2715f6b`.
- `/home/sahan/tools/microkit-sdk-2.4.0` exists.
- With `MICROKIT_SDK="$HOME/tools/microkit-sdk-2.4.0"`, reading `VERSION`
  prints `2.4.0` and running `bin/microkit --help` displays usage and lists
  `qemu_virt_aarch64`.

This establishes the student's download checksum and SDK host-tool check.
The later image build, first failed boot check and successful diagnostic are
recorded below.
The earlier attempt to execute the example output line ending in `: OK` was
a shell input mistake, not a checksum failure; the subsequent real checksum
command provides the verification evidence.

## Cloud Clang build and UART boot

The new [boot runner](../../tests/05c-sel4-network-driver/README.md) was executed
against the original teammate UART source and system description, unchanged.
The first build failed because strict C11 rejects the SDK's GNU `asm`
declarations. Changing the runner's language mode to GNU C11 resolved it;
no source assertions were disabled and the UART/SDK sources were not edited.

Original runner run: `/workspace/setup-ethercat/05c-boot-v2`. Host: Debian 13 x86-64,
Clang/LLD 19.1.7, QEMU 10.0.13 and Microkit SDK 2.4.0. The student's later WSL
attempt with Clang/LLD 18.1.3 and QEMU 8.2.2 built the image but failed the boot
check, as recorded below.

The cloud had no sudo or compiler/QEMU on PATH. Official Debian packages were
downloaded and extracted into the user-owned
`/workspace/setup-ethercat/host-tools/root/`; system package configuration was
not changed. Package versions are in [cloud-host-tools.txt](cloud-host-tools.txt).
The run used:

```sh
PATH=/workspace/setup-ethercat/host-tools/root/usr/lib/llvm-19/bin:/workspace/setup-ethercat/host-tools/root/usr/bin:$PATH \
LD_LIBRARY_PATH=/workspace/setup-ethercat/host-tools/root/usr/lib/x86_64-linux-gnu \
bash tests/05c-sel4-network-driver/run_boot.sh \
  /workspace/setup-ethercat/microkit-2.4.0-check/sdk/microkit-sdk-2.4.0 \
  /workspace/setup-ethercat/05c-boot-v2
```

| Check | Observed outcome |
|---|---|
| Clang compile / LLD link | PASS; no compiler diagnostics |
| SDK image builder | PASS; 36 root objects; 2.62 MiB loader image |
| seL4 kernel boot | PASS; reached user space |
| Microkit monitor | PASS; startup marker observed |
| Original UART PD | PASS; `SERIAL SERVER: starting` observed |
| Bounded run | QEMU timeout status 124 after 15 seconds; overall runner exited 0 |
| Network driver, Ethernet, SOEM, slaves, hardware timing | UNRUN |

Evidence: [environment/source hashes](cloud-boot/environment.txt),
[build output](cloud-boot/build.txt), [boot log](cloud-boot/qemu-boot.log),
[runner result](cloud-boot/result.txt). The runner was uncommitted during this
run; its recorded SHA-256 identifies the original script committed in
`af4aa7c`. The logged project HEAD is the preceding commit. The later console
changes and their fresh cloud test are documented separately below.
Boot-log CRLF line endings were normalized to LF; all log lines are retained.
Original experiments and historical logs were not overwritten.

The GIC target-ID inference warning is retained; the same warning appears in
the original teammate boot. Startup markers plus the bounded timeout establish
this functional boot only. A timeout by itself is not success, and the 15-second
test duration is not a timing or reliability measurement. No network device was
attached (`-nic none`).

## First student WSL build: boot check failed

The student supplied [this terminal transcript](wsl-boot-failure.txt) from
commit `af4aa7c909decb4862b3ade2e3d708601720267d`. The runner and original
source/system SHA-256 values match the cloud run. Output directory:
`/tmp/ethercat-05c-boot.ujkjn9`.

Compilation/link and SDK image generation completed: 36 root objects and a
2.62 MiB loader image. The timeout returned 124, but no QEMU text appeared
before the runner reported its first missing boot marker and exited 1.
At this first submission, the raw `qemu-boot.log`, image hash and runtime debug
logs were not supplied.
This is a failed WSL boot check. It does not demonstrate kernel, monitor or
UART startup; timeout by itself does not establish execution of the guest.

The subsequent
[diagnostic](../../tests/05c-sel4-network-driver/README.md#diagnose-the-first-wsl-silent-boot)
reused its existing image and captured UART, QEMU stderr and limited debug
output separately, with no interactive console and a 30-second foreground
timeout. That diagnostic passed, as recorded below. The original failed run
and its artifacts were not replaced.

## Cloud comparison with Ubuntu QEMU 8.2.2

To investigate the emulator-version difference, the official Ubuntu package
`qemu-system-arm_8.2.2+ds-0ubuntu1.18_amd64.deb` was downloaded from
`https://archive.ubuntu.com/ubuntu/pool/main/q/qemu/` and extracted into the
workspace, not installed system-wide. The reported binary version matches
the student's version string. Ubuntu libjpeg-turbo8 2.1.5-2ubuntu2 and
libfuse3-3 3.14.0-5build1 were also extracted; other libraries came from the
existing cloud environment/workspace. This is not a full Ubuntu/WSL replica.

The existing cloud-built image was exercised in a PTY:

| Comparison | Observed outcome |
|---|---|
| Original console, QEMU 10.0.13, 5-second timeout, job control disabled/enabled | All three startup markers present |
| Original console, Ubuntu QEMU 8.2.2, 5-second timeout, job control enabled | All three startup markers present |
| File console, Ubuntu QEMU 8.2.2, 30-second foreground timeout, stdin `/dev/null` | All three startup markers present; timeout status 124 |

The file-console run retained 2499 UART bytes, 71 stderr bytes and 1726 debug
bytes before line-ending normalization. Evidence is under
[cloud-diagnostics](cloud-diagnostics/environment.txt). These observations
show the cloud image boots under the tested QEMU versions/console modes;
they do not explain the WSL failure. The cloud image was built with Clang
19.1.7. At the time of this comparison the student's image hash was unknown;
the later WSL submission below reports the same SHA-256 despite using Clang
18.1.3. The actual WSL image file has not been uploaded.

## Student WSL diagnostic: boot passed

Provenance: the student supplied [this diagnostic transcript](wsl-boot-diagnostic.txt)
in chat on 2026-11-11. Codex did not execute commands on the student's machine.
The pasted command echo contains a garbled line before the diagnostic output;
it is retained as supplied. Only CRLF line endings were normalized to LF.
Use the [documented command](../../tests/05c-sel4-network-driver/README.md#diagnose-the-first-wsl-silent-boot)
for reproduction, not the transcript's command echo.

The diagnostic reused `/tmp/ethercat-05c-boot.ujkjn9/loader.img` from the failed
attempt without a rebuild. Its supplied SHA-256 is
`408f1aed9f5100d9a6c217a29f2b53224408b9e90e48300362572f1bee36c8ea`,
matching the image used in the cloud comparisons. The original `qemu-boot.log`
was confirmed to be zero bytes. New outputs were saved under
`/tmp/ethercat-05c-diagnostic.4fssVA`.

| Check | Supplied result |
|---|---|
| UART log size | 2499 bytes |
| QEMU stderr log size | 71 bytes; termination by signal 15 from `timeout` |
| QEMU debug log size | 1726 bytes; CPU-reset diagnostic tail supplied |
| seL4 kernel boot | `Booting all finished, dropped to user space` |
| Microkit monitor | `MON\|INFO: Microkit Monitor started!` |
| Original UART PD | `SERIAL SERVER: starting` |
| Bounded diagnostic command | Exit 124 after the configured 30-second timeout |

Outcome: **PASS for this UART boot diagnostic**. The three startup markers
establish execution; timeout status 124 alone would not. The GIC target-ID
inference warning is retained, as in the prior cloud and teammate runs.
Network-driver TX/RX, SOEM execution, actual slaves and hardware timing remain
untested in 5C. This is not a completion claim for the whole experiment.

The successful attempt changes console routing, monitor/input handling,
foreground timeout handling, timeout duration and debug logging together.
It establishes a working launch configuration, but does not isolate which
change resolved the earlier silent run or prove its underlying cause.

## Updated runner: fresh cloud build and boot passed

`run_boot.sh` now uses the diagnostic's file-console configuration, no terminal
input or interactive monitor, a 30-second foreground timeout, and separate
UART/stderr/debug logs. It records the generated image hash and checks the
same three startup markers. Monitor errors, kernel panics and assertion-failure
messages in UART or stderr fail the check. The original UART C source/system
and all prior evidence remain unchanged.

A fresh run in `/workspace/setup-ethercat/05c-boot-file-console` compiled,
linked and built an image successfully, then observed all three markers.
QEMU timeout status was 124 and the **overall runner exited 0**. Tested on
Debian 13 x86-64 with Clang/LLD 19.1.7, Microkit SDK 2.4.0 and the extracted
Ubuntu QEMU 8.2.2 binary described above. This is a cloud test, not a fresh
WSL execution. The generated image SHA-256 again matches the student's
diagnostic image.

Reproduction command for that cloud tool layout:

```sh
PATH=/workspace/setup-ethercat/host-tools/root/usr/lib/llvm-19/bin:/workspace/setup-ethercat/host-tools/ubuntu8/usr/bin:/workspace/setup-ethercat/host-tools/root/usr/bin:$PATH \
LD_LIBRARY_PATH=/workspace/setup-ethercat/host-tools/ubuntu8/lib/x86_64-linux-gnu:/workspace/setup-ethercat/host-tools/ubuntu8/usr/lib/x86_64-linux-gnu:/workspace/setup-ethercat/host-tools/root/usr/lib/x86_64-linux-gnu \
bash tests/05c-sel4-network-driver/run_boot.sh \
  /workspace/setup-ethercat/microkit-2.4.0-check/sdk/microkit-sdk-2.4.0 \
  /workspace/setup-ethercat/05c-boot-file-console
```

Evidence: [environment and hashes](cloud-boot-file-console/environment.txt),
[build](cloud-boot-file-console/build.txt), [UART](cloud-boot-file-console/qemu-boot.log),
[stderr](cloud-boot-file-console/qemu-stderr.log), [debug](cloud-boot-file-console/qemu-debug.log),
[result](cloud-boot-file-console/result.txt). CRLF line endings are normalized;
all log text is retained. The recorded HEAD is `d8e0d9d` because the runner
change was uncommitted during this test; its SHA-256 identifies the tested
script. The student subsequently repeated this updated runner using the
[WSL instructions](../../tests/05c-sel4-network-driver/README.md#run-on-student-wsl),
with results below.

## Updated runner: student WSL build and boot passed

Provenance: the student supplied [this complete terminal transcript](wsl-boot-pass.txt)
after updating the 5C branch to `bc2e3623d07b794370d10c0d4ec4338ab013a2fc`.
The commands ran on their WSL machine, not remotely in the cloud. Only CRLF
line endings were normalized when storing the supplied text.

Output directory: `/tmp/ethercat-05c-boot.nHG7pI`. SDK 2.4.0, Ubuntu Clang/LLD
18.1.3 and QEMU 8.2.2. The supplied runner and original source/system hashes
match the tested branch. The fresh image hash again matches
`408f1aed9f5100d9a6c217a29f2b53224408b9e90e48300362572f1bee36c8ea`.

| Check | Supplied outcome |
|---|---|
| Compile, link and Microkit image generation | PASS; 36 root objects, 2.62 MiB loader image |
| seL4 user-space entry | Startup marker present |
| Microkit monitor | Startup marker present |
| Original UART PD | `SERIAL SERVER: starting` present |
| Bounded QEMU run | Timeout status 124, as expected |
| Automated runner | PASS; final runner exit status **0** |

This completes the student's fresh UART build/boot precheck. The first failed
attempt and successful diagnostic remain preserved. No network device was
attached in this boot test. The next step is the
[VirtIO device probe](../../tests/05c-sel4-network-driver/device-probe.md).
