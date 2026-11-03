
import socket
import struct

INTERFACE = "ecat_tx"
ETHERTYPE = 0x88A4

# Ethernet header
destination = b"\xff" * 6
source = bytes.fromhex("02 00 00 00 00 01")

ethernet_header = (
    destination
    + source
    + struct.pack("!H", ETHERTYPE)
)

# EtherCAT Broadcast Read datagram
command = 0x07
index = 0x01

datagram = (
    bytes([command, index])
    + b"\x00" * 4       # Address
    + struct.pack("<H", 2)  # Data length
    + b"\x00" * 2       # IRQ
    + b"\x00" * 2       # Data
    + b"\x00" * 2       # Working counter
)

# EtherCAT header: type 1 + datagram length
ethercat_header = struct.pack(
    "<H", 0x1000 | len(datagram)
)

frame = ethernet_header + ethercat_header + datagram

# Minimum Ethernet frame size without FCS
frame = frame.ljust(60, b"\x00")

# Transmit directly at Ethernet Layer 2
with socket.socket(
    socket.AF_PACKET,
    socket.SOCK_RAW,
    socket.htons(ETHERTYPE)
) as sock:
    sent = sock.sendto(frame, (INTERFACE, ETHERTYPE))

print(f"Interface: {INTERFACE}")
print(f"EtherType: 0x{ETHERTYPE:04X}")
print(f"Frame size: {len(frame)} bytes")
print(f"Bytes sent: {sent}")
print("Frame transmission requested successfully")
