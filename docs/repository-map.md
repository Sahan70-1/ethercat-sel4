# Repository map and review

Reviewed 2026-11-04 against main `85c925705cfa42000c978275e1081ad265726dc1`
(147 tracked files). GitHub main matched the local checkout; it had no local changes.
The handoff was used as historical context and checked against repository evidence.

## Where things belong

| Location | Purpose |
|---|---|
| `docs/` | Cross-experiment explanations and port design |
| `tests/01-frame-handling/` | EtherCAT experiments 1 and 2: format and simulated WKC |
| `tests/02-virtual-ethernet/` | EtherCAT experiment 3: Python raw frame on Linux veth |
| `tests/03-soem-network/` | EtherCAT experiment 4: SOEM TX investigation |
| `tests/05a-soem-interface/` | EtherCAT experiment 5A: source audit validation |
| `tests/sel4/00-*` through `05-*` | Separate seL4 track: boot, notification, shared memory, PPC, notification latency, timer jitter |
| `results/` | Recorded evidence, with paths matching the relevant test group |
| `src/` | Reserved for integration code; no SOEM port exists here yet |
| `turorial/` | Legacy empty placeholder with a spelling error; retained to avoid unnecessary history churn |

Folder numbers in the original EtherCAT track do not equal experiment numbers:
`01-frame-handling` contains two experiments. Always include the track and name.
The seL4 `master.c` and `driver.c` names describe demo roles, not a working
SOEM master or Ethernet NIC driver.

## Is main clean and duplication-free?

The working tree was clean. Source and recorded results are sensibly separated.
The root overview needed a current status/index. The repository is **not**
duplication-free; the following are actual byte-identical content groups:

- Shared-memory experiment 02 and the functional part of seL4 experiment 04
  repeat `master.c`, `driver.c`, protocol header and system description.
- The notification benchmark and its pilot share Makefile, protocol, timer
  helpers and system configuration. Their master programs differ.
- Timer pilots repeat Makefiles and several system descriptions. `pilot/timer.c`
  equals `pilot-budget-10ms/timer.c`, but their system configurations differ.
- Empty `.gitkeep` files and repeated `.gitignore` files are harmless scaffolding.
- The three `full-icount-run0[123]-raw.log` timer logs are byte-identical
  (SHA-256 `147d146e17beb53783b76d04653b4aadd316a3a2d63c352848af4624c0812177`).
  Deterministic instruction-counted execution can produce identical output;
  file contents alone cannot prove independent boots. Preserve the logs and
  record per-run launch metadata for future measurements. This is not evidence
  sufficient to accuse anyone of copying or fabrication.

Do not deduplicate historical experiments casually: self-contained snapshots
preserve reproduction context. Use shared helpers for future experiments only
when doing so does not change recorded experiments.

## Evidence review

The source, READMEs, runners, saved outputs and file inventory were reviewed.
This is a structure/source review, not a proof that every implementation is correct.
Existing analysis scripts were executed on temporary copies, so tracked CSVs
and logs were not overwritten:

- PPC: 3 logs, 30,000 timed samples and 3,000 baseline samples accepted.
- Notification latency: 3 logs, 90,000 timed samples and 3,000 baselines accepted.
- Timer: 6 logs, 60,000 samples accepted; default TCG and artificial icount time
  remain separate. The analysis checks content, not independent-run provenance.
- Existing notification and timer `sha256-manifest.txt` checks passed.
- The two host C demos were compiled and executed successfully.

These are reanalysis results, **not new QEMU executions**. No seL4 image was
built or booted during 5A. No physical EtherCAT slave is demonstrated by the
current repository. The saved SOEM output explicitly says `no slaves found`.
The raw virtual-Ethernet experiment has a README observation but no dedicated
capture artifact under `results/02-virtual-ethernet/`.

Future captures should use fresh run directories with commands, tool versions,
commit IDs, host/clock mode and timestamps. Some existing analysis scripts write
fixed paths or refuse existing outputs; run them on temporary copies when reviewing.
