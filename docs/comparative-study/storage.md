# Storage

## What the project stores

| Data | Where | Format | Persistence |
|---|---|---|---|
| Game definition (entities, scenes, HUD, rules) | `data/<game>_*.json` | JSON | Files, read at start |
| Behaviours | `scripts/*.py` | Python source | Files, read at first use |
| Assets | `game/assets/rtype/`, `game2/assets/skyhop/` | PNG, GIF, OGG, MP3, WAV, TTF | Files, read on demand and cached |
| Player settings | `data/<game>_settings.json` | JSON | File, read at start |
| Live game state (entities, lives, scores) | Server memory | ECS registry | Lost when a room closes: a round is a short arcade session |
| Client state | Client memory | ECS registry, UI store | Rebuilt from snapshots |

Live state stays in memory on purpose: it changes 60 times per second and nothing in a round must survive a
restart.

## Techniques compared

| Technique | Persistence | Reliability | Constraints | Fits |
|---|---|---|---|---|
| Compiled constants | None (rebuild to change) | Total | Requires C++ and a build | Engine limits (packet size, queue sizes) |
| **JSON files** (ours) | Yes | Human-editable, may be invalid, so validated on load | Small, read once | Game content, settings |
| Binary files | Yes | Compact, versioning needed | Not editable by hand | Replays, very large maps |
| SQLite | Yes, transactional | Atomic writes, survives crashes | Embedded library | Scoreboards, accounts |
| Database server | Yes | High, with backups | A service to operate | Online rankings across servers |

## Reliability of the data files

- Every key is read through `DataNode`, which checks the type and reports the file and the path of a wrong value.
- Loading is all or nothing: a faulty file stops the program with a clear message and exit code 84, before
  anything runs. A game never starts half-loaded.
- Scripts are loaded when first needed: a broken script disables only its entities.
- Game files are versioned in Git with the code, so a content change can be reviewed and reverted like code.

## Constraints

| Constraint | Value |
|---|---|
| Size of data read at start | A few hundred kilobytes of JSON per game |
| Assets | Loaded once and kept in memory (`TextureManager`, `FontManager`); music streamed |
| Disk writes | None during play: the engine only reads files, which avoids corruption on crashes |

## Possible extensions

- **High scores**: a small JSON file written to a temporary file then renamed (atomic on both systems), or SQLite
  if history and queries are needed.
- **Settings menu**: write `<game>_settings.json` the same way.
- **Replays**: record the input stream per tick in a binary file; the fixed-timestep simulation replays it.
