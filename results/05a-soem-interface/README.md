# Experiment 5A validation — 2026-11-04

Environment: Debian 13, x86-64 cloud host; GCC 14.2.0; CMake 4.4.4
installed in `/workspace/setup-ethercat/python-tools` using pip.
SOEM: `49fc34923b973c17700592319159af22157350c9`.
Command (from repository root):

```sh
PATH=/workspace/setup-ethercat/python-tools/cmake/data/bin:$PATH \
  bash tests/05a-soem-interface/verify.sh /workspace/dependencies/SOEM
```

| Check | Outcome |
|---|---|
| Frame construction demo | PASS, local format checks |
| WKC demo | PASS, all three simulated cases |
| Pinned SOEM static library and simple_ng build | PASS |
| simple_ng usage/interface enumeration in restricted sandbox | FAILED: signal 11 |
| Same checks with interface access permitted | PASS; script exit 0; sample's expected usage exit 1 |
| Raw frame TX/RX, slave discovery, seL4/sDDF runtime | NOT RUN |

Both full transcripts are retained: [restricted run](host-validation.txt),
[run with interface access](host-validation-with-network-access.txt).
The script's PASS line only covers its listed host checks.

## Failure diagnosis

A direct libc `if_nameindex()` probe returned NULL with errno 1 (EPERM) in the
restricted sandbox. In pinned SOEM `oshw/linux/oshw.c`, `oshw_find_adapters`
iterates `ids[i]` without checking NULL. The restricted `simple_ng` usage path
therefore crashed. Repeating with the command sandbox restriction lifted
returned the adapter list and the expected usage exit status 1. The experiment
records an upstream error-handling defect triggered by restricted capabilities;
no upstream patch was made. Build success alone would have missed this.

## Existing evidence reanalysis

The existing PPC, notification and timer analysis scripts were run on copies
of `tests/` and `results/` in a fresh temporary directory. Only copied timer
analysis outputs were removed to allow regeneration; original evidence was
preserved. Scripts exited 0 and accepted respectively 30,000, 90,000 and
60,000 timed samples. This does not prove that saved runs were independent
and is not a new execution of any QEMU benchmark.

From repository root the following existing manifest checks exited 0:

```sh
sha256sum -c results/sel4/04-notification-latency/sha256-manifest.txt
sha256sum -c results/sel4/05-timer-jitter/sha256-manifest.txt
```

See [repository review](../../docs/repository-map.md) for identical timer logs
and reproduction limitations. Build outputs and downloaded dependencies remain
outside the repository. No generated binary is committed.

[Student WSL source inspection](wsl-inspection.md): checkout and function/call
locations and Linux library/sample build confirmed from supplied output;
sample startup/adapter enumeration also passed; final WSL host demos pending.
