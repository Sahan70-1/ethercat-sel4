# Experiment 5C: read-only device-probe results

Status: cloud device-identification check passed; two negative controls
correctly prevented a pass. The student WSL device-probe run is **pending**.
The separate [WSL UART build/boot](README.md#updated-runner-student-wsl-build-and-boot-passed)
has passed. No NIC initialization, queues, interrupts, Ethernet TX/RX or SOEM
execution is established by this probe.

## Tested source and environment

Actual cloud runs executed by Codex, separately from the student's WSL machine.
Host: Debian 13 x86-64; Clang/LLD 19.1.7, Microkit SDK 2.4.0 and the official
Ubuntu QEMU 8.2.2 package extracted into the workspace. See the earlier
[host-tool comparison](README.md#cloud-comparison-with-ubuntu-qemu-822) for
the library/environment distinction from WSL.

The [source and instructions](../../tests/05c-sel4-network-driver/device-probe.md)
pin `virt-8.2`, Cortex-A53, TCG, 2 GiB RAM and `virtio-mmio-bus.0`. The SDK
system description maps physical `0x0a000000` read-only and uncached into
the PD. Modern mode uses `virtio-mmio.force-legacy=false`. The NIC's QEMU
hub has no host network connection.

Final source hashes:

| File | SHA-256 |
|---|---|
| `device_probe.c` | `d00108f3edd09991c5db2cf5d6240963adfa9cc89ee855ef757c3c80a34a3a22` |
| `device_probe.system` | `c5d1a1275515553500674ca1803c7c611a72844e6d1953db6833930e7865e660` |
| `run_device_probe.sh` | `0f2e58ea0a2c6922b96aa25b6d1906813becf3e6c3ac5bb2f276f011d856e457` |

All three fresh builds produced image SHA-256
`27b21590f35f1671fbde21768f723f0475905e345b9fe5d02f18d5ad986da3f7`.
The code was uncommitted during testing, so each environment log records
preceding HEAD `bc2e3623d07b794370d10c0d4ec4338ab013a2fc`; the source hashes
identify the tested implementation. Compile/link used warnings as errors and
completed without compiler diagnostics. Each image had 36 root objects and
a reported loader size of 2.62 MiB.

## Outcomes

| QEMU configuration | Actual guest observation | Runner exit | Interpretation |
|---|---|---|---|
| Modern network card on bus 0 | Magic `0x74726976`, Version 2, Device ID 1, Vendor `0x554d4551`; device-identification PASS | **0** | seL4 PD can read and identify the intended network device |
| No card attached | Device ID 0; guest reports an empty slot and skips it; no device PASS | **1** | Expected negative control; the host check rejects the missing network device |
| Legacy network interface | Version 1, Device ID 1; guest rejects unsupported interface | **1** | Expected negative control; modern-interface requirement is enforced |

All three runs reached seL4 user space and the Microkit monitor/probe startup.
All used the same 30-second foreground timeout and returned internal status
124. The negative cases therefore demonstrate that boot and timeout alone
cannot satisfy the device-identification check.

The VirtIO specification treats Device ID 0 as a valid empty transport slot;
the guest does not report it as a device fault. The host test fails its
requirement to identify the deliberately expected network card. This is a
test-fixture condition, not a claim that an empty slot violates VirtIO.

## Reproduction and evidence

With the documented cloud Clang/QEMU paths and libraries on `PATH` and
`LD_LIBRARY_PATH`, the three runs used `PROBE_MODE=modern`, `absent` and
`legacy`, the same SDK, and separate fresh directories
`/workspace/setup-ethercat/05c-probe-final-{modern,absent,legacy}`.
For ordinary WSL use, follow the [normal probe command](../../tests/05c-sel4-network-driver/device-probe.md#run-in-the-existing-wsl-terminal).

| Case | Environment / build / QEMU arguments | Runtime evidence |
|---|---|---|
| Modern | [Environment](cloud-device-probe/modern/environment.txt), [build](cloud-device-probe/modern/build.txt), [command](cloud-device-probe/modern/qemu-command.txt) | [UART](cloud-device-probe/modern/qemu-boot.log), [stderr](cloud-device-probe/modern/qemu-stderr.log), [debug](cloud-device-probe/modern/qemu-debug.log), [result](cloud-device-probe/modern/result.txt), [exit](cloud-device-probe/modern/runner-exit.txt) |
| Absent | [Environment](cloud-device-probe/absent/environment.txt), [build](cloud-device-probe/absent/build.txt), [command](cloud-device-probe/absent/qemu-command.txt) | [UART](cloud-device-probe/absent/qemu-boot.log), [stderr](cloud-device-probe/absent/qemu-stderr.log), [debug](cloud-device-probe/absent/qemu-debug.log), [result](cloud-device-probe/absent/result.txt), [exit](cloud-device-probe/absent/runner-exit.txt) |
| Legacy | [Environment](cloud-device-probe/legacy/environment.txt), [build](cloud-device-probe/legacy/build.txt), [command](cloud-device-probe/legacy/qemu-command.txt) | [UART](cloud-device-probe/legacy/qemu-boot.log), [stderr](cloud-device-probe/legacy/qemu-stderr.log), [debug](cloud-device-probe/legacy/qemu-debug.log), [result](cloud-device-probe/legacy/result.txt), [exit](cloud-device-probe/legacy/runner-exit.txt) |

Runtime/build/environment logs retain all text with CRLF line endings normalized.
Saved QEMU command metadata also omits the trailing argument-separator space;
the recorded arguments are unchanged. The GIC target warning remains visible.
The hub-disconnected warning is expected for this
isolated identification check; it would not be evidence of a working external
frame path. No hardware timing, IRQ delivery, DMA, isolation proof or physical
slave communication is claimed.
