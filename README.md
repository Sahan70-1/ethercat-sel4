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

## Project Team

- E/21/198 — Jayasundara J.M.S.D.B.
- E/21/328 — Rathnayake R.M.P.M.
- E/21/353 — Sandeep Y.P.

## Project Status

Early-stage research and prototyping.

Initial areas of investigation include seL4 protection-domain communication, virtual Ethernet communication, and preparation for EtherCAT master integration.

Experimental measurements will be published after their methodology and results have been verified.

## Planned Experiments

1. seL4 inter-protection-domain communication
2. Virtual Ethernet frame communication
3. SOEM integration
4. EtherCAT slave communication
5. Real-time latency and jitter evaluation

## Documentation

Research notes, experiment instructions, and results will be maintained in the `docs/`, `tests/`, and `results/` directories as development progresses.

## License

No license has been selected yet. Reuse permissions will be determined with the project team and supervisor.
