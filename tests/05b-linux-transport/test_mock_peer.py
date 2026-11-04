#!/usr/bin/env python3
"""Test response filtering and the actual C probe using local socketpairs."""
import socket
import subprocess
import sys
import unittest

from mock_veth_peer import make_reply

PROBE = sys.argv.pop(1)


class MockPeerTests(unittest.TestCase):
    def request(self):
        return bytes.fromhex("ffffffffffff01010101010188a40e100701000030010200000000000000")

    def test_reply_fields_and_padding(self):
        for frame in (self.request(), self.request() + bytes(30)):
            reply = make_reply(frame)
            self.assertEqual(reply[:26], frame[:26])
            self.assertEqual(reply[26:30], bytes.fromhex("02000200"))
            self.assertEqual(reply[30:], frame[30:])
            self.assertIsNone(make_reply(reply))  # never echo a reply

    def test_rejects_unrelated_or_malformed(self):
        frame = self.request()
        for length in range(30):
            self.assertIsNone(make_reply(frame[:length]))
        self.assertIsNone(make_reply(frame + bytes(1519)))
        for offset in (0, 6, 12, 14, 16, 18, 20, 22, 24, 26, 28):
            changed = bytearray(frame)
            changed[offset] ^= 1
            self.assertIsNone(make_reply(changed), f"offset {offset}")

    def run_probe(self, mutate=None):
        client, peer = socket.socketpair(socket.AF_UNIX, socket.SOCK_DGRAM)
        with client, peer:
            peer.settimeout(2)
            process = subprocess.Popen([PROBE, "--test-fd", str(client.fileno())],
                                       pass_fds=(client.fileno(),), stdout=subprocess.PIPE,
                                       stderr=subprocess.STDOUT, text=True)
            try:
                frame = peer.recv(2048)
                reply = make_reply(frame)
                self.assertIsNotNone(reply)
                if mutate is not None:
                    reply = mutate(reply)
                peer.send(reply)
                output, _ = process.communicate(timeout=3)
                return process.returncode, output
            finally:
                if process.poll() is None:
                    process.kill()
                    process.communicate()

    def test_actual_probe_accepts_matching_reply(self):
        code, output = self.run_probe()
        self.assertEqual(code, 0, output)
        self.assertIn("PASS: matched response", output)

    def test_actual_probe_rejects_wrong_payload(self):
        def corrupt(reply):
            frame = bytearray(reply)
            frame[26] = 3
            return bytes(frame)
        code, output = self.run_probe(corrupt)
        self.assertEqual(code, 1, output)
        self.assertIn("FAIL:", output)

    def test_actual_probe_rejects_wrong_index(self):
        def corrupt(reply):
            frame = bytearray(reply)
            frame[17] ^= 1
            return bytes(frame)
        code, output = self.run_probe(corrupt)
        self.assertEqual(code, 1, output)
        self.assertIn("WKC=-1", output)


if __name__ == "__main__":
    unittest.main(verbosity=2)
