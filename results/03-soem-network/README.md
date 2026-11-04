# Experiment 4 — SOEM Linux transmission evidence

These are the original saved artifacts; no new network run was performed when
this index was added.

- [soem-output.txt](soem-output.txt): interface initialization succeeded,
  followed by `no slaves found`.
- [tcpdump.txt](tcpdump.txt): a captured 29-byte Ethernet frame with EtherType
  `0x88a4`; the EtherCAT command byte is `0x08` (Broadcast Write).

Together these support the recorded Linux SOEM transmit-path observation.
They do not demonstrate a slave response, successful discovery, cyclic
communication or timing guarantees. The saved text excerpts do not contain
complete run metadata or a packet-capture file; do not invent missing details.

[Experiment instructions and environment](../../tests/03-soem-network/README.md).
