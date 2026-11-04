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

Expected: two CTest suites pass, followed by a PASS line. Send the terminal
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
| `soem_transport` | Actual pinned SOEM code with mocked responses: WKC=0 vs no frame; resend then WKC=2; out-of-order indexes; truncated packets and bad WKC offsets; unexpected indexes and EtherType; TX failure; no-response deadline; 64 index reuse cycles; invalid caller index; unsupported secondary setup; SOEM TX/RX/wait through the Linux socketpair backend |

Mocks intentionally create responses and WKCs; no discovery or hardware
response is claimed. Frame sizes here are transport-capacity tests, not claims
that every tested size is a valid on-wire EtherCAT frame. This milestone has
not tested VLANs, FCS/padding conventions, redundancy or replay protection.

## Next practical gate

The student repeated both ordinary suites successfully; see [WSL evidence](../../results/05b-linux-transport/wsl-results.md).
Next validate the adapted `simple_ng` over
WSL veth plus tcpdump and compare against the original 5A/Experiment 4 path.
Use fresh output files; no slaves found is not successful slave discovery.
Cloud AF_PACKET creation currently returns EPERM, so this real Linux network
path is unrun here. Then address remaining OSAL/lifecycle and Microkit wait
scheduling design before sDDF integration.

[Design and limitations](../../docs/experiment-05b-linux-transport.md).
[Cloud results](../../results/05b-linux-transport/README.md).

## Guided veth prerequisite check (next student step)

Before sending frames, run in WSL and send the output:

```sh
ip -d link show ecat_tx
ip -d link show ecat_rx
command -v tcpdump
ls -l /tmp/ethercat-05b.wXt9sH/build/soem/samples/simple_ng/simple_ng
```

This inspects existing interfaces and checks the newly adapted binary. It does
not recreate interfaces, change link state or send frames. The veth pair should
be linked to each other and administratively UP before capture. Do not use the
old `~/FYP/deps/SOEM/build/...` binary for the adapted regression. If temporary
outputs were deleted, rerun the tests and use the new printed directory.
