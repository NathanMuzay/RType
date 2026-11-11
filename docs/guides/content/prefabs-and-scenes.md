# Prefabs and scenes

## Prefabs

A prefab is a named template: an object whose keys are component names.

```json
"explosion": {
    "Transform": {},
    "Sprite": {"texture": "explosions", "frame": [1, 99, 64, 64], "scale": 1.5},
    "SpriteAnimation": {"frame": [1, 99, 64, 64], "frameCount": 10, "stepX": 65, "framesPerSecond": 20},
    "SoundCue": {"sound": "explosion"},
    "ParticleEmitter": {"count": 14, "speed": 260, "lifetime": 0.5, "size": 5, "color": [255, 180, 60, 255]}
}
```

A game has two prefab files:

| File | Contains | Spawned by |
|---|---|---|
| `<game>_server_prefabs.json` | Gameplay entities: players, enemies, bullets, pickups, invisible directors | `serverScene`, `Respawn`, `Weapon`, `Tilemap`, scripts (`world.spawn`, `e.fire`) |
| `<game>_client_prefabs.json` | Appearance of replicated entities, effects, UI elements | Snapshots (through `kinds`), `DeathEffect`, scene `entities`, `ListView` rows |

An entity spawned at run time gets its position from whoever spawns it, so prefabs usually have an empty
`"Transform": {}`.

### Linking a server entity to its look

```json
// server prefab
"pod": { "...": "...", "Replicated": {"kind": 6} }
// game file
"kinds": {"6": "pod"}
// client prefab
"pod": {"Transform": {}, "Sprite": {"texture": "pods", "frame": [...], "scale": 3}}
```

The server and client prefabs do not need the same name, only the `kinds` table links them.

### Overriding components

Wherever a prefab is used in a scene, extra `components` replace or add components for that copy:

```json
{"prefab": "hud_line", "components": {"Transform": {"x": 24, "y": 12}, "Text": {"font": "hud", "size": 28, "color": [80, 200, 255, 255]}}}
```

A prefab with only `"Transform": {}` (R-Type: `hud_line`, `ui_element`) is a blank base used this way.

## Scenes

Client scenes are declared in `scenes` of the game file:

```json
"menu": {
    "music": "title",
    "escapeAction": "quit",
    "systems": ["Parallax", "Pointer", "FocusNavigation", "ButtonStyle", "Audio", "Cleanup"],
    "entities": [
        {"prefab": "star_far", "count": 120, "scatter": true},
        {"prefab": "ui_element", "components": {"...": "..."}}
    ]
}
```

| Key | Meaning |
|---|---|
| `music` | Music key, played while the scene is active |
| `escapeAction` | UI action sent when Escape (or gamepad Select) is pressed |
| `systems` | Client systems, in order ([System reference](../../reference/systems.md)) |
| `entities` | List of entries |

Each entry of `entities`:

| Key | Default | Meaning |
|---|---|---|
| `prefab` | none | Client prefab to spawn. Without it, an empty entity is created |
| `components` | none | Components to add or replace |
| `count` | 1 | Number of copies |
| `scatter` | false | Places each copy at a random position in the world (stars, clouds) |

The server scene (`serverScene`) uses the same entry format with server prefabs. It is spawned when a
simulation starts.

## Switching scenes

| Who | How |
|---|---|
| Online client | Automatically: `menu` -> `lobby` on connection, `room` on join, `game` on start, back on leave |
| Solo client | Actions `play` (to `game`) and `scene:<name>` |
| Both | A button with the action `scene:<name>` |

When a scene is left, all its entities are destroyed. Values in the UI store are kept.
