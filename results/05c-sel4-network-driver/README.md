# Experiment 5C: environment evidence

Status: student WSL tool inventory only. No driver build, QEMU boot, raw-frame
exchange or SOEM execution on seL4 has been performed for 5C.

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
archive/directory is absent. The next step is installing the required host
tools, checking their versions and looking for an existing SDK directory.
Installation, tool-version checks and SDK discovery are still pending.

See the [next student commands](../../docs/experiment-05c-sel4-network-driver.md#next-student-step-install-host-tools-and-locate-the-sdk).
Do not interpret this inventory as a driver or EtherCAT test result.
