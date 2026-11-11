# Experiment 5B — student WSL evidence

Provenance: terminal transcripts and a later screenshot supplied by the student
in chat. The original runner transcript is preserved in
[wsl-tests.txt](wsl-tests.txt) with line endings normalized. These commands were
not executed remotely by Codex on the student's machine.

Current outcome: the single-port Linux milestone passed. The latest screenshot
shows 3/3 local suites passing and a synthetic reply received over AF_PACKET/veth,
with payload 0x0002/WKC=2 and probe exit 0. The chronological evidence follows;
the original transcripts are preserved.

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
| Original CTest total | 100% tests passed, 0 failed out of 2 |
| Latest expanded CTest total | 100% tests passed, 0 failed out of 3; screenshot below |
| Sanitizer run on WSL | UNRUN; sanitizer evidence is cloud-only |
| Adapted AF_PACKET/veth TX | PASS; three frames captured on peer |
| Adapted AF_PACKET/veth RX with a synthetic reply | PASS; payload 0x0002/WKC=2, probe exit 0 |
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

At this stage, RX still needed the dedicated probe and mock peer below.
Cloud tests of that helper use local socketpairs only; the later student
AF_PACKET/veth result is recorded in the final section.


## Student RX-helper build and local test

The student updated the branch from d55b962 to fd02e6b, reconfigured the
existing `/tmp/ethercat-05b.wXt9sH/build` directory and built `veth_probe`.
The selected CTest invocation was:

```sh
ctest --test-dir /tmp/ethercat-05b.wXt9sH/build -R mock_peer_and_probe --output-on-failure
```

Result: 1/1 selected suite passed. This is an additional local socketpair test,
not a rerun of all three suites together and not an AF_PACKET/veth RX test.
The original two suites passed earlier as separately recorded above.
See [the supplied helper transcript](wsl-rx-helper-tests.txt); only the new
command/output section was retained from the pasted terminal history, with
line endings normalized. The mock responder/probe veth execution followed later.

## Student full local suite and synthetic veth RX

A later two-terminal screenshot, supplied in chat on 2026-11-11, shows:

- CTest: 100% tests passed, 0 failed out of 3; total real time 0.62 seconds.
  The third suite, `mock_peer_and_probe`, passed and its five Python cases
  reported OK. The screenshot starts partway through test output; it does not
  include the full configure/build log or individual results for suites 1–2.
- Responder on `ecat_rx`: `MOCK READY`, then
  `MOCK SENT: index=1, bytes=30, synthetic WKC=2`.
- Probe on `ecat_tx`: synthetic BRD, index 1, frame size 30 bytes; received
  WKC=2; PASS for matching payload 0x0002/WKC=2; exit status 0.

See [the visible output transcription](wsl-veth-rx.txt). Commands use the
real interface path, not `--test-fd`. The probe's PASS requires SOEM to accept
the requested index, completed buffer state, expected payload and WKC.
Together with the peer's reply output, this is evidence of a functional
synthetic request/reply over the Linux veth pair through the adapted SOEM code.

The full temporary output directory, Git HEAD, tool versions and execution
timestamp are not visible in this screenshot. Do not infer new values for
them from older runs or the date the image was supplied. The runner's
`NOT TESTED: AF_PACKET/veth` line describes the local runner alone; the
separately invoked probe and responder supply the veth evidence.

This completes the planned first Linux milestone. The mock fabricates data
and WKC; no real slave discovery or device response, seL4/sDDF traffic, cyclic
process data, reliability under load or physical timing was tested here.
Next is standalone sDDF VirtIO-net TX/RX in QEMU before adding the SOEM backend.
