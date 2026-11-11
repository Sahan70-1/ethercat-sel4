# Experiment index

There are two tracks: EtherCAT/Linux and seL4 foundations. Their numbers are
independent. Historical folder numbers are retained to preserve existing links.

| EtherCAT experiment | Instructions / code | Recorded evidence |
|---|---|---|
| 1 — Frame construction | [01-frame-handling](01-frame-handling/README.md), `frame_demo.c` | README observations; fresh cloud checks also in 5A results |
| 2 — Simulated Working Counter | [01-frame-handling](01-frame-handling/README.md), `ethercat_wkc_demo.c` | README observations; fresh cloud checks also in 5A results |
| 3 — Virtual Ethernet | [02-virtual-ethernet](02-virtual-ethernet/README.md) | README observations; no separate saved capture |
| 4 — SOEM Linux transmission | [03-soem-network](03-soem-network/README.md) | [Saved output and capture](../results/03-soem-network/README.md) |
| 5A — SOEM interface audit | [05a-soem-interface](05a-soem-interface/README.md) | [Cloud checks](../results/05a-soem-interface/README.md); student WSL source inspection, build, adapter enumeration and host demos recorded; 5A scope complete |
| 5B — Linux transport, first milestone | [05b-linux-transport](05b-linux-transport/README.md) | [Cloud synthetic tests](../results/05b-linux-transport/README.md); WSL 3/3 local suites, adapted veth TX and synthetic veth RX passed; Linux scope complete |
| 5C — seL4 network driver and SOEM | [Preparation, boot runner and diagnostics](05c-sel4-network-driver/README.md), [scope](../docs/experiment-05c-sel4-network-driver.md) | [Student SDK/cloud UART boot](../results/05c-sel4-network-driver/README.md) passed; WSL image generation passed but first boot check failed; diagnosis next; network/SOEM unrun |

Experiments 1 and 2 share one folder. Consequently Experiment 4 is in a folder
starting with `03`. It is present; it does not need duplicate source files.
It used upstream SOEM's `simple_ng`, which remains in the external SOEM checkout.

The separate seL4 track lives under `sel4/`: 00 boot, 01 notifications,
02 shared memory, 03 PPC latency, 04 notification latency and 05 timer jitter.
Each directory contains its own README and points to `results/sel4/`.
These provide foundations for the port, not an integrated SOEM/sDDF driver.

To reproduce the completed Linux milestone, use the
[5B WSL transport checks](05b-linux-transport/README.md).
The next integration gate is network-driver TX/RX in seL4/QEMU before SOEM;
see the [5C scope and driver-choice discussion](../docs/experiment-05c-sel4-network-driver.md).
