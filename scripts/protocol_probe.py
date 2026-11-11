#!/usr/bin/env python3
"""Headless R-Type client used to test the network protocol.

It speaks the binary UDP protocol directly, without the graphical client, so that the
reliability and bandwidth work can be measured and verified from a terminal.

    ./scripts/protocol_probe.py <port> [loss]

    port   UDP port of a running r-type_server
    loss   fraction of received packets to deliberately ignore, 0.0 to 1.0 (default 0.0)

An ignored packet is neither processed nor acknowledged: from the server's point of view it is
indistinguishable from a real network loss. Only client A applies the loss; client B stays clean
and acts as the reference.

The probe connects two clients, creates a room, starts a game, plays for a few seconds, then
reports:
  - the size of every datagram received, per message type;
  - whether the reliable message identifiers form an unbroken sequence, which proves that no
    reliable message was lost;
  - how many of the ignored packets carried a reliable message.

See docs/reference/protocol.md for the wire format.
"""
import random
import socket
import struct
import sys
import time

MAGIC = 0x454E
VERSION = 9
HEADER = ">HBBIIIHH"
HEADER_SIZE = 20
ACK_WINDOW = 32
HEARTBEAT_PERIOD = 0.1
PLAY_SECONDS = 12.0
DRAIN_SECONDS = 3.0

CONNECT, INPUT, HEARTBEAT, CREATE_ROOM, JOIN_ROOM, START_GAME, CHAT = 1, 2, 3, 6, 7, 9, 0x0C
BUTTON_FIRE = 0x10
BUTTON_UP = 0x01

NAMES = {
    0x81: "ACCEPT", 0x82: "REJECT", 0x83: "SNAPSHOT", 0x84: "HEARTBEAT_ACK",
    0x85: "ENTITY_DESTROYED", 0x86: "PLAYER_JOINED", 0x87: "PLAYER_KILLED", 0x88: "PLAYER_LEFT",
    0x89: "SERVER_SHUTDOWN", 0x8A: "ROOM_LIST", 0x8B: "ROOM_JOINED", 0x8C: "ROOM_REJECTED",
    0x8D: "ROOM_UPDATE", 0x8E: "GAME_STARTED", 0x8F: "KICKED", 0x90: "ROOM_CLOSED",
    0x91: "PLAY_SOUND", 0x92: "SNAPSHOT_DELTA", 0x93: "CHAT_MESSAGE",
}
QUIET = {0x83, 0x84, 0x8D, 0x92}


def text(value):
    return bytes([len(value)]) + value.encode()


def newer(candidate, previous):
    distance = (candidate - previous) & 0xFFFFFFFF
    return distance != 0 and distance < 0x80000000


class Probe:
    """One headless client: sequencing, acknowledgements, and bookkeeping."""

    def __init__(self, label, host, port, loss=0.0):
        self.label = label
        self.server = (host, port)
        self.loss = loss
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setblocking(False)
        self.sequence = 1
        self.token = 0
        self.slot = None
        self.room = None
        self.latest = None
        self.history = 0
        self.last_beat = 0.0
        self.received = []
        self.sizes = {}
        self.reliable = {}
        self.duplicates = 0
        self.dropped = 0
        self.dropped_reliable = []
        self.chat = []

    def send(self, kind, payload=b""):
        ack = self.latest if self.latest is not None else 0
        header = struct.pack(HEADER, MAGIC, VERSION, kind, self.sequence, ack, self.history, 0, len(payload))
        self.sock.sendto(header + payload, self.server)
        self.sequence += 1

    def beat(self):
        now = time.time()
        if self.token and now - self.last_beat >= HEARTBEAT_PERIOD:
            self.send(HEARTBEAT, struct.pack(">II", self.token, int(now * 1000) & 0xFFFFFFFF))
            self.last_beat = now

    def acknowledge(self, sequence):
        """Mirror of ReliableReceiver::receive: slide the window, or fill a late hole."""
        if self.latest is None:
            self.latest, self.history = sequence, 0
            return
        if newer(sequence, self.latest):
            shift = (sequence - self.latest) & 0xFFFFFFFF
            if shift > ACK_WINDOW:
                self.history = 0
            elif shift == ACK_WINDOW:
                self.history = 1 << (ACK_WINDOW - 1)
            else:
                self.history = ((self.history << shift) | (1 << (shift - 1))) & 0xFFFFFFFF
            self.latest = sequence
            return
        distance = (self.latest - sequence) & 0xFFFFFFFF
        if 1 <= distance <= ACK_WINDOW:
            self.history |= 1 << (distance - 1)

    def pump(self):
        while True:
            try:
                data = self.sock.recv(2048)
            except BlockingIOError:
                return
            magic, _, kind, sequence, _, _, reliable_id, size = struct.unpack(HEADER, data[:HEADER_SIZE])
            name = NAMES.get(kind, hex(kind))
            if magic != MAGIC or size != len(data) - HEADER_SIZE:
                print(f"   ! [{self.label}] malformed datagram ignored")
                continue
            if random.random() < self.loss:
                self.dropped += 1
                if reliable_id != 0:
                    self.dropped_reliable.append((reliable_id, name))
                continue
            self.acknowledge(sequence)
            self.received.append((sequence, kind))
            self.sizes.setdefault(name, []).append(len(data))
            if reliable_id != 0:
                if reliable_id in self.reliable:
                    self.duplicates += 1
                    continue
                self.reliable[reliable_id] = name
            if kind not in QUIET:
                print(f"   . [{self.label}] <- {name} seq={sequence} rid={reliable_id}")
            if kind == 0x81:
                self.token, player = struct.unpack(">IB", data[HEADER_SIZE:HEADER_SIZE + 5])
                print(f"     [{self.label}] token={self.token} player={player}")
            elif kind == 0x93:
                length = data[HEADER_SIZE + 1]
                self.chat.append(data[HEADER_SIZE + 2:HEADER_SIZE + 2 + length].decode("ascii", "replace"))
            elif kind == 0x8B:
                self.room = struct.unpack(">H", data[HEADER_SIZE:HEADER_SIZE + 2])[0]
                self.slot = data[-1]
                print(f"     [{self.label}] room={self.room} slot={self.slot}")

    def reliable_types(self):
        counts = {}
        for reliable_id in sorted(self.reliable):
            name = self.reliable[reliable_id]
            counts[name] = counts.get(name, 0) + 1
        return counts

    def report(self):
        kinds = {}
        for _, kind in self.received:
            name = NAMES.get(kind, hex(kind))
            kinds[name] = kinds.get(name, 0) + 1
        identifiers = sorted(self.reliable)
        unbroken = identifiers == list(range(1, len(identifiers) + 1))
        print(f"\n[{self.label}] {len(self.received)} packets processed, {self.dropped} deliberately ignored")
        print(f"[{self.label}] messages: {kinds}")
        print(f"[{self.label}] datagram sizes (bytes, 20-byte header included):")
        total = 0
        for name in sorted(self.sizes, key=lambda n: -sum(self.sizes[n])):
            values = self.sizes[name]
            total += sum(values)
            print(f"       {name:<18} n={len(values):<4} min={min(values):<5} "
                  f"avg={sum(values) // len(values):<5} max={max(values)}")
        print(f"       total received: {total} bytes")
        print(f"[{self.label}] reliable packets deliberately lost: {len(self.dropped_reliable)} "
              f"-> {self.dropped_reliable}")
        print(f"[{self.label}] duplicates discarded: {self.duplicates}")
        if not unbroken:
            print(f"[{self.label}] GAP in reliable identifiers: {identifiers}")
        return unbroken


def wait(probes, seconds):
    end = time.time() + seconds
    while time.time() < end:
        for probe in probes:
            probe.beat()
            probe.pump()
        time.sleep(0.01)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    port = int(sys.argv[1])
    loss = float(sys.argv[2]) if len(sys.argv) > 2 else 0.0
    print(f"Simulated receive loss on client A: {loss * 100:.0f} %\n")

    a = Probe("A", "127.0.0.1", port, loss)
    b = Probe("B", "127.0.0.1", port, 0.0)
    both = [a, b]

    print("1) connecting both clients")
    for _ in range(60):
        if a.token and b.token:
            break
        for probe in both:
            if not probe.token:
                probe.send(CONNECT, text(probe.label))
        wait(both, 0.1)
    if not (a.token and b.token):
        print("connection failed")
        return 1

    print("2) A creates a room")
    for _ in range(20):
        if a.room is not None:
            break
        a.send(CREATE_ROOM, struct.pack(">I", a.token) + text("probe") + text("") + bytes([2, 0, 0]))
        wait(both, 0.2)
    if a.room is None:
        print("room creation failed")
        return 1

    print("3) B joins the room")
    for _ in range(20):
        if b.room is not None:
            break
        b.send(JOIN_ROOM, struct.pack(">IH", b.token, a.room) + text(""))
        wait(both, 0.2)

    print("4) A starts the game")
    a.send(START_GAME, struct.pack(">I", a.token))
    wait(both, 0.5)

    print("5) A and B exchange 6 chat messages while A loses packets")
    for index in range(3):
        a.send(CHAT, struct.pack(">I", a.token) + text(f"HELLO FROM A {index}"))
        wait(both, 0.6)
        b.send(CHAT, struct.pack(">I", b.token) + text(f"HELLO FROM B {index}"))
        wait(both, 0.6)

    print(f"6) {PLAY_SECONDS:.0f} seconds of play, inputs at 30 Hz")
    end = time.time() + PLAY_SECONDS
    while time.time() < end:
        a.send(INPUT, struct.pack(">IB", a.token, BUTTON_FIRE))
        b.send(INPUT, struct.pack(">IB", b.token, BUTTON_FIRE | BUTTON_UP))
        for probe in both:
            probe.beat()
            probe.pump()
        time.sleep(0.033)

    print(f"7) loss stops, {DRAIN_SECONDS:.0f} seconds to drain retransmissions")
    a.loss = 0.0
    wait(both, DRAIN_SECONDS)

    unbroken_a, unbroken_b = a.report(), b.report()
    events_a = a.reliable_types().get("ENTITY_DESTROYED", 0)
    events_b = b.reliable_types().get("ENTITY_DESTROYED", 0)
    print("\n=== CHAT ===")
    print(f"A received {len(a.chat)}/6 : {a.chat}")
    print(f"B received {len(b.chat)}/6 : {b.chat}")
    chat_ok = len(a.chat) == 6 and len(b.chat) == 6
    print("\n=== VERDICT ===")
    print(f"chat messages delivered ............ {'6/6 both' if chat_ok else 'MISSING'}")
    print(f"A: reliable identifiers unbroken ... {'yes' if unbroken_a else 'NO'}")
    print(f"B: reliable identifiers unbroken ... {'yes' if unbroken_b else 'NO'}")
    print(f"ENTITY_DESTROYED  A={events_a}  B={events_b} (B is the lossless reference)")
    if unbroken_a and unbroken_b and events_a == events_b and chat_ok:
        print(f"PASS: no reliable message lost. {a.dropped} packets ignored, "
              f"{len(a.dropped_reliable)} of them carrying a reliable message.")
        return 0
    print("FAIL: reliable messages are missing on A.")
    return 1


sys.exit(main())
