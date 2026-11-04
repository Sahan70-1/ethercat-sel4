# Experiment 5A — practical steps on your WSL computer

Goal: identify where SOEM depends on Linux, then decide what seL4/sDDF must
provide. No adapter implementation or slave hardware is needed for this step.
The teammate's seL4 foundations are enough to begin this investigation; later
sDDF integration still needs a working NIC path and scheduling/timer tests.

## Step 1 — identify your existing SOEM checkout

Open your Ubuntu/WSL terminal. If your existing SOEM checkout is `~/SOEM`, run:

```sh
(
  cd ~/SOEM || exit
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

## What follows after step 1

1. Locate initialization, TX, RX, wait and retry functions in `nicdrv.c`.
2. Read the Linux socket calls and OSAL timing operations in small sections.
3. Build and run the host checks in `verify.sh` once tools and revision match.
4. Record your commands and output separately from the cloud run, then review
   the Linux-to-seL4 mapping together before beginning 5B.

Status: student WSL output pending. The existing cloud audit and build checks
do not establish that these steps have run on your computer.
