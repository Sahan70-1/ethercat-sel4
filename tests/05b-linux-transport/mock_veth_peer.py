#!/usr/bin/env python3
"""Return one explicitly synthetic reply to veth_probe; NOT a slave emulator."""
import argparse
import socket
import time


def make_reply(frame):
    # Accept only the probe's single BRD, two-byte read of address 0x0130.
    # Index stays unchanged. Reject unrelated discovery traffic and replies.
    if not 30 <= len(frame) <= 1518:
        return None
    if frame[:14] != b"\xff" * 6 + b"\x01" * 6 + b"\x88\xa4":
        return None
    if frame[14:17] != b"\x0e\x10\x07":
        return None
    if frame[18:30] != b"\x00\x00\x30\x01\x02\x00\x00\x00\x00\x00\x00\x00":
        return None
    reply = bytearray(frame)
    reply[26:30] = b"\x02\x00\x02\x00"  # synthetic data and WKC
    return bytes(reply)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("interface", help="isolated veth peer, normally ecat_rx")
    args = parser.parse_args()
    with socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(0x88A4)) as peer:
        peer.bind((args.interface, 0))
        deadline = time.monotonic() + 120
        print(f"MOCK READY on {args.interface}; waiting up to 120 seconds", flush=True)
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise SystemExit("FAIL: no matching probe request before timeout")
            peer.settimeout(remaining)
            try:
                frame, address = peer.recvfrom(65535)
            except TimeoutError:
                raise SystemExit("FAIL: no matching probe request before timeout") from None
            if address[2] == socket.PACKET_OUTGOING:
                continue
            reply = make_reply(frame)
            if reply is None:
                continue
            if peer.send(reply) != len(reply):
                raise SystemExit("FAIL: incomplete mock reply send")
            print(f"MOCK SENT: index={reply[17]}, bytes={len(reply)}, synthetic WKC=2", flush=True)
            print("No real slave is present or emulated.", flush=True)
            return


if __name__ == "__main__":
    main()
