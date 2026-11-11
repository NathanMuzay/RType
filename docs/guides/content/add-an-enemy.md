# Add an enemy

No C++ and no recompilation: an enemy is a prefab (JSON) plus a behaviour script (Python).
See the [scripting reference](../../reference/scripting-api.md) for every available command.

## 1. Write the behaviour

Create `game/scripts/diver.py`:

```python
import common


def on_spawn(e):
    common.start_moving(e, -200.0)
    e.diving = False


def on_update(e, dt):
    if not e.diving and e.x() < 1300.0:
        e.diving = True
        e.set_velocity(-650.0, 300.0)
    if common.cooldown(e, "shot", 1.5, 2.5):
        e.fire_at_player("enemy_bullet", 500.0)
```

`common.py` holds shared helpers (`start_moving`, `sine`, `bounce`, `follow_player_y`, `cooldown`,
`ring`...).

## 2. Declare the server prefab

In `game/data/rtype_server_prefabs.json`:

```json
"diver": {
  "Transform": {}, "Velocity": { "x": 0, "y": 0 },
  "Collider": { "width": 80, "height": 72, "layer": "enemy", "hits": ["player"] },
  "Health": { "points": 2 },
  "ContactDamage": { "amount": 25, "destroyOnHit": true },
  "ScoreValue": { "points": 150 },
  "Script": { "file": "diver.py" },
  "DespawnOffscreen": { "minX": -500, "minY": -400, "maxX": 2500, "maxY": 1480 },
  "Replicated": { "kind": 35 }
}
```

`kind` is the number sent on the network to tell the client which sprite to draw. Use a number not yet in
the `kinds` table of `rtype_game.json` (R-Type uses 1 to 34).

## 3. Declare how the client draws it

In `game/data/rtype_game.json`, map the kind to a client prefab (`"kinds": { "35": "diver" }`) and
register the sprite sheet in `textures`. In `game/data/rtype_client_prefabs.json`:

```json
"diver": {
  "Transform": {},
  "Sprite": { "texture": "chasers", "frame": [1, 3, 32, 29], "scale": 2.5 },
  "SpriteAnimation": { "frame": [1, 3, 32, 29], "frameCount": 8, "stepX": 33, "framesPerSecond": 10, "loop": true },
  "DeathEffect": { "prefab": "explosion" }
}
```

## 4. Make it appear

Add it to a stage script (`game/scripts/stage2.py`), either in `WAVES` or in the `RANDOM` table.
See [Add a level](add-a-level.md).
