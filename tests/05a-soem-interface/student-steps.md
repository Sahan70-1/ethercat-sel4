# Experiment 5A — practical steps on your WSL computer

Goal: identify where SOEM depends on Linux, then decide what seL4/sDDF must
provide. No adapter implementation or slave hardware is needed for this step.
The teammate's seL4 foundations are enough to begin this investigation; later
sDDF integration still needs a working NIC path and scheduling/timer tests.

## Step 1 — identify your existing SOEM checkout

Open your Ubuntu/WSL terminal. Your verified SOEM checkout is `~/FYP/deps/SOEM`. Run:

```sh
(
  cd ~/FYP/deps/SOEM || exit
  pwd
  git rev-parse HEAD
  git status --short
  ls oshw/linux osal/linux
)
```

If that directory does not exist, stop and report the error or the actual
checkout location. Do not clone over an existing folder or reset local work.
Send the output before proceeding so the instructions match your actual version.
The cloud audit used `49fc34923b973c17700592319159af22157350c9`.
A different revision needs comparison, not an automatic reset.

## Step 2 — locate networking operations

```sh
cd ~/FYP/deps/SOEM
grep -nE '^(int|static int|uint8|void) (ecx_|ec_setupheader)' oshw/linux/nicdrv.c
grep -nE '\b(socket|setsockopt|ioctl|bind|send|recv|ppoll|close)\(' oshw/linux/nicdrv.c
```

## Step 3 — read the primary TX/RX functions

```sh
cd ~/FYP/deps/SOEM
sed -n '269,298p' oshw/linux/nicdrv.c
sed -n '335,361p' oshw/linux/nicdrv.c
```

Send the output for discussion. TX passes a full frame and its length to
`send`; RX requests a nonblocking read into a temporary buffer. Index matching
and WKC parsing occur later, in `ecx_inframe`.

## Remaining steps

1. Locate initialization, TX, RX, wait and retry functions in `nicdrv.c`.
2. Read the Linux socket calls and OSAL timing operations in small sections.
3. Build and run the host checks in `verify.sh` once tools and revision match.
4. Record your commands and output separately from the cloud run, then review
   the Linux-to-seL4 mapping together before beginning 5B.

Status: steps 1 and 2 confirmed by student-provided terminal output; step 3
and WSL build/runtime checks pending. See the [WSL inspection record](../../results/05a-soem-interface/wsl-inspection.md).
