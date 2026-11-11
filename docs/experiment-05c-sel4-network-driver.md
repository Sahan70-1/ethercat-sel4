# Experiment 5C: seL4 network driver and SOEM integration

Status on 2026-11-11: scope/literature review and initial student WSL inventory
recorded. Host tools need installation; the SDK location is not yet known.
No 5C driver image has been built or booted, and no seL4 network TX/RX or SOEM
startup is claimed.
Work is on `experiment-05c-sel4-network-driver`, based on the completed Linux
milestone `85ae914`. Nothing has been merged into main.

## Objective and terminology

Run SOEM on seL4 with a network driver that transmits raw EtherCAT frames and
delivers received frames to the master. Later evaluate functional correctness,
isolation costs, latency and jitter. QEMU establishes functional behavior;
physical timing and real slave communication require later hardware tests.

The student's current supervisor context takes precedence over assumptions
in the older handoff. Reusing an upstream NIC driver is a possible implementation
choice, not an already agreed definition of the research contribution.

| Component | Responsibility |
|---|---|
| SOEM master PD | EtherCAT datagrams, indexes, WKC, configuration and process data; SOEM must be compiled for the seL4 environment |
| SOEM platform support | Networking calls, buffer transfer, monotonic time/deadlines, lifecycle, allocation and an explicit concurrency model |
| Network driver PD | Device initialization, raw-frame TX/RX, descriptor/buffer ownership, interrupts and device notification |
| seL4/Microkit configuration | Memory mappings, capabilities, channels, scheduling and access to the device/interrupts |

The earlier word "adapter" referred to the networking part of SOEM platform
support. It does not prescribe a new standalone service, an additional PD,
or application middleware. Platform functions remain necessary with either
an existing NIC driver or a newly written one: Linux sockets/pthreads are not
supplied simply by booting seL4.

The teammate's shared-memory/notification demos provide foundations for this
boundary. Their `driver.c` is not a NIC driver. In the intended microkernel
arrangement, the NIC driver runs in a user-level PD; "on seL4" does not by
itself mean adding a network stack inside the kernel.

## Driver choice: what is known and what remains open

QEMU VirtIO-net is the candidate initial device because the teammate's
environment uses `qemu_virt_aarch64` and no physical NIC has been confirmed.

The previously inspected sDDF revision
`4b47a3534b136e240fbad970aec277adc4d65fe7` contains an existing
[VirtIO-net driver](https://github.com/au-ts/sddf/blob/4b47a3534b136e240fbad970aec277adc4d65fe7/drivers/network/virtio/common/ethernet.c)
and lists Microkit SDK 2.4.0. It is a source-inspected candidate, not a tested
dependency combination for this project. Its comments describe a minimal
simulator-oriented implementation, not a performance-qualified physical driver.

The implementation choice remains open: reuse with attribution, modify/port,
or implement a driver against the VirtIO specification. A new implementation
would include device initialization, feature negotiation, queues, TX/RX buffers,
interrupt handling and recovery. Record this choice and the supervisor's
requirements before describing reused driver code as our own contribution.
The [uploaded paper](paper-review-sygulla-ethercat-control.md) does not resolve
this choice: it uses QNX and Acontis and does not document a new NIC driver.

## Proposed milestones and evidence

| Step | Work | Evidence needed before calling it complete |
|---|---|---|
| 5C.1: environment and driver design | Inspect WSL tools/SDK, choose the QEMU device and driver implementation approach, pin versions and define buffers/channels/scheduling | Tool/version inventory and a reviewable source/configuration plan |
| 5C.2: driver TX/RX | Build and boot the selected driver with a small raw-frame test PD, independently of SOEM | Captured outgoing 0x88A4 bytes; known incoming frame delivered with correct bytes/length; no-response deadline; repeated buffer reuse and queue exhaustion/recovery checks |
| 5C.3: SOEM on seL4 | Build SOEM with the required platform headers/services and connect it to the tested driver | Startup output; synthetic frame/index/payload/WKC checks; timeout/retry tests; demonstrated driver and timer progress while the master waits |
| Later functional/hardware evaluation | Actual slave discovery/configuration, cyclic data, physical timing and controlled Linux comparison | Real device setup and separately recorded functional/timing measurements |

For raw traffic outside QEMU, use a backend capable of carrying arbitrary L2
frames, such as a configured TAP/test peer. IP-only user networking is not a
sufficient EtherCAT validation path. Record capture locations and distinguish
driver acceptance of a TX buffer from actual transmission.

SOEM startup can be tested before a network driver is ready, but that proves
startup only. The proposed driver-first order establishes an independently
testable frame path before combining the components. The original Linux
5B tests remain comparison/regression evidence.

## First student step: inspect WSL without installing anything

In the existing WSL terminal:

```sh
uname -m
for tool in qemu-system-aarch64 clang ld.lld dtc make; do
    command -v "$tool" || printf 'MISSING: %s\n' "$tool"
done
printf 'MICROKIT_SDK=%s\n' "${MICROKIT_SDK:-not set}"
```

Send the output before choosing installation/build commands. This is an
inventory, not a driver test. A missing environment variable alone does not
prove the SDK is absent. The teammate's SDK/tool paths are on another machine
and must not be assumed to exist on the student's WSL computer.

The student supplied this inventory; see the
[environment evidence](../results/05c-sel4-network-driver/README.md).
Keep runtime logs separate from this initial host inventory and preserve the
existing experiments and 5B evidence.

## Next student step: install host tools and locate the SDK

The reported WSL host is x86-64, Make is available, and QEMU AArch64, Clang,
LLD and DTC are not on PATH. The SDK variable is unset. We will use host
executables for x86-64 while compiling/emulating an AArch64 guest.

The inspected sDDF README requires Make, Clang/LLVM, DTC and Python; its
VirtIO driver makefile uses the selected compiler/linker. These tools also
support development of our own driver. Installing them does not decide whether
the project reuses or writes the NIC driver. Ubuntu's `qemu-system-arm` package
provides `qemu-system-aarch64`.

Run the following commands in WSL. If either installation command fails,
stop and retain its error output:

```sh
sudo apt update
sudo apt install -y qemu-system-arm clang llvm lld device-tree-compiler \
  python3-venv python3-pip
```

Python virtual-environment support is for later isolated build dependencies;
do not install Python packages into the system interpreter at this step.
Then collect tool versions:

```sh
qemu-system-aarch64 --version | head -n 1
clang --version | head -n 1
ld.lld --version
llvm-objcopy --version | head -n 1
dtc --version
python3 --version
```

Before downloading another SDK, look for an existing directory in the Linux
home directory:

```sh
find ~ -maxdepth 5 -type d -name 'microkit-sdk*' 2>/dev/null
```

Send the version output and any SDK paths found. No output from this bounded
search means no matching directory was found within its search scope; it does
not prove the SDK is absent everywhere. The next step will inspect an existing
SDK or download the selected release for Linux x86-64. The earlier candidate
release is Microkit 2.4.0. No SDK download, source checkout or driver build is
part of these commands, and their successful execution is not yet recorded.
