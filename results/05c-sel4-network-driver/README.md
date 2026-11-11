# Experiment 5C: environment evidence

Status: student WSL host-tool versions recorded; official SDK archive, files
and command-line help checked separately in the cloud. SDK installation on
student WSL is pending. No driver build, QEMU boot, raw-frame exchange or SOEM
execution on seL4 has been performed for 5C.

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

This checks the host executable and expected guest files. It does not build or
boot an image, qualify the driver, or establish Microkit/sDDF compatibility.
