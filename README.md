# EtherCAT on seL4 Microkernel

**Fourth-Year Research Project — Group 19**

University of Peradeniya | Department of Computer Engineering

## Project Overview

This research investigates the integration of an EtherCAT master with the seL4 microkernel, focusing on real-time communication, performance isolation, latency, and jitter.

The proposed architecture uses the Simple Open EtherCAT Master (SOEM), the seL4 Device Driver Framework (sDDF), and isolated protection domains.

## Research Objectives

- Understand the seL4 microkernel and EtherCAT protocol.
- Investigate integration of SOEM with seL4.
- Explore user-space Ethernet driver communication.
- Implement and evaluate inter-domain communication.
- Measure cyclic communication latency, jitter, and deadline misses.
- Evaluate suitability for real-time industrial control.

## Technologies

- seL4 Microkernel
- EtherCAT
- SOEM
- seL4 Device Driver Framework (sDDF)
- Microkit
- C, Python
- QEMU
- Linux development environment

## Start here

- [Experiment index](tests/README.md): find Experiments 1–4, 5A and the separate seL4 track.
- [Results index](results/README.md): find the evidence for each experiment.
- [Repository map and review](docs/repository-map.md): file locations, experiment numbering and known duplication.
- [Experiment 5A: SOEM network interface audit](docs/experiment-05a-soem-network-audit.md).
- [Reproduce the 5A host checks](tests/05a-soem-interface/README.md).
- [5B Linux transport implementation and tests](tests/05b-linux-transport/README.md).

There are two experiment tracks. The EtherCAT track has local frame/WKC demos,
Linux veth transmission and recorded SOEM TX with no slaves found. The seL4
track under `tests/sel4/` has recorded QEMU boot, notification, shared-memory,
PPC latency, notification latency and timer-jitter experiments. Their original
READMEs and evidence remain in place. QEMU timing is not hardware timing.

Experiment 5A is complete for its source-audit and host-check scope, with
cloud validation and student WSL results recorded. No SOEM-to-sDDF port or
physical slave communication is implemented yet. The first 5B Linux transport
milestone is complete for its tested Linux scope: three local suites pass on
cloud and student WSL, adapted veth TX was captured, and the student verified
synthetic-response RX over veth with payload 0x0002, WKC=2 and probe exit 0.
See the [student evidence](results/05b-linux-transport/wsl-results.md).
5C sDDF integration is future work, starting with standalone VirtIO-net TX/RX
in QEMU before connecting SOEM.

## Project Team

- E/21/198 — Jayasundara J.M.S.D.B.
- E/21/328 — Rathnayake R.M.P.M.
- E/21/353 — Sandeep Y.P.

## Project Status

Early-stage research and prototyping.

Existing experiments cover seL4 protection-domain communication and virtual Ethernet transmission. The next integration work starts at SOEM’s OS-dependent network boundary.

Recorded QEMU measurements and their limitations are available under `results/sel4/`; see the repository review for the validation performed here.

## Research roadmap (not experiment numbering)

1. seL4 inter-protection-domain communication
2. Virtual Ethernet frame communication
3. SOEM integration
4. EtherCAT slave communication
5. Real-time latency and jitter evaluation

## Documentation

Research notes, experiment instructions, and results will be maintained in the `docs/`, `tests/`, and `results/` directories as development progresses.

## License

No license has been selected yet. Reuse permissions will be determined with the project team and supervisor.
