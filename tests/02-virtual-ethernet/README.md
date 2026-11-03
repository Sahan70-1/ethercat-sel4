# Experiment 3: Virtual Ethernet Communication

## Objective
Transmit an EtherCAT-formatted Ethernet frame between
two Linux virtual Ethernet interfaces.

## Environment
- Ubuntu 24.04 on WSL2
- Python 3
- Linux veth interfaces
- tcpdump

## Setup
sudo ip link add ecat_tx type veth peer name ecat_rx
sudo ip link set ecat_tx up
sudo ip link set ecat_rx up

## Receiver
sudo tcpdump -i ecat_rx -nn -e -XX -c 1 'ether proto 0x88a4'

## Sender
sudo python3 send_frame.py

## Observed Results
- Frame size: 60 bytes
- EtherType: 0x88A4
- Frames captured: 1
- Packets dropped: 0
- Transmission and capture successful

## Limitations
This experiment uses Linux virtual Ethernet interfaces.
No physical EtherCAT slaves, SOEM transmission, seL4 or QEMU
were involved.

The results demonstrate virtual Ethernet frame transport,
not full EtherCAT master operation or real-time performance.
