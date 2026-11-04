# Experiment 5A — student WSL source inspection

Provenance: terminal output supplied by the student in chat. These commands
were executed on the student's computer, not independently rerun there by Codex.

## Checkout verification

- Location: `/home/sahan/FYP/deps/SOEM`.
- HEAD: `49fc34923b973c17700592319159af22157350c9`.
- `git status --short`: no output (clean working tree).
- `osal/linux`: `osal.c`, `osal_defs.h`.
- `oshw/linux`: `nicdrv.c`, `nicdrv.h`, `oshw.c`, `oshw.h`.

## Networking source locations reported

| Function | Line in `oshw/linux/nicdrv.c` |
|---|---:|
| ecx_setupnic | 92 |
| ecx_closenic | 191 |
| ec_setupheader | 206 |
| ecx_getindex | 223 |
| ecx_setbufstat | 262 |
| ecx_outframe | 275 |
| ecx_outframe_red | 304 |
| ecx_recvpkt | 342 |
| ecx_inframe | 378 |
| ecx_waitinframe_red | 485 |
| ecx_waitinframe | 591 |
| ecx_srconfirm | 614 |

Linux call locations: socket 147; setsockopt 153; ioctl 158/163/167;
bind 174; close 194/196; send 290/327; recv 356; ppoll 516.
These match the pinned cloud source audit.

## TX/RX, matching, retries and timing walkthrough

The student supplied source excerpts using these commands in the same checkout:

```sh
sed -n '269,298p' oshw/linux/nicdrv.c
sed -n '335,361p' oshw/linux/nicdrv.c
sed -n '363,472p' oshw/linux/nicdrv.c
sed -n '485,548p' oshw/linux/nicdrv.c
sed -n '591,647p' oshw/linux/nicdrv.c
sed -n '1,85p' osal/linux/osal.c
```

Observed in the supplied source:

- TX selects the stack, marks the index TX and calls `send` with flags 0;
  -1 resets the slot EMPTY. The nonblocking comment is not sufficient to
  establish a nonblocking send.
- RX uses `MSG_DONTWAIT`, records the actual byte count and returns whether
  any bytes arrived.
- `ecx_inframe` handles cached and out-of-order responses, checks EtherType,
  strips the Ethernet header and extracts WKC using the length-derived offset.
- Wait uses requested 50-us ppoll intervals and an OSAL deadline; this is not
  a measured or guaranteed 50-us response time.
- `ecx_srconfirm` retries for results <= -1 while time remains; WKC=0 ends
  that retry loop despite the introductory comment.
- OSAL deadlines use `CLOCK_MONOTONIC`, with stop time equal to current time
  plus timeout. `osal_current_time` separately uses `CLOCK_REALTIME`.
  Relative and absolute sleeps use `clock_nanosleep` with the monotonic clock.
- The excerpt also shows allocation wrappers and the start of the pthread
  implementation. These additional OS services remain part of port planning.

These are source-inspection findings, not runtime test results. The cloud
[design audit](../../docs/experiment-05a-soem-network-audit.md) contains the
proposed sDDF mapping and remaining integration risks. WSL build validation
is recorded below; sample entry-point execution is recorded below; new frame-exchange checks remain unrun. No successful slave exchange or
seL4 integration is claimed.

## WSL build — confirmed from student terminal output

The initial command stopped at `cmake: command not found`; it did not configure
or build anything. The existing environment was located at
`/home/sahan/tools/cmake-venv` and activated with:

```sh
source ~/tools/cmake-venv/bin/activate
cmake --version
```

Reported tools: CMake 4.4.4, GNU C compiler 13.3.0, Python 3.12.3.
The student ran the documented fresh-build commands with samples enabled and
parallelism 2. Build directory: `/tmp/soem-05a.DuYd8f`.
CMake configuration and generation completed. Final output:

```text
[100%] Linking C static library libsoem.a
[100%] Built target soem
[ 86%] Built target soem
[ 93%] Building C object samples/simple_ng/CMakeFiles/simple_ng.dir/simple_ng.o
[100%] Linking C executable simple_ng
[100%] Built target simple_ng
```

Outcome: PASS for compilation of the pinned Linux library and sample, based
on supplied terminal output. No sample execution or frame exchange was part
of this command. Temporary build paths may disappear after cleanup/restart.

## WSL sample entry-point check

The student executed the newly built sample without an interface argument:

```sh
(
  set +e
  /tmp/soem-05a.DuYd8f/samples/simple_ng/simple_ng
  soem_status=$?
  echo "Exit status: $soem_status (expected: 1 for usage)"
)
```

Supplied output:

```text
Usage: simple_ng IFNAME1
IFNAME1 is the NIC interface name, e.g. 'eth0'

Available adapters:
    - lo  (lo)
    - eth0  (eth0)
    - ecat_rx  (ecat_rx)
    - ecat_tx  (ecat_tx)
Exit status: 1 (expected: 1 for usage)
```

Outcome: PASS for startup, adapter enumeration and expected usage status.
Interface names alone do not establish link state or successful frame exchange.
The invocation sends no frames. Final WSL frame-format/WKC demo checks are recorded below; these are software-only checks, not a new EtherCAT network experiment.

## WSL host demos — final checks

The student compiled both existing C demos with
`gcc -std=c11 -Wall -Wextra -Werror`, using a fresh temporary directory,
and ran both under `set -e`. Supplied output:

```text
EtherType: 0x88A4
EtherCAT type: 1
EtherCAT datagrams length: 14 bytes
Command: 0x07 (BRD: broadcast read)
Datagram data length: 2 bytes
Working counter: 0
Local format checks: PASS
Constructed 60-byte Ethernet buffer (not transmitted)
Case A: both slaves respond
  Received WKC: 2 (expected for healthy network: 2)
  Read data:    0x0002
  Master check: OK
  Simulation check: PASS

Case B: only one slave responds
  Received WKC: 1 (expected for healthy network: 2)
  Read data:    0x0002
  Master check: MISSING RESPONSE(S)
  Simulation check: PASS

Case C: neither slave responds
  Received WKC: 0 (expected for healthy network: 2)
  Read data:    0x0000
  Master check: MISSING RESPONSE(S)
  Simulation check: PASS

Both host demos exited successfully
```

Outcome: PASS for local frame format and all three synthetic BRD WKC cases.
`MISSING RESPONSE(S)` is expected for the deliberately incomplete cases; it
is not a failed simulation. No frames or real slave responses were involved.

## 5A completion

The agreed source-investigation and host-check scope is complete: pinned SOEM
revision, Linux TX/RX/index matching/retry/time inspection, candidate sDDF
mapping, Linux library/sample build, adapter enumeration and host demos.
WSL outcomes are student-reported terminal evidence; cloud checks are recorded
separately. No new WSL raw-frame exchange, seL4/sDDF integration or hardware
timing claim is made. 5B implementation has not started.
