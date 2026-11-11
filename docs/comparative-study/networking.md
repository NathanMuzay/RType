# Networking

Every choice below is followed by the measurement that justifies it. Figures come from
`scripts/protocol_probe.py` and `scripts/load_test.py`, both in this repository.

## Transport

| Option | Strengths | Weaknesses |
|---|---|---|
| **UDP** | No head-of-line blocking, low latency, datagram boundaries preserved | No delivery, ordering or duplicate protection |
| TCP | Reliable, ordered | One lost segment delays everything behind it; unsuitable for real-time state |
| UDP + a TCP side channel | Reliability without writing it | Two connections, two failure modes, two timeout policies |

**Choice: UDP only.** Beyond the subject's requirement, the technical reason is that reliability
is not a property we want uniformly. A lost position update must *not* be retransmitted: a
fresher one arrives 50 ms later, and re-delivering stale state is exactly the TCP behaviour that
causes visible stutter. A lost "enemy destroyed" event, on the other hand, is lost forever.

TCP cannot express that distinction; it reliably delivers everything, including what has already
expired. Our acknowledgement layer applies reliability message by message.

## Reliability over UDP

| Option | Cost | Weaknesses |
|---|---|---|
| TCP side channel | No code | Head-of-line blocking returns for the reliable stream; second connection to manage |
| Message duplication (send each event 3 times) | Trivial | Wastes bandwidth permanently; still fails on a burst |
| **Per-packet acknowledgement with a 32-bit bitfield** | ~250 lines | Must be designed carefully |

**Choice: acknowledgements carried in the packet header.** Each packet reports the most recent
sequence received from the peer plus a 32-bit history of the 32 before it, so one packet
acknowledges up to 33 and losing an acknowledgement is self-healing.

| Metric | Value |
|---|---|
| Header overhead | 10 bytes per packet (`ack`, `ackBits`, `reliableId`), on a 20-byte header |
| Share of a real session's traffic | **7 %** measured, dominated by small packets |
| Dedicated acknowledgement packets | **0** — they travel on inputs and snapshots |
| Game events lost at 60 % packet loss | **0** |

The 7 % figure is measured, not computed from the worst case: most packets are small, so a fixed
10 extra header bytes weigh more than a theoretical calculation on full snapshots suggests.

**Retransmission delay: `max(100 ms, rtt x 1.5)`.** The floor matters more than the formula. With
a 30 ms floor we measured **4 useless retransmissions at 0 % loss**: on a LAN the round-trip time
is about 1 ms, but a peer only acknowledges when it sends something, which may be 33 ms later.
The delay must exceed the peer's sending cadence, not just the round-trip time.

## State synchronisation

| Option | Loss tolerance | Bandwidth |
|---|---|---|
| Events only ("spawned", "moved") | A lost event desynchronises forever | Lowest |
| Complete snapshots | Each snapshot is self-sufficient | Highest |
| **Delta snapshots against an acknowledged baseline** | Self-sufficient keyframes every 5 s | Lowest in practice |

**Choice: deltas, with full snapshots as keyframes.** The design rule is that a delta is never
built against a snapshot that has merely been *sent*: only against one the client has
*acknowledged*. Otherwise a single loss corrupts every later delta, silently and permanently.

What the measurement changed in the design: we expected the gain to come from skipping entities
that had not moved. Measured over 1558 entity transitions:

| Observation | Measured |
|---|---|
| Entities whose position changed between two snapshots | **86 %** |
| Entities whose `kind` changed | **0 %** |
| Entities whose `visual` changed | **0.3 %** |
| Movements fitting in one signed byte | **100 %** |
| Player slots changed between two snapshots | **3 %** |
| World values changed between two snapshots | **7 %** |

In a horizontal shooter almost everything moves every frame, so skipping unchanged entities is
worth only 14 %. The gain is elsewhere: unchanged player slots and world values, and a compact
encoding of what did change.

| Encoding | Average snapshot | Ratio |
|---|---|---|
| Full snapshot | 223 bytes | baseline |
| Index lists for world values and player slots | 130 bytes | x1.7 |
| Plus compact entity changes | **105 bytes** | **x2.1** |

**Index lists rather than bit masks.** Masks were the first design. They were rejected because
the decoder is static and has no access to the game configuration: it cannot know how many bits
to read. An 8-bit mask would have capped the engine at 8 players and 8 world values; a 32-bit
mask would have cost 6 bytes more per packet. An explicit list of changed indices is
self-describing, imposes no limit, and is the same size in practice at the measured change rates.

## Data packing

| Technique | Applied | Where |
|---|---|---|
| Quantisation | Positions rounded to the pixel; rotation stored as 128 steps over 360 degrees | `Simulation::visualOf` |
| Sub-byte packing | `visual` = 1 flash bit + 7 angle bits; `type` = 1 direction bit + 7 identifier bits | `Protocol`, `Codec` |
| Adequate type sizes | Position deltas as `int8` instead of `int16` | `EntityChange` |
| Space-efficient serialisation | A field mask: only changed fields are written | `EntityChange` |
| No padding | Every field written individually, never a struct copy | `ByteWriter` |

The rotation is the clearest case: a `float` would cost 4 bytes, it costs 7 bits, a factor of 36
for a precision of 2.8 degrees, which is beyond what a sprite's tilt requires.

**Not applied: packing across byte boundaries.** An 11-bit identifier followed by a 6-bit kind
would need a bit-level cursor. We left it out deliberately: `kind` is no longer transmitted at
all once deltas are in place, and shrinking the identifier would require bounding network
identifiers, which the simulation does not do. Estimated remaining gain: about one byte per
entity.

## General-purpose compression

| Algorithm | Speed | Ratio on our payloads |
|---|---|---|
| RLE | Very fast | Poor: few long identical runs after delta encoding |
| Huffman | Moderate | Good, but costly to implement and to tune |
| LZ4 | Very fast | Moderate |
| zlib | Slow | Best, but unsuitable at 20 packets per second per client |

**Choice: none, for now, and the reason is a measurement.** Generic compression and delta
encoding exploit the same redundancy. Once the delta removes the repeated player slots, the
unchanged world values and the constant entity fields, a 105-byte payload has little left to
compress. The honest ordering is delta first, compression second, and only if the measured gain
justifies the dependency.

## Authority model

| Model | Consistency | Cheating | Responsiveness |
|---|---|---|---|
| **Server authoritative, client displays** (ours) | One truth | Prevented | One round trip for own movement |
| Server authoritative + client prediction | One truth | Prevented | Immediate, needs reconciliation |
| Peer-to-peer lockstep | All peers equal | Hard to prevent | Slowest peer blocks everyone |
| Host client | Host is the truth | Host can cheat | Host has no latency |

**Choice: server authoritative.** Client-side prediction with reconciliation is the documented
next step; the prerequisite is a sequence number on `INPUT`, which the protocol does not carry
yet.

## Input encoding

| Option | Effect of a lost packet |
|---|---|
| Key events ("pressed", "released") | A lost "released" leaves a key held down forever |
| **Held-button state, repeated ~30/s** | Corrected 33 ms later; released after 500 ms of silence |

**Choice: state, not events.** This is reliability obtained for free, by the nature of the data
rather than by a mechanism.

## Running several games on one server

| Option | Gain | Cost |
|---|---|---|
| **One simulation thread, isolated sessions per room** (ours) | — | — |
| Thread pool over rooms | Measured below | Thread pool, GIL handling, shared-state audit |
| One process per room | True isolation | Process management, memory, inter-process routing |

**Choice: one simulation thread, after measurement.** Per room and per tick, on an Apple M5:

| Stage | Time |
|---|---|
| C++ simulation (physics, collisions, ECS) | 8 µs |
| **Python scripts** | **24 µs** (73 %) |
| Network encoding | 5 µs |
| **Total** | **37 µs**, against a 16 667 µs tick budget |

Game logic is scripted in Python, whose global interpreter lock serialises bytecode execution
across threads. On four cores the best case is `16x24 + 16x13/4 = 436 µs` against 592 µs, a gain
of **0.9 % of a tick**, in exchange for a thread pool, correct GIL acquisition in every worker,
and an audit of every shared structure.

Load measured instead, reading the simulation rate from the `tick` field of snapshots, with no
instrumentation inside the server:

| Concurrent games | 8 | 16 | 32 | 48 | 96 |
|---|---|---|---|---|---|
| Simulation rate seen by clients | 60.0 | 60.0 | 60.0 | 60.0 | **60.0** |

No degradation anywhere, on a single simulation thread. Extrapolating from 37 µs, one thread
saturates at roughly 450 light games.

The real limit was not the processor. `maxRooms` is capped at **53** by the `ROOM_LIST` message
having to fit in one 1200-byte datagram; beyond that the room list would need pagination. The
configuration now allows 48 rooms and 192 clients.

If the game logic ever moved from Python to C++, the lock would disappear and this trade-off
would be worth revisiting.

## Chat reliability

Chat is the clearest demonstration of selective reliability. A lost position update is
irrelevant; a lost chat line is a visible bug. Both chat messages therefore travel on the
acknowledgement channel built for game events, at no extra cost.

| Scenario | Result |
|---|---|
| 6 messages exchanged at 40 % packet loss | **6/6 delivered to both clients** |
| 10 messages sent in one burst | **1 broadcast**, the rest discarded by the 500 ms rate limit |
| 3 messages spaced by 600 ms | **3 broadcast** |

The rate limit is not cosmetic: without it, any client could make the server broadcast an
unbounded number of reliable messages to every member of its room, which is an amplification
vector.

## Rates

| Value | Reason |
|---|---|
| 60 ticks/s | Smooth simulation, matches display rate |
| 20 snapshots/s | One every 3 ticks; balance between smoothness and bandwidth |
| ~30 inputs/s | Below one frame of delay at 60 Hz |
| 1 heartbeat/s, 5 s timeout | Detects crashed peers quickly without traffic overhead |
| Keyframe every 5 s | Bounds the lifetime of any undetected desynchronisation |

## Measurement method

Two points cost us time and are worth recording.

**Counting duplicates does not measure retransmissions.** When the original is lost, the
retransmission replaces it and never appears as a duplicate. The meaningful indicator is the
number of deliberately dropped packets that carried a reliable message, compared against final
completeness.

**A failing test is suspect before the code is.** Comparing exact message counts between two
clients reported a failure at 40 % loss: one client received two `ROOM_JOINED` instead of one.
The cause was the test harness retrying `CREATE_ROOM`, and the server legitimately answering
twice. The criterion became the continuity of reliable identifiers.

## Not implemented

| Technique | Status |
|---|---|
| Client-side prediction and server reconciliation | Analysed, not implemented; needs an input sequence number |
| Entity interpolation | Analysed, not implemented; the client applies snapshot positions directly |
| Server-side lag compensation, rollback | Out of scope |
| Generic compression (LZ4) | Deliberately deferred, see above |
| Bit-level packing across byte boundaries | Deliberately deferred, see above |
| Measurements under `netem` or `clumsy` | Linux `netem`: [real server and two production clients](../measurements/netem-2026-11-08/report.md), with bidirectional delay, loss, duplication, reordering and a rate limit. Windows `clumsy`: not run |
