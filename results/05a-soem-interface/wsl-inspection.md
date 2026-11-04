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
proposed sDDF mapping and remaining integration risks. WSL build validation and
any new network execution remain pending. No successful slave exchange or
seL4 integration is claimed.
