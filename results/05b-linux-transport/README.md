# Experiment 5B — cloud evidence

Scope: first single-port Linux transport milestone. Student WSL ordinary
tests also passed: see [WSL evidence](wsl-results.md). Adapted AF_PACKET/veth
TX is observed on WSL; synthetic-response RX remains pending. Existing experiments are preserved.

Host: Debian 13 x86-64; GCC 14.2.0; CMake 4.4.4; Python 3.12.14.
SOEM source: `49fc34923b973c17700592319159af22157350c9`.

Executed from project root:

```sh
PATH=/workspace/setup-ethercat/python-tools/cmake/data/bin:$PATH \
  bash tests/05b-linux-transport/run.sh /workspace/dependencies/SOEM \
  /workspace/setup-ethercat/05b-final

PATH=/workspace/setup-ethercat/python-tools/cmake/data/bin:$PATH \
  ENABLE_SANITIZERS=ON bash tests/05b-linux-transport/run.sh \
  /workspace/dependencies/SOEM /workspace/setup-ethercat/05b-sanitized
```

Both runner executions exited 0. After strengthening the TX-copy reuse
assertion, the ordinary build/test targets were rebuilt and rerun; the attached
ordinary logs are from that final run. Sanitizer tests already include it.

| Check | Outcome | Evidence |
|---|---|---|
| Linux transport suite | PASS | [CTest transcript](cloud-tests.txt) |
| Adapted actual SOEM suite with synthetic responses | PASS | Same transcript; 2/2 CTest suites passed |
| Build adapted library/sample/test executables | PASS, no compiler warnings in final build | [Build log](cloud-build.txt) |
| AddressSanitizer + UndefinedBehaviorSanitizer | PASS, 2/2 suites; no diagnostics | [Sanitized CTest transcript](cloud-sanitizer-tests.txt) |
| Adapted `simple_ng` usage/adapter enumeration | PASS; expected usage exit 1 | [Sample transcript](cloud-sample-usage.txt) |
| AF_PACKET socket creation | BLOCKED: PermissionError, errno 1, Operation not permitted | Read-only socket-create probe; no frames sent |
| Cloud raw veth/NIC checks, slaves, seL4/sDDF, physical timing | UNRUN | Student veth TX is recorded separately |

## Problems found and resolved during development

- Initial CMake generation failed because an exported SOEM target depended on
  an unexported transport target. The test harness now adds transport sources
  directly to SOEM and its build include path. This harness does not package
  an installed SDK.
- Initial restricted-sandbox local datagram sends failed a test assertion.
  The same suite passed with command socket access enabled. All final recorded
  suites used that access; no network destination or NIC traffic was involved.
- Initial generated RX checks produced signedness warnings; the generated
  comparisons were corrected and rebuilt without warnings.

SOEM source and application assertions were not disabled to pass tests.
Generated copies/build outputs remain outside the project checkout. No
upstream dependency files were changed. Setup repeatability was exercised
using fresh ordinary and sanitizer output directories.

The tests cover synthetic reply matching and WKC behavior; they cannot prove
slave compatibility or real network performance. The first Linux milestone is
reviewable, while the real raw-network validation gate remains open.


## Veth RX helper validation (later addition)

The probe and Python mock peer were added after the student's successful veth
TX check. The expanded cloud runner passes three CTest suites, including five
Python cases against the C probe over local socketpairs. See
[helper test transcript](cloud-rx-helper-tests.txt). This is not execution of
the AF_PACKET path; that remains blocked in the cloud. The student's earlier
2/2 transcript predates this third suite and remains unchanged.

The same expanded runner also passed 3/3 suites with AddressSanitizer and
UndefinedBehaviorSanitizer: [sanitizer transcript](cloud-rx-helper-sanitizer-tests.txt).
No sanitizer diagnostics were emitted.

Trailing whitespace in the two helper CTest transcripts was normalized;
all output text and results are retained.
