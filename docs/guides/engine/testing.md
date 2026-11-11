# Testing

The project has no automated test suite in the repository yet. This page describes how changes are checked
today and how to test the simulation without a window.

## Continuous integration

Every push to `main` or `dev` and every pull request runs `.github/workflows/ci.yml` on GitHub Actions:

| Job | Checks |
|---|---|
| Format check | `clang-format --dry-run --Werror` on every engine file |
| Build ubuntu-24.04 | Full build with GCC and Ninja, then starts `r-type_server` for 3 seconds |
| Build windows-2022 | Full build with MSVC |

Dependencies are not rebuilt at every run: vcpkg binary packages are kept in the GitHub Actions cache, keyed on
the vcpkg commit and `vcpkg.json`. The first run takes long, later runs reuse the cache. The executables of each
run can be downloaded from the run page (artifacts, kept 7 days).

## Before every merge

| Check | How |
|---|---|
| Builds without warnings on Linux | `cmake --build build --parallel "$(nproc)"` |
| Builds on Windows | `cmake --build out\windows --config Release --parallel 4` |
| R-Type online | One server, two clients on the same machine (`scripts/play.sh 2`), play a stage |
| R-Type on a LAN | Server on one PC, client on another with `-h <address>` |
| Skyhop | `./skyhop`, play, die, replay, pause, back to menu |
| Lobby | Create a room with a code, join with a wrong code then the right one, kick, leave as owner |
| Gamepad | Menus and game with a pad |
| Data change | The game starts (load errors stop it) and the changed content behaves as expected |

## Testing the simulation without a window

`Simulation` does not need a window, a network or a client. A small program can run a game faster than real
time, send inputs and read snapshots:

```cpp
#include "DataNode.hpp"
#include "NetworkSettings.hpp"
#include "ScriptEngine.hpp"
#include "Simulation.hpp"

int main()
{
    const std::filesystem::path data = "build/bin/data";
    const auto settings = engine::network::NetworkSettings::fromGame(
        engine::scene::DataNode::fromFile(data / "rtype_game.json"));
    engine::scripting::ScriptEngine scripts("build/bin/python");
    const std::vector<std::uint8_t> rules(settings.ruleCount(), 0);
    engine::simulation::Simulation simulation(data, "rtype", settings, scripts, rules);
    simulation.addPlayer(0);
    for (int tick = 0; tick < 60 * 120; ++tick) {
        simulation.setInput(0, engine::network::Protocol::BUTTON_FIRE);
        simulation.update(1.f / 60.f);
    }
    const auto snapshot = simulation.snapshot();
    return snapshot.gameState() == engine::network::Protocol::GAME_OVER ? 1 : 0;
}
```

Link it with `engine_simulation`. This kind of program can check that:

- a stage reaches its boss and its victory (read the `phase` world value);
- every snapshot fits in a packet: encode it with `Codec` and decode it back;
- a level is passable with a scripted sequence of inputs;
- no script error appears on the error output.

## Testing the network

| Tool | What it checks |
|---|---|
| `scripts/protocol_probe.py <port> [loss]` | Speaks the wire format from Python; with a loss ratio, checks that reliable messages and delta snapshots survive dropped packets |
| `scripts/load_test.py <port> <games>` | Starts many rooms at once and reports the simulation rate, snapshots per second and bandwidth |
| `scripts/netem_client.cpp` + `scripts/netem_test.py` | Real `ClientConnection` under kernel-level delay, loss, duplication and reordering ([Network measurements](../../measurements/index.md)) |

## Testing the protocol

`Codec::decode` must reject malformed data without crashing. Feed it truncated datagrams, wrong magic, wrong
sizes, random bytes: it must return `std::nullopt` every time ([Protocol, section 8](../../reference/protocol.md)).
