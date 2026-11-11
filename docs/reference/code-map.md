# Code map

Where to look for what. Paths are relative to the repository root.

## Repository

```text
.
├── CMakeLists.txt          declares the games (engine_add_game)
├── vcpkg.json              dependencies
├── mkdocs.yml              documentation website
├── engine/                 the engine (C++)
├── game/                   R-Type: data/, scripts/, assets/rtype/
├── game2/                  Skyhop: data/, scripts/skyhop/, assets/skyhop/
├── docs/                   this documentation
└── scripts/play.sh         builds and starts a server and N clients
```

## Engine

```text
engine/
├── CMakeLists.txt          every library, options, engine_add_game
├── apps/src/               ServerMain, ClientMain, LocalMain (generic launchers)
├── core/                   errors, clock, timestep, events, geometry, command line, asset paths
├── physics/                Aabb, collision detection
├── ecs/
│   ├── include, src        Registry, SparseArray, Entity, systems manager, UiStore
│   ├── components/<cat>/   physics, combat, gameplay, presentation, interface
│   └── systems/<cat>/      events, physics, gameplay, presentation, interface
├── scene/                  DataNode, factories, prefabs, scene builder, component and system loaders
├── net/                    UdpTransport, ByteWriter, ByteReader
├── network/
│   ├── protocol/           Protocol, Packet, Codec, visitors, NetworkSettings
│   ├── messages/<cat>/     connection, lobby, game, console
│   ├── server/             GameServer, GameRoom, ClientSession, ServerRunner
│   └── client/             ClientConnection
├── render/                 Window, SfmlRenderer, textures, fonts
├── audio/                  SfmlAudio
├── input/                  KeyBindings, InputManager
├── client/                 GameView, OnlineClient, SnapshotPresenter, console, debug metrics
├── scripting/              ScriptEngine, ScriptSystem, ScriptWorld, ScriptEntity, Python bindings
├── simulation/             Simulation, SimulationFactory
└── local/                  LocalClient (solo mode)
```

## I want to change...

| Topic | Look at |
|---|---|
| The main loop of the server | `network/server/src/ServerRunner.cpp` |
| What happens in one tick | `simulation/src/Simulation.cpp` |
| What goes into a snapshot | `simulation/src/SimulationSnapshot.cpp` |
| How a snapshot is shown | `client/src/SnapshotPresenter.cpp`, `client/src/GameViewSync.cpp` |
| Lobby and rooms on the server | `network/server/src/GameServerRooms.cpp` |
| Lobby and rooms on the client | `client/src/OnlineClientLobby.cpp` |
| Connection, heartbeat, input sending | `network/client/src/ClientConnection.cpp` |
| Packet validation | `network/messages/*/src/*Message.cpp` (`isValid`), `network/protocol/src/Codec.cpp` |
| How a JSON component is read | `scene/src/EngineComponentLoaders*.cpp` |
| How a JSON system is built | `scene/src/EngineSystemBuilders.cpp` |
| What Python can call | `scripting/src/ScriptApiBindings.cpp`, `ScriptEntity.cpp`, `ScriptWorld.cpp` |
| How hooks are called | `scripting/src/ScriptSystemImpl.cpp` |
| Drawing order | `ecs/systems/presentation/src/RenderSystem.cpp` |
| Keyboard and gamepad | `input/src/InputManager.cpp`, `render/src/Window.cpp` |
| Console | `client/src/DevConsole.cpp`, `game/scripts/console.py` |
| Solo mode | `local/src/LocalClient.cpp` |

## R-Type content

| Topic | File |
|---|---|
| Everything global (values, rules, scenes, HUD) | `game/data/rtype_game.json` |
| Gameplay entities | `game/data/rtype_server_prefabs.json` |
| Appearance and UI parts | `game/data/rtype_client_prefabs.json` |
| Player settings | `game/data/rtype_settings.json` |
| Stage order, intro, boss, victory | `game/scripts/campaign.py` |
| Stages | `game/scripts/stage1.py`, `stage2.py`, `stage3.py` |
| Bosses | `dobkeratops.py`, `warship.py`, `vessel.py`, `boss_common.py` |
| Enemies | `bydo.py`, `pod.py`, `hopper.py`, `heavy.py`, `chaser.py`, `turret.py`, `splitter.py`, `mine.py`, `carrier.py`, `snake.py`, `ground_turret.py` |
| Player, power-ups, Force | `player.py`, `powerup.py`, `force.py` |
| Shared helpers | `common.py` |
