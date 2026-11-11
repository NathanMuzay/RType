# Security

## Vulnerabilities per technology

| Technology | Main risks |
|---|---|
| UDP | Source address spoofing, flooding, amplification (small request, large reply), loss and reordering used to confuse state |
| C++ | Buffer overflows, out-of-bounds reads, use-after-free, uncaught exceptions ending the process |
| Binary protocol | Malformed lengths, oversized counts, invalid enum values, forged fields |
| Embedded Python | A script can do anything the process can (files, network); a script error could stop the game |
| JSON data | Malformed or hostile files: huge values, wrong types |
| Third-party libraries (SFML, Asio, nlohmann-json, Python) | Vulnerabilities in image or audio decoding, networking, parsing |
| Asset files | Malformed images or sounds loaded by the client |

## Mitigations implemented

| Risk | Mitigation | Where |
|---|---|---|
| Out-of-bounds reads | Every read is bounds-checked; a short payload throws and the datagram is dropped | `ByteReader` |
| Oversized packets | Datagrams over 1200 bytes dropped; the writer refuses to exceed the limit | `UdpTransport`, `ByteWriter` |
| Malformed packets | Magic, version, direction, payload size, exact length, field ranges, text characters, list sizes and consistency checked; any failure drops the packet | `Codec`, `isValid` of each message |
| Struct layout attacks | Nothing is copied to or from the wire as raw memory | Every message |
| Memory exhaustion | Receive and send queues capped at 256 datagrams, client and room limits (192 and 48), entity limit per snapshot, text fields capped | `UdpTransport`, `NetworkSettings`, `TextRules` |
| Connection flooding | 32 `CONNECT` per second at most; `ACCEPT` repeated at most every 200 ms per address | `GameServer` |
| Spoofed messages | A message must come from the session's address and carry its random non-zero token | `GameServer` |
| Replay and reordering | Sequence numbers for inputs and heartbeats, tick numbers for snapshots | `GameServer`, `ClientConnection` |
| Amplification | Nothing large is sent before the session is confirmed and has joined a started room | `GameServer` |
| Room code brute force | 5 wrong codes per session, then joins are refused | `GameServer` |
| Cheating | Every gameplay decision is made by the server; clients only send buttons | Architecture |
| Console abuse | Only the room owner, only with the console rule ON | `GameServer::handleConsole` |
| Script failures | Exceptions are caught per hook, the script is disabled for that entity only | `ScriptSystemImpl` |
| Bad data files | Typed access, errors reported with the path, program stops before running | `DataNode` |
| Crashes | Every error is a project exception caught by the launcher; I/O thread errors are forwarded to the main thread | Launchers, `UdpTransport` |
| Memory safety | RAII, containers, `std::span`, no `new` / `delete` | Whole code base |
| Dependency drift | vcpkg baseline pinned: everyone builds the same versions | `vcpkg.json` |

## Known limits

- Tokens prove address ownership, not identity: there is no encryption, signature or account. Someone who can
  watch the traffic can impersonate a player. The game targets LAN play.
- A modified packet whose fields stay valid is accepted (no checksum or MAC; UDP's own checksum only catches
  accidental corruption).
- Scripts are trusted content: run only games whose scripts you trust. Sandboxing Python reliably is not possible
  inside the same process.

## Long-term monitoring

| Practice | How |
|---|---|
| Follow advisories of the dependencies | Watch the GitHub security advisories and changelogs of SFML, Asio, nlohmann-json, pybind11 and CPython |
| Update dependencies | Move the vcpkg `builtin-baseline` regularly, in a dedicated pull request, then run the full test checklist |
| Watch the server | The server logs connections, departures with their reason, rooms and script errors; unusual numbers of clients or rooms are visible there |
| Review protocol changes | Every change of a message requires an RFC update and a review of its `isValid` |
| Report problems privately | [Security policy](../contributing/security-policy.md) |

Planned improvements: counters of dropped datagrams per reason and per address, printed periodically, and
alerts when an address exceeds a normal rate.
