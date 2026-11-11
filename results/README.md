# Results index

- EtherCAT experiments 1–2: observations in
  [frame-handling instructions](../tests/01-frame-handling/README.md);
  new cloud executions are also captured in the 5A host transcripts.
- EtherCAT experiment 3: observations in
  [virtual Ethernet instructions](../tests/02-virtual-ethernet/README.md).
  No separate capture is stored here.
- **EtherCAT experiment 4:** [03-soem-network](03-soem-network/README.md).
  The folder number differs from the experiment number because experiments
  1 and 2 share the first test folder.
- EtherCAT experiment 5A: [cloud validation](05a-soem-interface/README.md).
  Student [WSL source inspection](05a-soem-interface/wsl-inspection.md) is recorded;
  WSL build, sample adapter enumeration and both host demos passed.
  The agreed 5A source-investigation and host-check scope is complete.
- seL4 foundation experiments 00–05: `sel4/`, with corresponding source and
  methodology under [tests/sel4](../tests/sel4/).

Existing logs are historical evidence. Reanalysis of those logs is not a new
hardware or QEMU run. Keep future runs separate and record their environment.

- EtherCAT experiment 5B, first Linux milestone: [cloud results](05b-linux-transport/README.md).
  The expanded three-suite runner passes with synthetic peers on cloud and
  [student WSL](05b-linux-transport/wsl-results.md). Sanitizer checks passed
  on cloud. Adapted veth TX and synthetic-response RX over veth passed on WSL;
  the probe checked payload 0x0002/WKC=2 and exited 0. This completes the
  tested Linux milestone; seL4/sDDF integration remains unrun.

- EtherCAT experiment 5C: [student WSL environment inventory](05c-sel4-network-driver/README.md).
  The x86-64 host has Make; QEMU AArch64, Clang, LLD and DTC were not found on
  PATH. The SDK location is unknown. Installation and all 5C driver/runtime
  checks remain pending; this is preparation evidence only.
