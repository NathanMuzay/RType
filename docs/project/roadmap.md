# Roadmap

## Done

| Area | Features |
|---|---|
| Engine | Modules with private third-party dependencies, ECS with sparse arrays, data-driven prefabs, scenes and systems, standalone CMake with `engine_add_game`, generic launchers |
| Scripting | Embedded Python with hooks, per-entity error isolation, full world and entity API, console scripts |
| Network | Binary UDP protocol v9, sessions, heartbeats, timeouts, reliable messages, delta snapshots, rooms with codes and rules, kicks, room chat, player names, console messages, sounds |
| Client | Menus, lobby, room and game scenes in JSON, HUD bound to values, pause menu, F1 console, F3 metrics, loading screen, gamepad |
| Solo mode | `LocalClient`, `LOCAL` games |
| R-Type | Three themed stages (a warship corridor, a giant vessel hunting the players), three bosses, power-ups, the Force (front or back, free, super-charged shots), charged shot, level choice, difficulty scaled by player count, friendly fire and bonus rules, intro, ending and credits, two-player versus mode |
| Second game | Skyhop, solo, Kenney CC0 assets |
| Platforms | Linux, Windows, LAN play |
| Documentation | Architecture, references, guides, RFC, comparative study, accessibility, team processes |
| Settings screen | Volumes, text size, toggle fire, labels, dead zone, key and button rebinding, saved per player |
| Plugins | Systems in shared libraries, loaded from the game file and reloaded while the game runs |
| Tooling | GitHub Actions CI (format check, Linux and Windows builds, cached dependencies), `clang-format` |

## Next

| Item | Track | Why |
|---|---|---|
| Entity interpolation and client-side prediction | Network | Hide the round-trip delay on the local ship |
| Player identities, persistent sessions, reconnection | Network | Rejoin a game after a crash |
| Global scoreboard | Network | Keep scores across games |
| Measurements under `clumsy` on Windows | Network | Same evidence as the Linux `netem` run |
| Colorblind palettes, option to disable shake and flashes | Accessibility | See [Accessibility](accessibility.md) |
| Automated tests of the simulation and the codec | Architecture | See [Testing](../guides/engine/testing.md) |

Each item becomes a GitHub issue and is developed on its own branch ([Git workflow](../contributing/git-workflow.md)).

## Part 2 — Advanced networking track

| Feature | Status |
|---|---|
| Per-client packet sequencing | Done |
| Acknowledgements with a 32-bit bitfield, retransmission of reliable messages | Done |
| Round-trip time measured per client, server side | Done |
| Delta snapshots against an acknowledged baseline | Done |
| Compact entity encoding, field masks, `int8` position deltas | Done |
| Protocol reference rewritten (now version 9) | Done |
| Headless protocol probe with simulated loss (`scripts/protocol_probe.py`) | Done |
| Load generator and scaling measurements (`scripts/load_test.py`) | Done |
| Room and client limits raised to the protocol ceiling (48 / 192) | Done |
| Text chat per room | Done |
| Player identities, persistent sessions, reconnection | To do |
| Global scoreboard | To do |
| Server administration console | To do |
| Measurements under `netem` and `clumsy` | Linux `netem`: [measured with two production clients](../measurements/netem-2026-11-08/report.md). Windows `clumsy`: not run |
| Entity interpolation, client prediction with reconciliation | Out of the chosen scope, analysed in the [comparative study](../comparative-study/networking.md) |
