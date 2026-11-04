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
| Adapted AF_PACKET/veth exchange | PENDING |
| Real slave exchange, seL4/sDDF, hardware timing | UNRUN |

The synthetic-peer tests cover byte preservation, invalid/truncated input,
backpressure/recovery, buffer reuse, deadlines/interruption, WKC=0 versus no
response, retry, index matching and Linux socketpair integration. Their success
does not demonstrate NIC/slave communication. No raw Ethernet frame was sent
by this runner. Next check the existing veth setup before capturing the adapted
sample's raw TX path.
