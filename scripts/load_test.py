#!/usr/bin/env python3
"""Load test for the R-Type server: how many concurrent games does one server hold?

    ./scripts/load_test.py <port> <games> [seconds]

Each simulated player connects, creates its own room and starts a game, so `games` rooms run
concurrently. Nothing is measured inside the server: the simulation rate is derived from the
`tick` field carried by every snapshot, which is what a real client observes.

Reported per run:
  - rooms actually started;
  - simulation rate seen by the clients, which must stay at the configured tick rate;
  - snapshots received per second and per client, expected at tickRate / snapshotEveryTicks;
  - bytes per second received by one client, and for the whole set.

A server that cannot keep up shows a simulation rate below the configured one.
"""
import socket
import struct
import sys
import time

MAGIC = 0x454E
VERSION = 9
HEADER = ">HBBIIIHH"
HEADER_SIZE = 20
CONNECT, INPUT, HEARTBEAT, CREATE_ROOM, START_GAME = 1, 2, 3, 6, 9
SNAPSHOT, SNAPSHOT_DELTA, ACCEPT, ROOM_JOINED = 0x83, 0x92, 0x81, 0x8B
INPUT_PERIOD = 0.1
HEARTBEAT_PERIOD = 1.0


class Player:
    def __init__(self, port, index):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setblocking(False)
        self.server = ("127.0.0.1", port)
        self.index = index
        self.sequence = 1
        self.token = 0
        self.room = None
        self.latest = None
        self.history = 0
        self.last_input = 0.0
        self.last_beat = 0.0
        self.first_tick = None
        self.last_tick = None
        self.first_seen = None
        self.last_seen = None
        self.snapshots = 0
        self.bytes = 0

    def send(self, kind, payload=b""):
        ack = self.latest if self.latest is not None else 0
        header = struct.pack(HEADER, MAGIC, VERSION, kind, self.sequence, ack, self.history, 0, len(payload))
        self.sock.sendto(header + payload, self.server)
        self.sequence += 1

    def pump(self):
        while True:
            try:
                data = self.sock.recv(2048)
            except BlockingIOError:
                return
            magic, _, kind, sequence, _, _, _, size = struct.unpack(HEADER, data[:HEADER_SIZE])
            if magic != MAGIC or size != len(data) - HEADER_SIZE:
                continue
            self.bytes += len(data)
            if self.latest is None or ((sequence - self.latest) & 0xFFFFFFFF) < 0x80000000:
                self.latest = sequence
            if kind == ACCEPT:
                self.token = struct.unpack(">I", data[HEADER_SIZE:HEADER_SIZE + 4])[0]
            elif kind == ROOM_JOINED:
                self.room = struct.unpack(">H", data[HEADER_SIZE:HEADER_SIZE + 2])[0]
            elif kind in (SNAPSHOT, SNAPSHOT_DELTA):
                tick = struct.unpack(">I", data[HEADER_SIZE:HEADER_SIZE + 4])[0]
                now = time.time()
                if self.first_tick is None:
                    self.first_tick, self.first_seen = tick, now
                self.last_tick, self.last_seen = tick, now
                self.snapshots += 1

    def play(self, now):
        if not self.token:
            return
        if now - self.last_beat >= HEARTBEAT_PERIOD:
            self.send(HEARTBEAT, struct.pack(">II", self.token, int(now * 1000) & 0xFFFFFFFF))
            self.last_beat = now
        if self.room is not None and now - self.last_input >= INPUT_PERIOD:
            self.send(INPUT, struct.pack(">IB", self.token, 0x10))
            self.last_input = now
        self.pump()

    def rates(self):
        if self.first_seen is None or self.last_seen is None or self.last_seen <= self.first_seen:
            return None
        elapsed = self.last_seen - self.first_seen
        return ((self.last_tick - self.first_tick) / elapsed, self.snapshots / elapsed, self.bytes / elapsed)


def step(players, seconds):
    end = time.time() + seconds
    while time.time() < end:
        now = time.time()
        for player in players:
            player.play(now)
        time.sleep(0.005)


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    port = int(sys.argv[1])
    games = int(sys.argv[2])
    seconds = float(sys.argv[3]) if len(sys.argv) > 3 else 20.0
    players = [Player(port, i) for i in range(games)]

    for _ in range(80):
        if all(p.token for p in players):
            break
        for player in players:
            if not player.token:
                player.send(CONNECT, b"\x00")
            player.pump()
        time.sleep(0.05)
    connected = sum(1 for p in players if p.token)

    for _ in range(60):
        if all(p.room is not None for p in players if p.token):
            break
        for player in players:
            if player.token and player.room is None:
                name = f"load{player.index:03d}"
                player.send(CREATE_ROOM,
                            struct.pack(">I", player.token) + bytes([len(name)]) + name.encode()
                            + b"\x00" + bytes([2, 0, 0]))
            player.pump()
        time.sleep(0.1)
    rooms = sum(1 for p in players if p.room is not None)

    for player in players:
        if player.room is not None:
            player.send(START_GAME, struct.pack(">I", player.token))
    step(players, 2.0)
    for player in players:
        player.first_tick = player.last_tick = player.first_seen = player.last_seen = None
        player.snapshots = player.bytes = 0

    print(f"{connected}/{games} connected, {rooms} rooms started, measuring for {seconds:.0f} s...")
    step(players, seconds)

    rates = [p.rates() for p in players]
    rates = [r for r in rates if r is not None]
    if not rates:
        print("no snapshot received")
        return 1
    ticks = sorted(r[0] for r in rates)
    snaps = [r[1] for r in rates]
    data = [r[2] for r in rates]
    print(f"\nrooms running ................ {rooms}")
    print(f"simulation rate seen by clients  min {ticks[0]:.1f} | median {ticks[len(ticks) // 2]:.1f} "
          f"| max {ticks[-1]:.1f} ticks/s")
    print(f"snapshots per client ......... {sum(snaps) / len(snaps):.1f} /s")
    print(f"bytes per client ............. {sum(data) / len(data) / 1024:.1f} KiB/s")
    print(f"bytes for all clients ........ {sum(data) / 1024:.1f} KiB/s")
    return 0


sys.exit(main())
