# Experiment 5B — student WSL evidence

Provenance: terminal transcript supplied by the student in chat, preserved in
[wsl-tests.txt](wsl-tests.txt) with line endings normalized. These commands were
not executed remotely by Codex on the student's machine.

The transcript shows the student fetched and switched to
`experiment-05b-linux-transport` and ran:

```sh
source ~/tools/cmake-venv/bin/activate
bash tests/05b-linux-transport/run.sh ~/FYP/deps/SOEM
```

Output directory: `/tmp/ethercat-05b.wXt9sH`.
Compiler reported GNU 13.3.0; Python 3.12.3. CMake 4.4.4 was verified in the
prior 5A session in this virtual environment; this runner does not print its
version again. The preparation script accepted the clean pinned SOEM checkout.

| Check | Student-supplied outcome |
|---|---|
| Library, tests and adapted simple_ng build | PASS |
| linux_transport suite | PASS |
| soem_transport suite | PASS |
| CTest total | 100% tests passed, 0 failed out of 2 |
| Sanitizer run on WSL | UNRUN; sanitizer evidence is cloud-only |
| Adapted AF_PACKET/veth TX | PASS; three frames captured on peer |
| Adapted AF_PACKET/veth RX with a synthetic reply | PENDING |
| Real slave exchange, seL4/sDDF, hardware timing | UNRUN |

The synthetic-peer tests cover byte preservation, invalid/truncated input,
backpressure/recovery, buffer reuse, deadlines/interruption, WKC=0 versus no
response, retry, index matching and Linux socketpair integration. Their success
does not demonstrate NIC/slave communication. No raw Ethernet frame was sent
by this runner. A separate raw TX check is recorded below.

## Adapted SOEM veth TX check

The student first confirmed `ecat_tx@ecat_rx` and `ecat_rx@ecat_tx`, both UP
and LOWER_UP, type veth and MTU 1500. tcpdump was `/usr/bin/tcpdump`; the adapted
sample existed at the build path above. The peer capture and sample execution
were run in separate terminals. See [supplied TX output](wsl-veth-tx.txt).

Three 29-byte EtherType 0x88A4 frames were captured on ecat_rx. Their bytes
match the saved Experiment 4 capture: broadcast destination, source
01:01:01:01:01:01, BWR command 0x08, index 1. This is a comparison to historical
bytes, not a fresh simultaneous baseline measurement. tcpdump reported three
captured, 19 received by filter, zero dropped by kernel; capture was limited
to three frames. These counts do not imply 19 saved packet records.

The adapted sender initialized ecat_tx, reported no slaves found and exited 0.
Its exit code alone is not a discovery success criterion. The capture provides
the TX evidence. It does not test the master's receive path, real slave
responses, cyclic process data, seL4 integration or real-time performance.
The capture timestamps are retained as supplied, without deriving timing claims.

Next gate: a deliberately synthetic reply on veth using the new dedicated
probe and mock peer. Cloud tests of that helper use local socketpairs only;
its AF_PACKET/veth execution remains pending on the student's WSL machine.
