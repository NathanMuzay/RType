# Network protocol

Version **9** of the R-Type wire protocol. This document is the complete reference: a developer
should be able to write a new client, or a new server, from this page alone.

## 1. Overview

| Property | Value |
|---|---|
| Transport | UDP over IPv4; the client resolves a host name to an address before connecting |
| Default port | 4242 |
| Byte order | Big-endian for every multi-byte field |
| Header size | 20 bytes, identical on every packet |
| Maximum datagram | 1200 bytes, header included |
| Protocol version | 9 (`Protocol::VERSION`) |

One datagram carries exactly **one message**. There is no stream, no framing and no fragmentation:
a message that would not fit in 1200 bytes is never produced.

All in-game traffic uses UDP. No TCP connection is opened at any point. Delivery guarantees, where
they are required, are provided by the acknowledgement layer described in section 4.

### Layers

```
  application   CONNECT, INPUT, SNAPSHOT, ENTITY_DESTROYED, ...   section 6
  reliability   ack, ackBits, reliableId, retransmission          section 4
  framing       magic, version, type, sequence, payload size      section 3
  transport     UDP / IPv4
```

### Source map

| Responsibility | File |
|---|---|
| Message types, constants, reliability policy | `engine/network/include/Protocol.hpp` |
| Header encoding, decoding and validation | `engine/network/src/Codec.cpp` |
| Big-endian primitives | `engine/net/src/ByteWriter.cpp`, `engine/net/src/ByteReader.cpp` |
| Acknowledgement window | `engine/network/src/ReliableReceiver.cpp` |
| Retransmission and round-trip estimation | `engine/network/src/ReliableSender.cpp` |
| Snapshot delta encoding | `engine/network/src/SnapshotDiff.cpp` |
| Server sessions and rooms | `engine/network/src/GameServer.cpp` |
| Client connection | `engine/network/src/ClientConnection.cpp` |

## 2. Primitive types

| Notation | Size | Encoding |
|---|---|---|
| `u8` | 1 | unsigned |
| `u16` | 2 | unsigned, big-endian |
| `u32` | 4 | unsigned, big-endian |
| `i8` | 1 | signed, two's complement |
| `i16` | 2 | signed, two's complement, big-endian |
| `string` | 1 + n | `u8` length, then that many bytes; printable ASCII (0x20..0x7E) only; 255 bytes maximum |

Structures are written field by field. There is no compiler padding and no alignment requirement:
a receiver must never map a datagram onto a struct.

## 3. Packet header

Every datagram begins with these 20 bytes.

| Offset | Size | Field | Description |
|---|---|---|---|
| 0 | u16 | `magic` | Always `0x454E`, the ASCII letters `EN`. Any other value: drop the datagram. |
| 2 | u8 | `version` | Protocol version, `9`. |
| 3 | u8 | `type` | Message type. Bit 7 set means server → client. |
| 4 | u32 | `sequence` | Packet counter of the sender, starting at 1 and increased by 1 for every packet sent. |
| 8 | u32 | `ack` | Most recent `sequence` received from the peer, or `0` when nothing has been received yet. |
| 12 | u32 | `ackBits` | Receipt status of the 32 packets before `ack`. Bit `i` refers to packet `ack - 1 - i`. |
| 16 | u16 | `reliableId` | Identity of the message across its retransmissions, or `0` for an unreliable message. |
| 18 | u16 | `payloadSize` | Number of bytes after the header. |

### The direction bit

Bit 7 of `type` encodes the direction: `0x01`..`0x7F` are client → server, `0x81`..`0xFF` are
server → client. A receiver must reject any packet whose direction does not match its own role.
This prevents a client from injecting messages that only a server may produce.

### Sequence numbers

Each side keeps its own independent counter, and acknowledges the counter of the other side. The
two numbering spaces are never mixed.

Sequence numbers wrap around at 2^32. Comparison is modular: `candidate` is newer than `previous`
when `(candidate - previous) mod 2^32` lies in `1 .. 2^31 - 1`. A distance of exactly 2^31 is not
considered newer.

Because sequences start at **1**, the value `0` in the `ack` field unambiguously means "I have
received nothing from you yet".

## 4. Reliability

UDP may lose, duplicate, reorder or delay datagrams. This protocol does not retransmit everything:
it retransmits only what cannot be reconstructed from later traffic.

### Acknowledgements

Acknowledgements are never sent in a dedicated packet. They travel in the header of packets that
are sent anyway — inputs at 30 Hz from the client, snapshots at 20 Hz from the server.

A receiver maintains the highest `sequence` it has received and a 32-bit history of the packets
before it:

```
   ack     = 43
   ackBits = 0b...00001101
                     ││││
                     │││└─ bit 0 -> packet 42 : received
                     ││└── bit 1 -> packet 41 : missing
                     │└─── bit 2 -> packet 40 : received
                     └──── bit 3 -> packet 39 : received
```

One packet therefore acknowledges up to 33 packets. Losing an acknowledgement is harmless: the
next one repeats the same information.

A packet must be recorded in the acknowledgement window as soon as it is received, independently
of whether its payload is processed. Acknowledging and processing are two distinct operations.

### Reliable messages

A sender keeps a copy of every reliable message it sends, together with the `sequence` of the
packet that carried it. When that sequence is acknowledged, the copy is dropped. When it is not
acknowledged within the retransmission delay, the message is sent again — **in a new packet, with
a new `sequence`, and with the same `reliableId`**.

Sequence numbers must stay strictly increasing, so a retransmission is never a byte-for-byte copy
of the original packet. The `reliableId` is what lets the receiver recognise a message it has
already processed: it keeps the last 256 identifiers seen and silently drops duplicates.

Two deduplication levels coexist: `sequence` discards duplicate *packets*, `reliableId` discards
duplicate *messages*.

### Retransmission delay

The delay is `max(100 ms, smoothed round-trip time x 1.5)`. The round-trip time is measured from
acknowledgements — the sender knows when it sent each pending message — and smoothed with a
weighted average keeping 7/8 of the previous estimate.

The 100 ms floor matters: on a local network the round-trip time is about 1 ms, but a peer only
acknowledges when it sends something, which may be up to 33 ms later. A delay shorter than the
peer's sending cadence causes retransmissions of packets that were never lost.

A sender keeps at most 64 unacknowledged messages per peer. Beyond that, the oldest is dropped;
a peer that stops acknowledging altogether is removed by the session timeout.

### Which messages are reliable

| Reliable | Reason |
|---|---|
| `CREATE_ROOM`, `JOIN_ROOM`, `LEAVE_ROOM`, `START_GAME`, `KICK_PLAYER`, `DELETE_ROOM`, `CHAT` | A lost lobby action leaves the player stuck with no feedback |
| `ENTITY_DESTROYED`, `PLAYER_JOINED`, `PLAYER_KILLED`, `PLAYER_LEFT`, `PLAY_SOUND`, `GAME_STARTED`, `ROOM_JOINED`, `ROOM_REJECTED`, `ROOM_CLOSED`, `KICKED`, `CHAT_MESSAGE` | A lost event is lost forever; nothing later restates it |

| Unreliable | Reason |
|---|---|
| `SNAPSHOT`, `SNAPSHOT_DELTA`, `INPUT`, `HEARTBEAT` | Superseded by fresher data within 33 to 50 ms. Retransmitting stale state is the TCP behaviour this protocol avoids |
| `ROOM_LIST`, `ROOM_UPDATE` | Resent periodically by the server, therefore self-healing |
| `CONNECT`, `ACCEPT`, `REJECT` | Connection has its own retry logic (section 7) |
| `CONSOLE_COMMAND`, `CONSOLE_TEXT` | Developer console only; the user retypes a lost command |
| `DISCONNECT`, `SERVER_SHUTDOWN` | The sender stops immediately afterwards and would not be there to retransmit; the timeout covers the case |

## 5. Validation rules

A receiver must drop a datagram, without answering and without disconnecting, when any of the
following holds. None of these may crash the program, allocate unbounded memory, or disconnect a
third party.

| Condition | Checked against |
|---|---|
| Fewer than 20 bytes, or more than 1200 | datagram length |
| `magic` is not `0x454E` | header |
| Direction bit does not match the receiver's role | `type` |
| `type` is not a known message type | section 6 |
| `version` differs from 9, except for `CONNECT` and `REJECT` | header |
| `payloadSize` differs from the actual number of bytes after the header | header |
| The payload ends before a declared field, or leaves unread bytes | body |
| A declared count exceeds its configured maximum | body |
| A string contains a byte outside 0x20..0x7E, or exceeds its maximum length | body |
| Semantic checks of the message fail (see each message) | body |

`CONNECT` and `REJECT` are exempt from the version check so that a mismatched client can be told
why it is being refused, rather than being ignored.

Beyond the per-packet checks, a server must also apply:

- a token check: every message that carries a session token is ignored unless the token matches
  the session registered for the source address and port;
- a rate limit on `CONNECT`: at most 32 requests per second across all sources, and at most one
  `ACCEPT` per 200 ms per client;
- a rate limit on `CHAT`: one message per session every 500 ms, silently discarding the rest.

Display names are not authenticated. Two players may choose the same name, and a name is not
tied to any identity beyond the lifetime of the session.

## 6. Message catalogue

### Client to server

| Type | Name | Payload |
|---|---|---|
| `0x01` | `CONNECT` | `name:string`, the player's display name, may be empty |
| `0x02` | `INPUT` | `token:u32`, `buttons:u8` |
| `0x03` | `HEARTBEAT` | `token:u32`, `timestamp:u32` |
| `0x04` | `DISCONNECT` | `token:u32` |
| `0x05` | `LIST_ROOMS` | `token:u32` |
| `0x06` | `CREATE_ROOM` | `token:u32`, `name:string`, `code:string`, `ruleCount:u8`, `rules:u8[ruleCount]` |
| `0x07` | `JOIN_ROOM` | `token:u32`, `roomId:u16`, `code:string` |
| `0x08` | `LEAVE_ROOM` | `token:u32` |
| `0x09` | `START_GAME` | `token:u32` |
| `0x0A` | `KICK_PLAYER` | `token:u32`, `slot:u8` |
| `0x0B` | `DELETE_ROOM` | `token:u32` |
| `0x0C` | `CONSOLE_COMMAND` | `token:u32`, `text:string` |
| `0x0D` | `CHAT` | `token:u32`, `text:string` |

### Server to client

| Type | Name | Payload |
|---|---|---|
| `0x81` | `ACCEPT` | `token:u32`, `playerId:u8` |
| `0x82` | `REJECT` | `reason:u8` |
| `0x83` | `SNAPSHOT` | see 6.2 |
| `0x84` | `HEARTBEAT_ACK` | `timestamp:u32`, echoed unchanged |
| `0x85` | `ENTITY_DESTROYED` | `entityId:u16` |
| `0x86` | `PLAYER_JOINED` | `playerId:u8` |
| `0x87` | `PLAYER_KILLED` | `playerId:u8` |
| `0x88` | `PLAYER_LEFT` | `playerId:u8`, `reason:u8` |
| `0x89` | `SERVER_SHUTDOWN` | empty |
| `0x8A` | `ROOM_LIST` | `roomCount:u8`, then `roomCount` room entries (6.1) |
| `0x8B` | `ROOM_JOINED` | `roomId:u16`, `name:string`, `slot:u8` |
| `0x8C` | `ROOM_REJECTED` | `reason:u8` |
| `0x8D` | `ROOM_UPDATE` | `ownerSlot:u8`, `slotCount:u8`, then per slot `occupied:u8` (0 or 1) and `name:string` |
| `0x8E` | `GAME_STARTED` | empty |
| `0x8F` | `KICKED` | empty |
| `0x90` | `ROOM_CLOSED` | empty |
| `0x91` | `PLAY_SOUND` | `sound:u8` |
| `0x92` | `CONSOLE_TEXT` | `text:string` |
| `0x93` | `SNAPSHOT_DELTA` | see 6.3 |
| `0x94` | `CHAT_MESSAGE` | `slot:u8`, `text:string` |

### Constants

| Field | Values |
|---|---|
| `buttons` bits | `0x01` up, `0x02` down, `0x04` left, `0x08` right, `0x10` fire, `0x20` action, `0x40` bomb. Bit `0x80` is reserved and must be zero |
| `REJECT.reason` | `1` server full, `2` version mismatch |
| `PLAYER_LEFT.reason` | `1` quit, `2` timeout |
| `ROOM_REJECTED.reason` | `1` not found, `2` full, `3` wrong code, `4` room limit reached, `5` invalid name, `6` too many attempts, `7` not allowed |
| Player slot state | `0` empty, `1` alive, `2` dead |
| Game state | `0` waiting, `1` playing, `2` game over |

### 6.1 Room entry

Used inside `ROOM_LIST`.

| Field | Type | Notes |
|---|---|---|
| `id` | u16 | Room identifier, non-zero |
| `name` | string | |
| `players` | u8 | Occupied slots |
| `capacity` | u8 | Total slots |
| `flags` | u8 | `0x01` password protected, `0x02` game in progress |

### 6.2 `SNAPSHOT`

A complete, self-sufficient picture of the world.

```
tick          u32    simulation step
gameState     u8     0 waiting, 1 playing, 2 game over
valueCount    u8     then valueCount x u16, the world values
playerCount   u8     then playerCount player slots
entityCount   u8     then entityCount entities
```

Player slot, 10 bytes plus its values:

```
state      u8
lives      u8
health     u8     0..100
score      u32
avatarId   u16    entity id of this player's ship, 0 when not alive
valueCount u8     then valueCount x u16, the per-player values
```

Entity, 8 bytes:

```
id       u16    non-zero, unique within the snapshot
kind     u8     game-defined entity type
x        i16    world coordinates, in pixels
y        i16
visual   u8     bit 7: hit flash; bits 0..6: rotation, 128 steps over 360 degrees
```

Semantic rules: `valueCount` and `playerCount` must match the values configured for the game;
entity identifiers must be unique and non-zero; a slot in state `alive` must have a non-zero
`avatarId` that appears in the entity list, at least one life, and health at most 100; a slot in
state `empty` must have all fields zero.

The number and meaning of the world and player values are game-defined and are not part of the
protocol. The reference game uses five world values (`stage`, `phase`, `progress`, `bossHealth`,
`shake`) and three player values (`weapon`, `force`, `charge`).

### 6.3 `SNAPSHOT_DELTA`

The same world state, expressed as the difference from a previously acknowledged `SNAPSHOT`.

```
tick           u32
baselineTick   u32    tick of the snapshot this delta applies to, never 0
gameState      u8
worldCount     u8     then worldCount entries of { index:u8, value:u16 }
playerCount    u8     then playerCount entries of { slot:u8, player slot as in 6.2 }
entityCount    u8     then entityCount entity changes (below)
removedCount   u8     then removedCount x u16, identifiers that no longer exist
```

Anything not mentioned keeps the value it had in the baseline.

Entity change:

```
id      u16
fields  u8    which fields follow
              0x01 dx present        i8, add to the baseline x
              0x02 dy present        i8, add to the baseline y
              0x04 position present  i16 x then i16 y, absolute
              0x08 kind present      u8
              0x10 visual present    u8
```

`0x04` is mutually exclusive with `0x01` and `0x02`. Fields are written in the order `dx`, `dy`,
`position`, `kind`, `visual`. A change whose identifier is absent from the baseline describes a
new entity and must carry `0x04`; one that carries neither `0x04` nor a known baseline entity is
discarded.

Bits outside `0x1F` are reserved and make the message invalid.

### Applying a delta

1. Look up the snapshot whose tick equals `baselineTick`. If it is unknown, **discard the whole
   message** and wait for a full `SNAPSHOT`; never apply a delta to the wrong baseline.
2. Copy the baseline.
3. Overwrite the world values and player slots the delta lists.
4. Remove the entities in the removal list.
5. For each entity change, resolve it against the baseline entity of the same identifier and
   replace it; entities the delta does not mention are kept unchanged.
6. The result is a complete snapshot, equivalent to the `SNAPSHOT` the server would have sent.

A client must keep the last 32 snapshots it has reconstructed, indexed by tick, so that it can
serve as a baseline.

### When the server sends a full snapshot

| Condition | Rationale |
|---|---|
| No acknowledged snapshot yet | New client, or client that just entered a room |
| The acknowledged baseline is more than one second of ticks old | The server no longer knows what the client holds |
| The last full snapshot was sent more than five seconds ago | Periodic keyframe, bounding any undetected desynchronisation |

A server must never build a delta against a snapshot that has not been acknowledged. The property
that makes this safe is inductive: a delta is always built on the confirmed baseline, so a client
that acknowledges the packet carrying it necessarily held that baseline and could apply it.

## 7. Session lifecycle

### Connecting

```
client                                   server
  | ---- CONNECT ----------------------> |  creates a session, allocates a token
  | <--- ACCEPT (token, playerId) ------ |
  |                                      |
  | ---- CONNECT (lost) ---------------X |
  |      retry after 1000 ms, 5 times    |
```

`CONNECT` carries no token, only the player's display name, which the server stores on the
session and republishes to the other members of a room through `ROOM_UPDATE`. An empty name is
accepted; clients then display a fallback such as `P1`. The server answers `ACCEPT` with a random
32-bit token that the client must repeat in every subsequent message, and the slot index it was
given. The token identifies the
session; a message whose token does not match the session registered for the source address is
ignored.

A client that receives no answer after 5 attempts, one second apart, gives up. The server answers
`REJECT` with reason `1` when every slot is taken, and reason `2` when the version does not match.

Repeating `CONNECT` from an address that already has a session re-sends the `ACCEPT`, at most once
every 200 ms.

### Chat

A client in a room may send `CHAT`. The server validates the text, applies the rate limit, and
broadcasts `CHAT_MESSAGE` to every member of that room, including the sender, with the sender's
slot. The message carries the slot rather than the name, because clients already learn the name
of every slot from `ROOM_UPDATE`; repeating it in each line would cost up to 13 bytes per
message. Both messages are reliable: a chat line is never silently dropped.

Chat is scoped to a room. There is no lobby-wide or server-wide chat.

### Console

`CONSOLE_COMMAND` carries one line typed in the developer console. Only the room owner may send it;
any other sender gets a `CONSOLE_TEXT` refusal. When the game file names a `consoleRule` and that
room rule is off, only the commands listed in `freeCommands` are accepted. A command sent before
the game starts is queued and runs at the start. The server answers with `CONSOLE_TEXT`, shown in
the console of the sender.

### Rooms

```
client                                   server
  | ---- LIST_ROOMS -------------------> |
  | <--- ROOM_LIST --------------------- |
  | ---- CREATE_ROOM or JOIN_ROOM -----> |
  | <--- ROOM_JOINED or ROOM_REJECTED -- |
  | <--- ROOM_UPDATE (every second) ---- |
  | ---- START_GAME -------------------> |  owner only
  | <--- GAME_STARTED ------------------ |
  | <--- SNAPSHOT / SNAPSHOT_DELTA ----- |  20 per second
```

The client that creates a room owns it: only the owner may `START_GAME`, `KICK_PLAYER` or
`DELETE_ROOM`. Any other sender receives `ROOM_REJECTED` with reason `7`.

Leaving as the owner closes the room: every member receives `ROOM_CLOSED`. A kicked player
receives `KICKED`.

A room may be password protected; `JOIN_ROOM` then has to carry the matching code. Only wrong-code
attempts are counted, and after five of them the session is refused with reason `6` for the rest
of its lifetime; the counter is never reset.

`CREATE_ROOM` and `JOIN_ROOM` sent by a client that is already in a room are not errors: the
server answers `ROOM_JOINED` again for the room it is in. A client that retries because it missed
the first answer therefore receives two `ROOM_JOINED`, with two different `reliableId` values.
Both are legitimate and must be handled.

### Disconnecting

A client sends `DISCONNECT` once. A client that stops sending is removed after 5 seconds of
silence, and the other members of its room receive `PLAYER_LEFT` with reason `2`. The server
sends `SERVER_SHUTDOWN` to every confirmed client before stopping.

Both sides must survive the disappearance of a peer at any point, including in the middle of a
message exchange.

## 8. Timing

These values are read from the game configuration; the table gives the reference game's settings.

| Parameter | Value | Who |
|---|---|---|
| Simulation rate | 60 ticks per second | server |
| Snapshot rate | every 3 ticks, so 20 per second | server |
| Input rate | every 33 ms | client |
| Input release | buttons are cleared after 500 ms without an input | server |
| Heartbeat | every 1000 ms | client |
| Connect retry | every 1000 ms, 5 attempts | client |
| Session timeout | 5000 ms of silence | both |
| Room update | every 1000 ms | server |
| Retransmission | `max(100 ms, rtt x 1.5)` | both |
| Keyframe | at most 5 s between full snapshots | server |

Inputs carry the **state** of the buttons, not press and release events: a lost input is corrected
by the next one 33 ms later, whereas a lost "released" event would leave a key stuck.

## 9. Worked example

Real bytes captured on the wire, one player holding Fire and Right. They were captured with protocol
version 8: with the current version the third byte is `09`, every other byte is laid out the same way.

### `INPUT`, 25 bytes

```
45 4e 08 02 00 00 00 18 00 00 00 16 00 1f ff ff 00 00 00 05 a5 ca ca a6 18
```

| Bytes | Field | Value |
|---|---|---|
| `45 4e` | magic | `EN` |
| `08` | version | 8 |
| `02` | type | `INPUT`, client to server |
| `00 00 00 18` | sequence | 24 |
| `00 00 00 16` | ack | 22 |
| `00 1f ff ff` | ackBits | packets 1 to 21 all received |
| `00 00` | reliableId | unreliable |
| `00 05` | payloadSize | 5 |
| `a5 ca ca a6` | token | |
| `18` | buttons | `0b00011000` = fire + right |

### `ENTITY_DESTROYED`, 22 bytes

```
45 4e 08 85 00 00 00 59 00 00 00 6d ff ff ff ff 00 03 00 02 00 10
```

`type` is `0x85`, server to client. `ackBits` is `ff ff ff ff`: the previous 32 packets all
arrived. `reliableId` is 3, so the sender keeps a copy until this packet is acknowledged. The
payload is the single identifier 16.

### `SNAPSHOT`, 118 bytes

```
45 4e 08 83 00 00 00 19 00 00 00 19 00 bf ff ff 00 00 00 62
00 00 00 03 01 05 00 01 00 00 00 00 00 00 00 00
04 01 01 64 00 00 00 00 00 01 03 00 00 00 00 00 00 ...
02 00 01 01 00 d2 00 c8 00 00 02 02 01 44 00 dc 00
```

| Bytes | Meaning |
|---|---|
| `00 00 00 03` | tick 3 |
| `01` | playing |
| `05` then `00 01 00 00 00 00 00 00 00 00` | 5 world values: stage 1, the rest zero |
| `04` | 4 player slots |
| `01 01 64 00 00 00 00 00 01 03 00 00 00 00 00 01` | slot 0: alive, 1 life, 100 health, score 0, avatar entity 1, values 0/0/1 |
| three slots of zeroes | slots 1 to 3 are empty |
| `02` | 2 entities |
| `00 01 01 00 d2 00 c8 00` | entity 1, kind 1, at (210, 200) |
| `00 02 02 01 44 00 dc 00` | entity 2, kind 2, at (324, 220) — the missile just fired |

### The same frame as a `SNAPSHOT_DELTA`

Between tick 30 and tick 33 the ship advances 30 pixels and three missiles advance 75 each; the
world values are unchanged and only slot 0 changes, in one of its values.

```
00 00 00 21   tick 33
00 00 00 1e   baselineTick 30
01            playing
00            no world value changed            (10 bytes saved)
01 00 <16>    one slot changed: slot 0          (48 bytes saved)
04            four entity changes
   00 01 01 1e      entity 1, dx only, +30
   00 02 01 4b      entity 2, dx only, +75
   00 03 01 4b      entity 3, dx only, +75
   00 04 01 4b      entity 4, dx only, +75      (16 bytes saved)
00            nothing removed
```

134 bytes become 69. Each entity costs 4 bytes instead of 8: the kind and the visual are omitted
because they did not change, and `+75` fits in one signed byte while `1074` needs two.

## 10. Writing a client

A minimal client has to:

1. Send `CONNECT` until an `ACCEPT` arrives, then store the token and repeat it in every message.
2. Maintain a sequence counter starting at 1, and fill `ack` and `ackBits` from the packets
   received so far. **Without this the server will retransmit endlessly.**
3. Send `HEARTBEAT` at least every 1000 ms, including while idle in menus. A client silent for
   5 seconds is dropped, and if it owns a room the room is closed.
4. Send `INPUT` about 30 times per second while in a game.
5. Accept packets of unknown type without crashing, and validate every length before reading.
6. Keep the last 32 reconstructed snapshots by tick, so that deltas can be applied.
7. Track the `reliableId` values already processed, and ignore repeats.

A client that ignores `SNAPSHOT_DELTA` entirely still works, at reduced quality: it will only
update on the full snapshots sent at most five seconds apart.

`scripts/protocol_probe.py` is a working headless client of about 300 lines, used to test this
protocol. It implements points 1 to 5 above and can deliberately drop a fraction of the packets
it receives, which makes it a readable reference when something does not behave as documented.

## 11. Security considerations

| Risk | Mitigation |
|---|---|
| Malformed or truncated datagram | Every length and count is validated before use; a failing packet is dropped silently |
| Memory exhaustion | Datagrams capped at 1200 bytes, receive queues at 256 entries, pending reliable messages at 64 per peer |
| Session hijacking | A 32-bit random token bound to the source address and port is required on every message |
| Connection flooding | At most 32 `CONNECT` per second, and one `ACCEPT` per client per 200 ms |
| Chat flooding | One chat message per session every 500 ms; text validated for length and printable characters |
| Endless retransmission to a peer that never acknowledges | Bounded by 64 pending messages per peer and by the 5 s session timeout |
| Cheating | The server is authoritative: positions, collisions, lives and scores are computed server-side and clients only submit button states |
| Amplification | No message produces an answer larger than its request, except `LIST_ROOMS`, which is only served to an established session |

The protocol is not encrypted and not authenticated beyond the session token. Tokens are
predictable only to the extent that the server's random generator is; traffic can be observed and
replayed by anyone on the path. This is acceptable for a LAN game and would not be for an exposed
service.
