# Experiment 5B — first Linux transport implementation

This milestone implements a copying raw-frame interface and connects it to a
**disposable copy** of pinned SOEM. It tests actual SOEM frame construction,
response matching, WKC and retries using synthetic peers. It is not a seL4 port.
The existing seL4 foundation experiments are sufficient to begin this Linux
work; no new seL4 test is a prerequisite here.

## Run on the student's WSL computer

Requires GCC, Make, Git, Python >=3.12 and CMake >=3.28. Reuse the verified
CMake environment. The SOEM checkout must be clean and at the 5A revision
`49fc34923b973c17700592319159af22157350c9`.

From the project root on branch `experiment-05b-linux-transport`:

```sh
source ~/tools/cmake-venv/bin/activate
bash tests/05b-linux-transport/run.sh ~/FYP/deps/SOEM
```

Expected: three CTest suites pass, followed by a PASS line. Send the terminal
output, including the printed output-directory path. The runner retains
configure/build/test logs in that directory. No `sudo`, interface setup, NIC
frames or slave hardware is required. The runner uses AF_UNIX datagram
socketpairs to test frame-byte transport locally. These are synthetic peers,
not virtual EtherCAT slaves. Sandbox restrictions on local socket traffic can
block these tests even when compilation succeeds.

To check memory/undefined behavior separately, use a new output directory:

```sh
ENABLE_SANITIZERS=ON bash tests/05b-linux-transport/run.sh ~/FYP/deps/SOEM
```

An optional second argument chooses a **new, nonexistent** output directory.
The default creates a fresh directory under `/tmp`. Repeated runs preserve all
previous outputs and do not patch your existing SOEM source or build.

## What changed

- `src/transport/ec_transport.h/.c`: portable full-frame operations, explicit
  results and size/ownership rules.
- `src/transport/ec_linux_transport.h/.c`: borrowed socket endpoint; nonblocking
  send/receive, truncation detection, monotonic deadlines and interrupt-safe wait.
- `prepare_soem.py`: checks the pinned clean checkout, exports committed source,
  then applies exact, checked substitutions to the disposable copy. The source
  checkout is not modified. Existing upstream license notices remain in place.
- The test CMake project compiles this adapted SOEM library and `simple_ng`
  against the project transport sources. It is a test/build harness, not an
  installable packaged SOEM SDK.

## Tests and scope

| Suite | Checks |
|---|---|
| `linux_transport` | Byte preservation at 14/60/1518-byte capacities; zero/short/oversized arguments; short/truncated/oversized RX; empty RX; 256 reuse cycles; queue backpressure/recovery; expired/future deadlines; interrupted wait; closed descriptor errors |
| `mock_peer_and_probe` | Five Python checks: reply fields/padding, rejection of malformed/unrelated requests, actual C probe acceptance of a matching reply, rejection of wrong payload/index; local socketpairs only |
| `soem_transport` | Actual pinned SOEM code with mocked responses: WKC=0 vs no frame; resend then WKC=2; out-of-order indexes; truncated packets and bad WKC offsets; unexpected indexes and EtherType; TX failure; no-response deadline; 64 index reuse cycles; invalid caller index; unsupported secondary setup; SOEM TX/RX/wait through the Linux socketpair backend |

Mocks intentionally create responses and WKCs; no discovery or hardware
response is claimed. Frame sizes here are transport-capacity tests, not claims
that every tested size is a valid on-wire EtherCAT frame. This milestone has
not tested VLANs, FCS/padding conventions, redundancy or replay protection.

## Recorded outcome and next integration gate

The first Linux milestone passed: the student ran all three local suites,
captured adapted `simple_ng` TX over veth, and received the dedicated mock
reply with payload 0x0002/WKC=2 and probe exit 0. See
[WSL evidence](../../results/05b-linux-transport/wsl-results.md).
Cloud AF_PACKET creation returns EPERM, so raw veth exchange is student-tested
only. The automated runner itself uses local socketpairs and does not test veth.

Next is standalone sDDF VirtIO-net TX/RX in QEMU before connecting SOEM.
OSAL/lifecycle, buffer ownership and Microkit wait scheduling remain to be
implemented and tested. Real slaves and physical timing remain later gates.

[Design and limitations](../../docs/experiment-05b-linux-transport.md).
[Cloud results](../../results/05b-linux-transport/README.md).

## Reproduce from Windows: WSL login, local tests and veth RX

The responder accepts only the probe's single BRD read with zero initial
payload/WKC. It preserves the index and returns fabricated payload 0x0002 and
WKC=2. It does not implement discovery, state transitions or any slave device.
Use only the isolated veth pair. The probe checks both data and WKC and exits
1 on a missing/mismatched response. Its timeout is a functional limit, not a
performance measurement.

1. Open Windows PowerShell (Terminal 1) and enter WSL:

   ```powershell
   wsl -u sahan
   ```

2. Open the project and check for local changes:

   ```sh
   cd ~/FYP/ethercat-sel4
   git status --short
   ```

   If changes are listed, resolve them before switching or pulling. Otherwise:

   ```sh
   git fetch origin
   git switch experiment-05b-linux-transport
   git pull --ff-only
   source ~/tools/cmake-venv/bin/activate
   cmake --version
   ```

3. Inspect the veth pair:

   ```sh
   ip -d link show ecat_tx
   ip -d link show ecat_rx
   ```

   Existing interfaces must be reciprocal veth peers. If only one exists or
   the pair is unexpected, stop and inspect before changing anything.
   Only if both interfaces are absent, create them:

   ```sh
   sudo ip link add ecat_tx type veth peer name ecat_rx
   ```

   Bring the verified pair up:

   ```sh
   sudo ip link set ecat_tx up
   sudo ip link set ecat_rx up
   ```

4. Build in a fresh directory and run all local tests in Terminal 1:

   ```sh
   soem_05b_session=$(mktemp -d /tmp/ethercat-05b-session.XXXXXX)
   bash ~/FYP/ethercat-sel4/tests/05b-linux-transport/run.sh \
     ~/FYP/deps/SOEM "$soem_05b_session/run"
   ```

   Expect 3/3 suites passing. Keep Terminal 1 open: it holds the directory
   variable. Generated sources and configure/build/test logs remain under
   that fresh directory; no old `/tmp` path is required.

5. Open a second PowerShell tab (Terminal 2), enter WSL, then start the peer:

   ```powershell
   wsl -u sahan
   ```

   ```sh
   sudo python3 ~/FYP/ethercat-sel4/tests/05b-linux-transport/mock_veth_peer.py ecat_rx
   ```

   Wait for `MOCK READY` and leave Terminal 2 running.

6. In Terminal 1, within 120 seconds, run:

   ```sh
   sudo "$soem_05b_session/run/build/veth_probe" ecat_tx
   echo "Probe exit status: $?"
   ```

Expected and observed on student WSL: `MOCK SENT` in Terminal 2, and the probe's
PASS line with synthetic payload 0x0002/WKC=2 and exit 0 in Terminal 1.
If the responder times out, restart it before repeating the probe. Stop at
other errors and retain the output for investigation. Record both terminal
outputs and the local test summary for each new run; an expectation is not
evidence of that run's success.

Rootless tests of the same C probe use `--test-fd` and AF_UNIX sockets;
that option is a test injection path, not a NIC interface.
