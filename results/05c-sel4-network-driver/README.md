# Experiment 5C: environment evidence

Status: student WSL host tools and SDK checks passed. A new Clang build and
boot of the original UART demo passed in the cloud; student WSL boot is next.
No network driver, raw-frame exchange or SOEM execution on seL4 has been tested
for 5C. The boot check does not complete the network-driver milestone.

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
It does not demonstrate an image build or boot on the student's WSL machine.
The earlier attempt to execute the example output line ending in `: OK` was
a shell input mistake, not a checksum failure; the subsequent real checksum
command provides the verification evidence.

## Cloud Clang build and UART boot

The new [boot runner](../../tests/05c-sel4-network-driver/README.md) was executed
against the original teammate UART source and system description, unchanged.
The first build failed because strict C11 rejects the SDK's GNU `asm`
declarations. Changing the runner's language mode to GNU C11 resolved it;
no source assertions were disabled and the UART/SDK sources were not edited.

Final run: `/workspace/setup-ethercat/05c-boot-v2`. Host: Debian 13 x86-64,
Clang/LLD 19.1.7, QEMU 10.0.13 and Microkit SDK 2.4.0. The student's WSL
Clang/LLD 18.1.3 and QEMU 8.2.2 still need the same boot reproduction.

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
run; its recorded SHA-256 identifies the final script. The logged project HEAD
is the preceding commit. Only documentation/evidence was added afterward.
Boot-log CRLF line endings were normalized to LF; all log lines are retained.
Original experiments and historical logs were not overwritten.

The GIC target-ID inference warning is retained; the same warning appears in
the original teammate boot. Startup markers plus the bounded timeout establish
this functional boot only. A timeout by itself is not success, and the 15-second
test duration is not a timing or reliability measurement. No network device was
attached (`-nic none`).
