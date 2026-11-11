# Create a game from scratch

This tutorial builds a new game on the engine without writing any C++. It follows **Skyhop** (`game2/`), a
solo one-button game: a plane flies between rocks, a tap makes it climb, gravity pulls it down.
Every snippet below comes from the real files.

## 1. Create the folders

```text
game2/
├── data/
│   ├── skyhop_game.json             main file
│   ├── skyhop_server_prefabs.json   gameplay entities
│   ├── skyhop_client_prefabs.json   appearance and UI
│   └── skyhop_settings.json         controls and volumes
├── scripts/skyhop/                  Python behaviours
└── assets/skyhop/                   images, sounds, music, font
```

The data files MUST be named `<game>_game.json`, `<game>_...`: the launcher finds the game by its name.
Scripts and assets are put in a subfolder named after the game so they never collide with another game's files
once copied next to the executables.

## 2. Declare the game in CMake

In the root `CMakeLists.txt`:

```cmake
engine_add_game(skyhop
    LOCAL skyhop
    DATA game2/data
    SCRIPTS game2/scripts
    ASSETS game2/assets)
```

| Keyword | Builds |
|---|---|
| `LOCAL <exe>` | A solo executable, no server ([Solo mode](../../architecture/solo-mode.md)) |
| `SERVER <exe>` and `CLIENT <exe>` | An online server and client, like R-Type |

Use the three together for a game playable both ways.

## 3. Write the game file

Start with the general keys ([Game file reference](../../reference/game-file.md)):

```json
{
  "world": {"width": 1920, "height": 1080},
  "window": {"width": 1280, "height": 720, "title": "Skyhop"},
  "tickRate": 60,
  "network": { "...": "copy the block of rtype_game.json" },
  "files": {"serverPrefabs": "skyhop_server_prefabs.json", "clientPrefabs": "skyhop_client_prefabs.json",
            "settings": "skyhop_settings.json", "scripts": "../scripts/skyhop"},
  "layers": ["player", "obstacle", "gate"],
  "worldValues": [{"name": "phase", "labels": ["ready", "flying", "over"]}, {"name": "score"},
                  {"name": "best"}, {"name": "restart"}],
  "playerValues": [],
  "rules": [],
  "eventSounds": ["flap", "score", "crash"],
  "playerPrefab": "player",
  "serverScene": [{"prefab": "match"}, {"prefab": "director"}],
  "serverSystems": ["Movement", "Collision", "Damage", "Script", "Offscreen", "Match", "Respawn"],
  "startScene": "menu"
}
```

The `network` block is required even in solo mode, because it sets the entity limit.

## 4. Gameplay entities (server prefabs)

```json
{
  "match": {"Match": {"restartDelay": 1.0}},
  "player": {"Lives": {"count": 1}, "Score": {},
             "Respawn": {"prefab": "plane", "delay": 0.0, "invulnerability": 0.0, "x": 420, "y": 420, "spacingY": 0}},
  "director": {"Script": {"file": "director.py"}},
  "plane": {"Transform": {}, "Velocity": {},
            "Collider": {"width": 92, "height": 70, "layer": "player", "hits": ["gate"]},
            "PlayerControl": {"speed": 0}, "ContactDamage": {"amount": 1, "destroyOnHit": false},
            "Replicated": {"kind": 1}, "Script": {"file": "plane.py"}},
  "gate": {"Transform": {}, "Velocity": {"x": -380, "y": 0},
           "Collider": {"width": 16, "height": 1080, "layer": "gate"},
           "Health": {"points": 1}, "ScoreValue": {"points": 1},
           "DespawnOffscreen": {"minX": -400, "minY": -1200, "maxX": 2600, "maxY": 2200},
           "Script": {"file": "gate.py"}}
}
```

What each piece does:

- `player` is the player entity; `Respawn` creates its avatar, the `plane`.
- The plane has `PlayerControl` with speed 0: it receives the buttons, but its script decides the movement.
- An invisible `gate` sits between two rocks. The plane's `ContactDamage` kills it when crossing, which
  triggers `gate.py`'s `on_death` and gives a point.
- `Replicated` with `kind: 1` makes the plane visible on the client.
- `director` is an entity without `Transform`: it is never cleared and runs the game rules.

## 5. Behaviours (scripts)

`scripts/skyhop/plane.py` (shortened):

```python
FLAP_SPEED = -760.0
GRAVITY = 2300.0

def on_update(e, dt):
    tap = tapped(e)
    if e.world.label("phase") == "ready":
        e.set_velocity(0.0, 0.0)
        if tap:
            e.world.set_label("phase", "flying")
            flap(e)
        return
    if tap:
        flap(e)
    e.set_velocity(0.0, e.vy() + GRAVITY * dt)

def flap(e):
    e.set_velocity(0.0, FLAP_SPEED)
    e.world.play_sound("flap")

def on_contact(e, other):
    if other.number("deadly", 0.0) > 0.0 and e.world.label("phase") == "flying":
        crash(e)
```

`scripts/skyhop/gate.py`:

```python
def on_death(e):
    world = e.world
    world.play_sound("score")
    world.set("score", world.get("score") + 1)
    world.set("best", max(world.get("best"), world.get("score")))
```

`scripts/skyhop/director.py` spawns a pair of rocks every `SPACING / SPEED` seconds while the phase is `flying`,
and resets the run when the plane asks for it.

## 6. Appearance (client prefabs and resources)

In the game file, declare the resources and the kinds:

```json
"textures": {"plane": "skyhop/sprites/plane.png", "rock": "skyhop/sprites/rockGrass.png"},
"fonts": {"hud": "skyhop/fonts/hud.ttf"},
"sounds": {"flap": "skyhop/sounds/flap.ogg", "score": "skyhop/sounds/score.ogg"},
"music": {"main": "skyhop/music/main.wav"},
"kinds": {"1": "plane", "2": "rock", "3": "rock_down"},
"renderSystems": [{"system": "Render", "background": [120, 190, 230, 255]}]
```

In `skyhop_client_prefabs.json`:

```json
"plane": {"Transform": {},
          "Sprite": {"texture": "plane", "frame": [0, 0, 88, 73], "scale": 1.3},
          "SpriteAnimation": {"frame": [0, 0, 88, 73], "frameCount": 3, "stepX": 88, "framesPerSecond": 14, "loop": true},
          "DeathEffect": {"prefab": "crash"}}
```

## 7. Menu and HUD (scenes)

The solo driver needs a `menu` with a `play` button and a `game` scene:

```json
"scenes": {
  "menu": {
    "music": "title", "escapeAction": "quit",
    "systems": ["Parallax", "Pointer", "FocusNavigation", "ButtonStyle", "SpriteAnimation", "Audio", "Cleanup"],
    "entities": [
      {"prefab": "ui", "components": {
        "Transform": {"x": 750, "y": 520},
        "RectShape": {"width": 420, "height": 100, "color": [232, 151, 52, 255]},
        "Text": {"font": "hud", "size": 52, "color": [255, 255, 255, 255], "content": "PLAY", "offsetX": 150, "offsetY": 22},
        "Button": {"action": "play", "width": 420, "height": 100, "normal": [232, 151, 52, 255],
                   "hover": [250, 190, 70, 255], "pressed": [190, 110, 30, 255], "clickSound": "click"},
        "Focusable": {"order": 0}}}
    ]
  },
  "game": {
    "music": "main", "escapeAction": "pause",
    "systems": ["SpriteAnimation", "DeathEffect", "BoundText", "Visibility", "Movement", "Particles", "Audio", "Cleanup"],
    "entities": [
      {"prefab": "ui", "components": {"Transform": {"x": 940, "y": 40},
        "Text": {"font": "hud", "size": 110, "color": [255, 255, 255, 255]},
        "BoundText": {"format": "{world.score}"}, "Foreground": {}}}
    ]
  }
}
```

More in [UI and HUD](ui-and-hud.md) and [Prefabs and scenes](prefabs-and-scenes.md).

## 8. Build and run

```sh
cmake --build build --parallel "$(nproc)"
./build/bin/skyhop
```

## Checklist

- [ ] Every `kind` of a server prefab has an entry in `kinds`, and that client prefab exists.
- [ ] Every texture, sound and font key used by a prefab is declared in the game file.
- [ ] Every name in `Collider.hits` is in `layers`.
- [ ] Every sound used by `world.play_sound` is in `eventSounds`.
- [ ] Every value set by `world.set` is in `worldValues`.
- [ ] `serverSystems` contains the systems your components need (`Movement` for `Velocity`...).
- [ ] Assets are free to use, and their license file is in the assets folder ([Assets](assets.md)).

When something is missing, the engine stops at load time with a message naming the key, or prints a
`[script]` error: see [Troubleshooting](../../getting-started/troubleshooting.md).
