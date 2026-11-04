# Experiment index

There are two tracks: EtherCAT/Linux and seL4 foundations. Their numbers are
independent. Historical folder numbers are retained to preserve existing links.

| EtherCAT experiment | Instructions / code | Recorded evidence |
|---|---|---|
| 1 — Frame construction | [01-frame-handling](01-frame-handling/README.md), `frame_demo.c` | README observations; fresh cloud checks also in 5A results |
| 2 — Simulated Working Counter | [01-frame-handling](01-frame-handling/README.md), `ethercat_wkc_demo.c` | README observations; fresh cloud checks also in 5A results |
| 3 — Virtual Ethernet | [02-virtual-ethernet](02-virtual-ethernet/README.md) | README observations; no separate saved capture |
| 4 — SOEM Linux transmission | [03-soem-network](03-soem-network/README.md) | [Saved output and capture](../results/03-soem-network/README.md) |
| 5A — SOEM interface audit | [05a-soem-interface](05a-soem-interface/README.md) | [Cloud checks](../results/05a-soem-interface/README.md); student WSL source inspection recorded; build/runtime checks pending |

Experiments 1 and 2 share one folder. Consequently Experiment 4 is in a folder
starting with `03`. It is present; it does not need duplicate source files.
It used upstream SOEM's `simple_ng`, which remains in the external SOEM checkout.

The separate seL4 track lives under `sel4/`: 00 boot, 01 notifications,
02 shared memory, 03 PPC latency, 04 notification latency and 05 timer jitter.
Each directory contains its own README and points to `results/sel4/`.
These provide foundations for the port, not an integrated SOEM/sDDF driver.

For the next practical session, start with
[5A step 1](05a-soem-interface/student-steps.md).
