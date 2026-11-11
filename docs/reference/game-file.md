# Game file reference

The game file `data/<game>_game.json` is the entry point of a game. This page lists every key the engine reads.
Examples come from `game/data/rtype_game.json` and `game2/data/skyhop_game.json`.

## General

| Key | Type | Required | Description |
|---|---|---|---|
| `world` | `{width, height}` | yes | Size of the game world in world units. Everything is placed in this space. R-Type: 1920 x 1080 |
| `window` | `{width, height, title}` | yes | Initial window size in pixels and title. The world is scaled to fit |
| `tickRate` | integer | yes | Simulation ticks per second (60) |
| `files` | object | yes | Names of the other files, see below |
| `startScene` | string | yes | First client scene after loading |
| `loadingScene` | string | no | Scene shown while `preload` is loaded |
| `loadingMinSeconds` | number | no | Minimum time on the loading scene (default 0) |
| `preload` | list of strings | no | Texture and sound keys loaded at start |
| `playerPrefab` | string | yes | Server prefab spawned for each player |
| `consoleRule` | string | no | Rule that must be ON for console commands to run (R-Type: `cheats`) |
| `plugins` | list of strings | no | Plugins to load from `bin/plugins/` ([Write a plugin](../guides/engine/write-a-plugin.md)) |

### `files`

| Key | Description |
|---|---|
| `serverPrefabs` | Server prefab file, relative to `data/` |
| `clientPrefabs` | Client prefab file, relative to `data/` |
| `settings` | Player settings file, relative to `data/` |
| `scripts` | Scripts folder, relative to `data/` (R-Type: `../scripts`, Skyhop: `../scripts/skyhop`) |
| `console` | Console script inside the scripts folder (optional) |

## Network

The `network` block is read by `NetworkSettings` on both sides. Both sides MUST use the same values.

| Key | R-Type | Description |
|---|---|---|
| `defaultPort` | 4242 | Port used when `-p` is not given |
| `maxPlayers` | 4 | Seats per room |
| `maxEntities` | 130 | Maximum entities in one snapshot |
| `snapshotEveryTicks` | 3 | One snapshot every N ticks |
| `inputIntervalMs` | 33 | Client input period |
| `inputReleaseDelayMs` | 500 | Inputs released after this silence |
| `heartbeatIntervalMs` | 1000 | Client heartbeat period |
| `connectRetryIntervalMs` | 1000 | Delay between two `CONNECT` |
| `connectAttempts` | 5 | `CONNECT` sent before giving up |
| `sessionTimeoutMs` | 5000 | Silence before a client is dropped |
| `maxClients` | 192 | Clients connected at the same time |
| `maxRooms` | 48 | Rooms at the same time |
| `roomNameMaxLength` | 16 | Characters in a room name |
| `roomCodeMaxLength` | 8 | Characters in a room code |
| `maxJoinAttempts` | 5 | Wrong codes allowed per session |
| `chatMaxLength` | 64 | Characters in a chat message |
| `chatIntervalMs` | 500 | Minimum delay between two chat messages of one client |
| `playerNameMaxLength` | 12 | Characters in a player name |

Every key is required, even by a solo game such as Skyhop, which shares the settings code.

`maxPlayers` and `maxEntities` must give a snapshot of at most 1200 bytes, or the server refuses to start with
`maxPlayers and maxEntities do not fit in one packet`.

## Gameplay tables

| Key | Type | Description |
|---|---|---|
| `layers` | list of strings | Collision layers, at most 32. `Collider.layer` and `Collider.hits` use these names |
| `worldValues` | list of `{name, labels?}` | Values shared with every client (stage, phase, score...) |
| `playerValues` | list of `{name, labels?}` | Values per player (weapon, charge...) |
| `rules` | list of `{name, labels, maxPlayers?}` | Room options chosen at creation. The first label is the default. `maxPlayers`, one number per label, limits the seats of a room using that label |
| `eventSounds` | list of strings | Sound keys the server may play with `world.play_sound` |
| `kinds` | object `"number": "client prefab"` | Appearance of each `Replicated.kind` |
| `serverScene` | list of entities | Entities created when a simulation starts (R-Type: `match`, `director`) |
| `serverSystems` | list of names | Server systems in execution order, see [Systems](systems.md) |

```json
"worldValues": [
    {"name": "stage"},
    {"name": "phase", "labels": ["intro", "running", "boss", "clear", "victory", "defeat"]}
],
"rules": [
    {"name": "difficulty", "labels": ["NORMAL", "HARD", "EASY"]},
    {"name": "cheats", "labels": ["OFF", "ON"]}
]
```

## Resources

| Key | Type | Description |
|---|---|---|
| `textures` | object `key: path` | Images, relative to the assets folder |
| `fonts` | object `key: path` | Fonts (`.ttf`) |
| `sounds` | object `key: path` | Short sounds |
| `music` | object `key: path` | Streamed music |

```json
"textures": {"player_ships": "sprites/players/r-typesheet42.gif"},
"fonts": {"hud": "fonts/bytebounce.medium.ttf"},
"sounds": {"shot": "sounds/LASER_GUN.mp3"},
"music": {"main": "music/MAIN_THEME.mp3"}
```

## Client

| Key | Type | Description |
|---|---|---|
| `scenes` | object `name: scene` | Client scenes, see [Prefabs and scenes](../guides/content/prefabs-and-scenes.md) |
| `renderSystems` | list | Usually `[{"system": "Render", "background": [r, g, b, a]}]` |
| `uiMessages` | object `key: text` | Texts shown by the online client (connection errors, room errors, tags) |

### `uiMessages` keys

`online`, `connecting`, `connectionFailed`, `invalidAddress`, `roomLost`, `kicked`, `roomClosed`, `roomNotFound`,
`roomFull`, `wrongCode`, `tooManyRooms`, `invalidRoomName`, `tooManyAttempts`, `notAllowed`, `locked`, `playing`,
`waiting`, `noRooms`, `ownerTag`, `youTag`, `emptySlot`, `playerLeft`, `playerLost` (`{slot}` is replaced by the
player number).

Translating these texts is enough to translate the online screens.

---

# Settings file reference

`data/<game>_settings.json` holds the default preferences of a game. The settings screen saves the player's own
choices in `data/<game>_user_settings.json` (same keys), which is read instead when it exists.

| Key | Default | Description |
|---|---|---|
| `controls.up`, `down`, `left`, `right` | - | Movement keys |
| `controls.fire` | - | Fire key |
| `controls.action` | `LShift` | Action key |
| `toggleFire` | `false` | `true`: press fire once to start firing, once to stop (no need to hold) |
| `showPlayerLabels` | `true` | Shows `P1`, `P2`... above ships |
| `musicVolume` | 40 | 0 to 100 |
| `soundVolume` | 70 | 0 to 100 |
| `textScale` | 1.0 | Multiplies every text size |
| `serverAddress` | `127.0.0.1` | Server used when `-h` is not given |
| `gamepad.fire` | 0 | Gamepad button for fire |
| `gamepad.action` | 1 | Gamepad button for action |
| `gamepad.deadzone` | 35 | Stick dead zone, 0 to 100 |

**Key names**: a letter `A` to `Z`, a digit `0` to `9`, or one of `Up`, `Down`, `Left`, `Right`, `Space`,
`Enter`, `Tab`, `Backspace`, `LShift`, `RShift`, `LControl`, `RControl`, `LAlt`, `RAlt`.
An unknown name stops the client with `KeyBindings: unknown key name <name>`.
