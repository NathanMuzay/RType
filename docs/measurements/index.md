# Network measurements with netem

This test bench runs the real server against two headless C++ clients. The clients use
`engine::network::ClientConnection`, so reliability and delta snapshot reconstruction are exactly the ones of the
game. The clients never drop packets on purpose: delay, loss, duplication and reordering are applied by the Linux
kernel with `tc netem`.

| Tool | Role |
|---|---|
| `scripts/netem_client.cpp` | Two headless clients: create a room, join, start, fire, then report what they received as JSON |
| `scripts/netem_test.py` | Runs every network condition in a disposable network namespace and writes `report.md` |
| `scripts/protocol_probe.py` | Python client speaking the wire format directly, with optional simulated loss ([Protocol](../reference/protocol.md)) |
| `scripts/load_test.py` | Many players in parallel rooms, to measure how many games one server holds |

## Build

From the repository root, once the project builds normally ([Building](../getting-started/building.md)):

```sh
cmake --build build --target r-type_server --parallel "$(nproc)"
c++ -std=c++20 -O2 -pthread \
  $(find engine -type d -name include -printf '-I%p ') \
  -Ibuild/vcpkg_installed/x64-linux/include \
  scripts/netem_client.cpp \
  -Wl,--start-group $(find build -name 'libengine_*.a' -not -path '*/vcpkg*') -Wl,--end-group \
  -o build/bin/netem_client
```

Change the vcpkg include path if `VCPKG_INSTALLED_DIR` is customised.

Quick check without netem, against a running server:

```sh
./build/bin/r-type_server -p 4399 &
./build/bin/netem_client build/bin/data/rtype_game.json 4399 5
```

The last line, `NETEM_RESULT {...}`, must contain `"passed":true` for both clients.

## Run every condition

Install `iproute2` if `tc` or `ip` is missing. The kernel must allow user and network namespaces and provide the
netem module (a standard Ubuntu or WSL2 kernel does).

```sh
NETEM_HOST_NS=$(readlink /proc/self/ns/net)
unshare --user --map-root-user --net \
  python3 scripts/netem_test.py --host-netns "$NETEM_HOST_NS" \
  --server build/bin/r-type_server \
  --client build/bin/netem_client \
  --seconds 20 --output docs/measurements/my-run
```

The script checks that its network namespace differs from the host one, only enables its own loopback and never
changes a host interface. It stops every server and removes the qdisc at the end; destroying the namespace also
removes the settings if the process is killed.

To run a single condition, add for example `--case loss_10pct`. The seven conditions are defined in
`scripts/netem_test.py`, and their exact commands are kept in `results.json`. Each output is saved with its
scenario, and `report.md` presents the measurements and their limits.

## Runs

| Date | Report |
|---|---|
| 2026-11-08 | [netem results](netem-2026-11-08/report.md) |

The 2026-11-08 run was made before the chat and player names were added (protocol v8). Libraries were built
from the sources of that day with CMake in Release (`-O3 -DNDEBUG`), then the server and the probe were linked
with GCC 13, C++20 and `-O2`. On that machine the pybind11 CMake package was missing from vcpkg, so a local
CMake configuration exposed the pybind11 headers already installed and linked them to the system Python 3.12.3.
The game, transport and protocol code were not modified for the run. Only the report is kept in the repository:
run the script again to get the raw logs and `results.json`.
