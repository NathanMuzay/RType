# Design decisions

Each entry gives the decision, why it was taken, what it costs, and what it makes possible.
New decisions are added at the end, with the same structure.

## 1. A generic engine, games made of data

**Decision.** `engine/` contains everything written in C++. A game is a folder of JSON, Python and assets.

**Why.** Content creators must be able to add levels, enemies and menus without touching the engine, and the
engine must prove it is reusable by running more than one game.

**Cost.** Every feature must be designed generically, and errors in data are found at load time instead of
compile time.

**Makes possible.** Two different games (R-Type and Skyhop) on the same engine, content edited by people who do
not write C++.

## 2. Authoritative server, display-only client

**Decision.** The server simulates everything. Clients send button states and draw snapshots.

**Why.** A single source of truth keeps every player in the same game and prevents cheating on positions,
lives or score.

**Cost.** Without client-side prediction, a player's own ship reacts after one round trip. On a LAN this delay
is not noticeable.

**Makes possible.** Client prediction later, which needs exactly this model.

## 3. Entity Component System

**Decision.** Game objects are entities made of data components, processed by systems.

**Why.** Composition instead of inheritance: behaviours combine freely and can be added or removed at run time
(a respawned ship gets `Invulnerable` for two seconds, then loses it).

**Cost.** The behaviour of one object is spread over several systems.

**Makes possible.** Prefabs written in JSON as lists of components.

## 4. Sparse arrays and component masks

**Decision.** One `SparseArray<T>` per component type, indexed by entity, plus a 64-bit mask per entity for
queries.

**Why.** Direct access by entity, contiguous memory for loops, and queries that compare integers instead of
looking at each storage.

**Cost.** Memory is allocated up to the highest entity index for every component type, and a game is limited to
64 component types.

## 5. Generational entity handles

**Decision.** An entity is an index plus a generation.

**Why.** Indices are reused; the generation makes an old handle (kept by a script or a system) detectably stale
instead of silently pointing to a new entity.

## 6. System order chosen by the game

**Decision.** Systems are created by name from the game JSON, and run in the listed order.

**Why.** The order matters (movement before collision, collision before damage) and differs between games:
Skyhop does not need weapons, R-Type does not need gravity.

**Cost.** A wrong order is a data bug, not a compile error.

## 7. Python for game behaviours

**Decision.** Behaviours specific to a game are Python scripts, embedded with pybind11, server side only.

**Why.** JSON cannot express timelines, boss patterns or conditions. Python is readable by content creators,
needs no compilation, and is a standard choice for game scripting. See
[Libraries and tools](../comparative-study/libraries.md).

**Cost.** An embedded interpreter, a copy of the Python standard library next to the server, and slower code
than C++ (acceptable for a few hundred entities).

**Makes possible.** Errors isolated per entity: a broken script never crashes the server.

## 8. Fixed-timestep simulation

**Decision.** The simulation always advances by 1/60 s, independently of the frame rate.

**Why.** Identical physics on every machine, and a simulation that behaves the same for the same inputs.

## 9. Binary UDP protocol with snapshots and deltas

**Decision.** One UDP datagram per message, a 20-byte header carrying sequence and acknowledgements, world
state 20 times per second as a delta against the last snapshot the client acknowledged, with a full snapshot
as a keyframe. Messages that must not be lost (lobby actions, game events, chat) are resent until acknowledged.

**Why.** TCP's retransmissions would delay fresh data behind stale data. A lost snapshot is replaced by the next
one, and the 1200-byte limit avoids fragmentation. Deltas halve the bandwidth of a full snapshot.

**Cost.** The server keeps a short snapshot history per client, and a client must apply a delta on the exact
baseline it names.

**Makes possible.** More entities per packet, and play on slower links ([measurements](../measurements/index.md)).

## 10. Network code hidden behind interfaces

**Decision.** `GameServer` only knows `IGameSession` and `ISessionFactory`. The simulation only exchanges
buttons and snapshots.

**Why.** The network layer and the game evolve separately, and the transport can be replaced.

**Makes possible.** The solo mode, which plugs the simulation directly into the view (decision 13).

## 11. Visitors instead of switches on message types

**Decision.** `Packet::visit` applies a visitor (`MessageValidator`, `MessageWriter`, `TokenReader`) to the
message held in a `std::variant`.

**Why.** Adding a message no longer means editing several long `switch` statements: the compiler finds every
place that must handle it.

## 12. Third-party libraries wrapped in one module each

**Decision.** SFML only in render, audio and input. Asio only in net. nlohmann-json only in scene. pybind11 only
in scripting. Each is a private dependency.

**Why.** Replacing a library touches one module. Headers of the rest of the engine stay free of third-party
types.

## 13. Solo mode as a second transport

**Decision.** `LocalClient` runs the simulation in the client process and applies its snapshots directly.

**Why.** Some games do not need a server, and players should not have to start one.

**Cost.** The solo build needs both the client and the server modules.

## 14. Project-specific exceptions and exit code 84

**Decision.** Every error is an exception deriving from `EngineError`, named after its module. Launchers exit
with code 84 on error.

**Why.** Clear messages that name the class that failed, and the exit code required by the school.

## 15. CMake and vcpkg, Linux and Windows

**Decision.** CMake with a vcpkg manifest (`vcpkg.json`).

**Why.** The same commands on both platforms, and dependencies installed automatically at the version fixed by
the baseline. See [Libraries and tools](../comparative-study/libraries.md).

## 16. Run-time plugins for systems

**Decision.** Systems can be shipped as shared libraries, listed in the game file, loaded at start and reloaded when
their file changes. The engine loads a temporary copy, so the original can be rebuilt while the game runs.

**Why.** The last level of modularity described by the subject: extend the engine without rebuilding it, and
iterate on a system in seconds.

**Cost.** Plugins must be built with the same compiler and engine version, and only add systems.

## 17. Components without virtual functions

**Decision.** `IComponent` is a plain base with a protected, non-virtual destructor.

**Why.** Components are stored by value and never deleted through a base pointer, so a virtual destructor only
added a hidden pointer to every component. That pointer referenced the code of whoever created the component: a
component created by a plugin pointed into the plugin, and crashed after the plugin was reloaded. Components are
now plain data, smaller and safe to share between the engine and plugins.

## Known limits

| Limit | Effect | Possible improvement |
|---|---|---|
| No client prediction | One round trip of input delay | Predict the local avatar, reconcile with snapshots |
| No encryption | Fine on a LAN, not on the Internet | DTLS, or a secure handshake |
| 64 component types | Enough for both games | A larger mask type |
| One Python interpreter | Scripts of all rooms share it | Sub-interpreters if isolation becomes needed |
