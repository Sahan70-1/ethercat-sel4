# Experiment 4: SOEM Network Interface Investigation

The historical folder is named `03-soem-network` because experiments 1 and 2
share `01-frame-handling`. This is Experiment 4 in the EtherCAT track, separate
from seL4 Experiment 04.

## Saved results

See [the original SOEM output and tcpdump capture](../../results/03-soem-network/README.md).
This experiment uses upstream SOEM's `simple_ng` sample; no separate copy of
that program is required in this repository.

## Objective
Investigate how the SOEM EtherCAT master transmits frames using the Linux networking interface.

## Environment
- Ubuntu 24.04 on WSL2
- SOEM source commit: `49fc349`
- Linux virtual Ethernet interfaces: `ecat_tx` and `ecat_rx`
- tcpdump

## Method
1. Examined `oshw/linux/nicdrv.c` and `src/ec_base.c`.
2. Identified the network initialization, frame transmission and reception functions.
3. Ran SOEM's `simple_ng` sample on `ecat_tx`.
4. Captured EtherCAT traffic on `ecat_rx`.

## Observations
- SOEM successfully initialized the virtual Ethernet interface.
- An EtherCAT frame with EtherType `0x88A4` and a Broadcast Write command was observed.
- No EtherCAT slaves were discovered.

## Conclusion
The experiment demonstrates SOEM's Linux raw-Ethernet transmission path in a virtual environment.

## Limitations
No EtherCAT slave or valid slave response was available. This experiment does not verify cyclic EtherCAT communication, slave discovery success, or real-time performance.
