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

This establishes the local revision and relevant source locations only.
Student TX/RX walkthrough, build validation and any new network execution
remain pending. No successful slave exchange or seL4 integration is claimed.
