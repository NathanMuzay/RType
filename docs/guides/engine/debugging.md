# Debugging

## In game

| Tool | How | Shows |
|---|---|---|
| Debug overlay | **F3** | FPS, ping, entity count, memory, received and sent KB/s, ping history (30 bars) |
| Console | **F1** | Server answers, local settings, cheats to reach a situation quickly (`spawn`, `skip`, `god`) |
| `showMetrics` | `set showMetrics 1` in the console | Same as F3 |

Typical use: reproduce a boss bug with a room created with CHEATS ON, then `skip` until the boss and `god`.

## Server output

The server prints its events on the standard output (connections, rooms, rounds, kills) and script errors on
the error output. Keep the terminal of the server visible while testing.

```text
[script] warship.py on_update failed, script disabled for this entity:
Traceback (most recent call last):
  File ".../scripts/warship.py", line 42, in on_update
NameError: name 'speeed' is not defined
```

Add temporary traces from a script with `world.log("x=%.1f" % e.x())`: they appear as `[script] ...`.

## Load errors

Data errors stop the program at load time with exit code 84 and one line naming the problem:

| Message starts with | Typical cause |
|---|---|
| `DataNode: cannot open` | Wrong file name in `files`, or data not copied (rebuild) |
| `DataNode: invalid JSON in` | Missing comma or quote. The message gives the parser's position |
| `DataNode: <path> must be ...` | Wrong type (a number written as text...) |
| `LayerTable: unknown layer` | A `Collider.hits` name not in `layers` |
| `PrefabLibrary: unknown prefab` | Typo in a prefab name |
| `KeyBindings: unknown key name` | Typo in the settings file |
| `NetworkSettings: ... do not fit in one packet` | `maxEntities` or `maxPlayers` too high |

## With a debugger

Build in Debug:

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel "$(nproc)"
gdb --args ./build-debug/bin/r-type_server -p 4242
```

On Windows, open `out\windows\RType.sln` in Visual Studio, set the executable as startup project and press F5,
or build with `--config Debug`.

Useful breakpoints:

| Problem | Breakpoint |
|---|---|
| A packet is ignored | `Codec::decode` (each `return std::nullopt` is one rejected check) |
| An entity disappears | `Registry::kill`, `CleanupSystem::update` |
| Damage not applied | `DamageSystem::canInteract` |
| A client does not join | `GameServer::handleJoinRoom` |

## Network

Watch the traffic with Wireshark on the game port (`udp.port == 4242`). Every datagram starts with `45 4E 07`
(magic and version 7), the fourth byte is the message type ([Protocol](../../reference/protocol.md)).
