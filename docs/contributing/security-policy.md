# Security policy

## Reporting a vulnerability

If you find a way to crash the server, to act for another player, or to run code on someone's machine:

1. **Do not open a public issue.** Contact a maintainer directly (see the README authors).
2. Give the steps to reproduce, or the datagram that triggers the problem (hexadecimal dump).
3. A fix is prepared on a private branch, then merged and released with a short note.

## What the server guarantees

| Threat | Protection |
|---|---|
| Malformed datagrams | Every field is checked before use, decoding never reads past the datagram, invalid packets are dropped silently |
| Oversized packets | Datagrams above 1200 bytes are dropped by the transport |
| Flood of connection requests | 32 `CONNECT` per second at most, 16 clients at most |
| Session hijacking | 32-bit random token bound to the sender's address and port |
| Replayed or reordered inputs | Sequence numbers per session, older ones dropped |
| Brute force on room codes | 5 wrong codes per session, then refused |
| Cheating by clients | Clients only send buttons; positions, damage, score are computed by the server |
| Console abuse | Commands only for the room owner, and only when the room rule `cheats` is ON |

## Known limits

- No encryption and no server authentication: the protocol is meant for LAN or trusted networks.
- Scripts are trusted content: a Python script can do anything the server process can. Only run games whose
  scripts you trust.
- Third-party libraries are pinned by the vcpkg baseline; updating it is the way to receive security fixes.

See the [security part of the comparative study](../comparative-study/security.md) for the analysis.
