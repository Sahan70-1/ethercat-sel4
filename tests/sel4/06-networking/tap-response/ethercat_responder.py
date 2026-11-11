#!/usr/bin/env python3
import socket

INTERFACE = "ecat-tap0"
ETH_P_ALL = 0x0003
RESPONDER_MAC = bytes.fromhex("02 ec a7 00 00 01")

sock = socket.socket(
    socket.AF_PACKET,
    socket.SOCK_RAW,
    socket.htons(ETH_P_ALL)
)

sock.bind((INTERFACE, 0))

print(f"ECAT_RESPONDER: listening on {INTERFACE}", flush=True)

while True:
    frame = sock.recv(2048)

    if len(frame) < 30:
        continue

    if frame[12:14] != b"\x88\xa4":
        continue

    # Only handle our expected EtherCAT BRD request.
    if frame[14:16] != b"\x0e\x10":
        continue

    if frame[16:18] != b"\x07\x01":
        continue

    if frame[18:24] != b"\x00\x00\x30\x01\x02\x00":
        continue

    if frame[28:30] != b"\x00\x00":
        continue

    guest_mac = frame[6:12]

    print("ECAT_RESPONDER: BRD request received", flush=True)
    print("ECAT_RESPONDER: guest MAC =", guest_mac.hex(":"), flush=True)

    response = bytearray(frame)

    # Ethernet destination: the seL4 client.
    response[0:6] = guest_mac

    # Ethernet source: simulated endpoint.
    response[6:12] = RESPONDER_MAC

    # AL Status = 0x0002 (PRE-OPERATIONAL).
    response[26:28] = b"\x02\x00"

    # Simulated working counter = 1.
    response[28:30] = b"\x01\x00"

    sock.sendall(response)

    print("ECAT_RESPONDER: simulated response sent", flush=True)
    print("ECAT_RESPONDER: AL Status = 0x0002", flush=True)
    print("ECAT_RESPONDER: WKC = 1", flush=True)
    break

sock.close()
