# Experiment 5B — Linux transport seam, first milestone

5A located Linux dependencies; 5B now supplies the first implemented transport
seam and Linux tests. Work is on `experiment-05b-linux-transport`, based on the
reviewable 5A branch. No main merge is implied.

## Interface and ownership

`ec_transport` contains an operations table, opaque context and frame capacity.
Its send/receive/now/wait operations use `EC_IO_OK`, `AGAIN`, `TIMEOUT`,
`INVALID` and `ERROR`. TX/RX do not block. No WKC or EtherCAT index appears
in the transport API: those remain SOEM responsibilities.

A successful send means frame bytes were accepted/copied, not wire completion.
Caller TX storage may immediately be reused. Receive copies into caller
storage and reports actual length; failure reports zero length and the bytes
must not be parsed. An invalid incoming datagram is consumed and dropped.
Each endpoint has one caller; this is not a general multithreaded queue API.
Platform owners create and close resources. The Linux adapter borrows a pointer
to a live socket descriptor and never closes it. Test fixtures close their own
sockets; SOEM still owns its sockets.

A future sDDF implementation must copy TX bytes into an acquired free buffer,
submit its descriptor and reclaim it only after return. RX must copy and
return each descriptor exactly once. That implementation does not exist yet.

## Linux adapter

The adapter uses `MSG_DONTWAIT | MSG_NOSIGNAL` for TX. EAGAIN/EWOULDBLOCK,
ENOBUFS and EINTR map to AGAIN. Short sends map to ERROR.
RX uses `MSG_DONTWAIT | MSG_TRUNC`, rejecting datagrams larger than caller
capacity rather than silently parsing a prefix. Generic wrappers also reject
frames shorter than an Ethernet header or above the configured capacity.

Time uses `CLOCK_MONOTONIC` in microseconds. Wait takes an absolute deadline,
uses ppoll, and recomputes remaining time after EINTR, retaining the original
deadline. Expiry returns TIMEOUT; invalid/error/hangup descriptors return ERROR.
It permits spurious wakeups: callers must recheck queues and deadline.
No physical timing bounds or performance results follow from these tests.

## Connection to pinned SOEM

`prepare_soem.py` exports commit
`49fc34923b973c17700592319159af22157350c9` into a fresh directory and verifies
patch anchors. It refuses another revision, tracked source changes, or an
existing output directory. This avoids changing the student's existing SOEM.
The generated sources retain upstream SOEM license notices; distributing an
integrated SOEM artifact remains subject to upstream GPLv3/commercial terms.

The generated `ec_stackT` gains a transport endpoint. Primary setup binds the
Linux endpoint to SOEM's socket. `ecx_outframe` and `ecx_recvpkt` call the new
interface. The wait loop uses the endpoint clock/wait operations with its
existing OSAL deadline and at most 50-us requested wait slices. SOEM retains
index allocation, out-of-order buffering, WKC extraction and resend policy.
WKC=0 still terminates `ecx_srconfirm` retries.

Before parsing, the adapted code checks actual RX length for the EtherCAT
header and WKC position; it verifies the expected copy length fits the packet.
Invalid requested indexes are rejected before indexing arrays. These are narrow
bounds corrections needed for malformed-frame tests, not a full protocol parser
hardening or replay-prevention design.

This first milestone supports **one port**. Secondary setup explicitly returns
failure. Existing redundant-path code is retained but not qualified or exercised.
It must not be advertised as supported. Socket setup/close, OSAL clocks/sleeps,
allocation, pthreads, interface enumeration and the port's pthread types remain
Linux-specific. The new wait interface does not make those services portable.
In particular the Linux OSAL deadline and transport clock share the same epoch;
a future platform must preserve that relationship or redesign both together.

## Verification gates

1. Portable wrapper/Linux implementation: tested with local datagram sockets.
2. Adapted actual SOEM library: tested with explicit mocks and socketpair peers.
3. Adapted SOEM sample: built; usage/adapter enumeration checked separately.
4. AF_PACKET/veth TX: observed on student WSL; three captured frames match
   historical Experiment 4 bytes. Synthetic RX over veth also passed on WSL:
   the mock sent a 30-byte reply with index 1 and WKC=2; SOEM accepted payload
   0x0002/WKC=2 and the probe exited 0. Cloud raw-socket creation is denied.
5. Student WSL ordinary reproduction: all three suites passed in the latest
   screenshot; see the separate
   [student evidence](../results/05b-linux-transport/wsl-results.md).
   WSL sanitizer run is unrun; cloud sanitizer suites passed.
6. Microkit/sDDF integration, physical slaves and timing: future work.

Tests exercise failure cases as well as success, retain assertion checks even
in Release builds, and are also run under address/undefined behavior sanitizers.
They do not replace a real NIC regression. The automated suite finishes without
opening a raw socket or changing network interfaces.

## Why the teammate's seL4 work is enough for this step

5B's first milestone runs on Linux. The existing shared-memory/notification
experiments demonstrate foundations relevant to later queue integration.
They do not yet establish sDDF buffer ownership, NIC traffic, timer wakeup or
progress of a lower-priority driver while SOEM waits. Those are explicit gates
before a usable seL4 backend. Keep using the existing EtherCAT/SOEM library
rather than writing a new protocol implementation.

[WSL commands and test list](../tests/05b-linux-transport/README.md).
[Recorded cloud evidence](../results/05b-linux-transport/README.md).


The veth RX helper adds a five-case local suite using the actual C probe and
Python response filter. The filter only accepts a fixed single BRD request;
it is not a general EtherCAT parser or slave emulator. The responder returns
one fabricated reply and exits, with a bounded wait for the probe request.
Student raw TX evidence is in [the WSL record](../results/05b-linux-transport/wsl-results.md).

## Completion and next integration gate

The first 5B milestone is complete for the single-port Linux scope above.
The latest student screenshot supplies evidence of a functional synthetic
exchange through AF_PACKET/veth, in addition to local tests. Its visible
output was [transcribed separately](../results/05b-linux-transport/wsl-veth-rx.txt);
it is not a full build log or an independently executed cloud test.

5C has not started. Begin by pinning compatible Microkit/sDDF versions and
building and booting an existing sDDF VirtIO-net example in QEMU. Establish
raw Ethernet TX/RX before connecting SOEM; demonstrate buffer reclamation and
that waiting lets the driver run. Then implement the sDDF transport endpoint
and remaining OSAL/lifecycle services, with timer and scheduling tests.
Use the existing VirtIO network driver for this step. A new hardware NIC
driver is not required to begin. Real slaves, cyclic data and physical timing
remain later validation gates.
