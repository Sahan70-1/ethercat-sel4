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
  WSL build passed; sample runtime checks remain pending.
- seL4 foundation experiments 00–05: `sel4/`, with corresponding source and
  methodology under [tests/sel4](../tests/sel4/).

Existing logs are historical evidence. Reanalysis of those logs is not a new
hardware or QEMU run. Keep future runs separate and record their environment.
