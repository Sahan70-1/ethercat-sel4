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

## Step 4 — response matching, waits and time

```sh
cd ~/FYP/deps/SOEM
sed -n '363,472p' oshw/linux/nicdrv.c
sed -n '485,548p' oshw/linux/nicdrv.c
sed -n '591,647p' oshw/linux/nicdrv.c
sed -n '1,85p' osal/linux/osal.c
```

These excerpts were supplied and discussed. Monotonic deadlines and matching
remain SOEM semantics that a future adapter must preserve.

## Step 5 — fresh Linux build check

Run in Ubuntu/WSL. CMake >=3.28 is required. The verified existing environment
is `~/tools/cmake-venv`; activate it in each new terminal using
`source ~/tools/cmake-venv/bin/activate`. It provides CMake 4.4.4. The fresh temporary directory preserves the existing SOEM build.

```sh
(
  set -e
  cd ~/FYP/deps/SOEM
  cmake --version
  gcc --version | head -1
  soem_build_dir=$(mktemp -d /tmp/soem-05a.XXXXXX)
  echo "Build directory: $soem_build_dir"
  cmake -S . -B "$soem_build_dir" -DSOEM_BUILD_SAMPLES=ON
  cmake --build "$soem_build_dir" --target soem simple_ng -j 2
)
```

Send the output. Compilation does not test Ethernet exchange. Host demo and
sample entry-point checks follow after the build; see `verify.sh` for the
complete cloud reproduction procedure.

## Step 6 — sample usage and adapter list

The student's successful build is `/tmp/soem-05a.DuYd8f`.
Run without an interface argument:

```sh
(
  set +e
  /tmp/soem-05a.DuYd8f/samples/simple_ng/simple_ng
  soem_status=$?
  echo "Exit status: $soem_status (expected: 1 for usage)"
)
```

Expected behavior: usage text and available adapters; exit status 1 is the
sample's documented usage path. This invokes interface enumeration, not frame
transmission. Do not add `sudo` or an interface name for this check. If the
file is missing, rebuild using step 5 and use the newly printed directory.

## Step 7 — final host demo checks

These existing experiments provide a small regression check before closing 5A.
Run from the student's existing project checkout; outputs go to a fresh
temporary directory so no existing executable is overwritten.

```sh
(
  set -e
  cd ~/FYP/ethercat-sel4
  demo_dir=$(mktemp -d /tmp/ethercat-05a-demos.XXXXXX)
  gcc -std=c11 -Wall -Wextra -Werror tests/01-frame-handling/frame_demo.c -o "$demo_dir/frame_demo"
  gcc -std=c11 -Wall -Wextra -Werror tests/01-frame-handling/ethercat_wkc_demo.c -o "$demo_dir/wkc_demo"
  "$demo_dir/frame_demo"
  "$demo_dir/wkc_demo"
  echo "Both host demos exited successfully"
)
```

Expected: local format PASS, all three simulated WKC cases PASS, final success
line. Send the output. These programs do not send frames or use real slaves.

Status: steps 1–7 confirmed from student output. The agreed 5A investigation
and host checks are complete; 5B implementation remains future work.
See the [WSL inspection record](../../results/05a-soem-interface/wsl-inspection.md).
