# Paper review: EtherCAT control architecture for humanoid robots

Reviewed 2026-11-11 to inform Experiment 5C. This is a literature review,
not a reproduction of the authors' experiments or a seL4 driver test.

## Source and method

Felix Sygulla, Robert Wittmann, Philipp Seiwald, Tobias Berninger,
Arne-Christoph Hildebrandt, Daniel Wahrmann and Daniel Rixen,
*An EtherCAT-Based Real-Time Control System Architecture for Humanoid Robots*.

- [Original PDF link](https://mediatum.ub.tum.de/doc/1394924/0185778354196.pdf).
- The student supplied `0185778354196.pdf` after the web download was blocked.
  It is an eight-page research paper, rather than a thesis.
- SHA-256 of the uploaded file:
  `2d2c11ca62e077f1d9ace128dd33024b0b67f9f29885c6d9ceacd0cce2f06876`.
- All eight pages were read through layout-preserving text extraction.
  Page 4/Figure 4 and page 7/Figure 7 were also inspected visually.
- Page references below count PDF pages from 1. Publication year/venue are
  not inferred from the PDF creation timestamp.

The paper is reference material. Its architecture does not establish our
supervisor's implementation requirements or override the student's direction.
The copyrighted PDF is not copied into this repository.

## What the paper implements

| Aspect | Evidence in the uploaded paper |
|---|---|
| Application | LOLA humanoid robot with decentralized joint controllers |
| Master operating system | QNX Neutrino 6.6, on an Intel i7 computer; Section III, p. 3 |
| EtherCAT master | Commercial Acontis stack; Section IV-A, p. 4 |
| Author-developed software | Hardware-layer framework, bus middleware, device abstractions and control methods; Sections IV-V, pp. 4-5 |
| Communication structure | Planning, stabilization and Hardware-Driver processes, using shared memory; Section IV-A, p. 4 |
| Scheduling | Main, higher-priority EtherCAT and highest-priority timing threads; Section IV-D, pp. 4-5, Tables I-II and Figure 7 on p. 7 |
| Evaluation | Software timing, physical bus performance, clock synchronization and robot joint tracking; Section VI, pp. 6-7 |

This paper does not present a SOEM port, a seL4 implementation, sDDF integration,
or instructions for implementing an Ethernet NIC driver from scratch.
The separate vision computer runs Linux; the EtherCAT master runs QNX.

## The meaning of "driver" in this paper

The Hardware-Driver Process contains the middleware and device abstractions.
It handles robot data, EtherCAT-stack calls, errors, safety and communication
with the planning/stabilization processes. Its EC thread copies PDO data and
calls the master stack to receive and submit frames.

That application-level process is different from a NIC driver controlling
device registers, DMA descriptors and interrupts. The paper does not document
such a controller implementation. Consequently, its use of "Low-Level Hardware
Driver" is not evidence that the authors wrote a new Ethernet NIC driver.

The paper's middleware wraps PDO/SDO communication for robot applications,
above the EtherCAT master stack. It is also different from the platform
functions SOEM needs below its protocol code to send/receive frames and wait.
Our earlier term "adapter" referred to those platform functions; it does not
require the paper's robot middleware or a separate adapter protection domain.

## Reported measurements and their limits

These are the authors' measurements, not results achieved by this project.

| Quantity | Reported result | Location |
|---|---|---|
| Software cycle | Nominal 500 us (2 kHz); observed mean 500.34 us, standard deviation 1.02 us | Section VI-A, p. 6 |
| CPU work per cycle | 86 us | Section VI-A, p. 6 |
| Bus total delay | Mean 124.1 us, standard deviation 10.2 us over 10 seconds; capture setup adds delay | Section VI-B, p. 6 |
| Total control-loop latency | 644 us at a 2 kHz bus update rate | Conclusion, p. 8 |

The paper explicitly says the delay between calling `sendFrames` and actual
bus transmission was unknown (Section VI-A). CPU execution, cycle interval,
bus delay and end-to-end latency must therefore remain separate quantities.
Its suggestion of possible bus rates above 4 kHz is not a demonstrated
4 kHz seL4 result. These measurements do not prove our hardware deadlines
or a worst-case bound.

## Consequences for Experiment 5C

The student's current objective is an EtherCAT master on seL4 with a functioning
network driver and measured timing/isolation behavior. The proposed order is:

1. Establish raw Ethernet TX/RX through a seL4 network driver in QEMU.
2. Port SOEM's required platform services and connect it to that tested path.
3. Check synthetic EtherCAT frame matching and timeouts, then actual slave
   communication when suitable hardware is available.
4. Measure scheduling, driver/PD communication and end-to-end timing separately.

The paper supports the relevance of scheduling, shared-memory communication
and separate timing measurements. It does not choose our NIC, establish that
an existing driver meets the requirements, or require a driver from scratch.
The existing sDDF VirtIO-net source is a candidate/reference; the reuse versus
new implementation decision must be explicit and its provenance recorded.

See the [5C scope and first steps](experiment-05c-sel4-network-driver.md).
