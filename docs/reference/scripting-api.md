# Scripting reference

This page lists every function a Python script can call. How scripts are loaded and run is explained in
[Scripting architecture](../architecture/scripting.md).

## Conventions

- Positions are in world units, `x` to the right, `y` downward. `x()` and `y()` are the top-left corner.
- Angles are in **degrees**: `0` = right, `90` = down, `180` = left, `270` (or `-90`) = up.
- Speeds are in world units per second. Times are in seconds.
- Hooks receive the entity handle, usually named `e`. The world is `e.world`.

## Hooks

```python
def on_spawn(e): ...              # first tick
def on_update(e, dt): ...         # every tick
def on_damage(e, amount): ...     # health went down by amount
def on_death(e): ...              # killed
def on_contact(e, other): ...     # touched another entity
```

Console script only:

```python
def run(world, ship, text):       # ship is the sender's avatar, or None
    return "text printed in the console"
```

## Entity (`e`)

### State

| Function | Returns |
|---|---|
| `e.alive()` | `True` if the entity exists and is not being destroyed |
| `e.exists()` | `True` if the entity still exists (even if tagged for destruction this tick) |
| `e.age()` | Seconds since the entity was first seen by its script |
| `e.every(name, seconds)` | `True` once every `seconds`. The first call starts the timer and returns `False` |
| `e.number(name, fallback=0.0)` | Number from the `Script.params` of the prefab |
| `e.text(name, fallback="")` | Text from the `Script.params` of the prefab |
| `e.world` | The world object (below) |

Handles accept new attributes, which is how a script stores its per-entity state: `e.phase = 2`.

### Position and movement

| Function | Effect |
|---|---|
| `e.x()`, `e.y()` | Top-left corner |
| `e.width()`, `e.height()` | Size of the collider (0 without collider) |
| `e.center_x()`, `e.center_y()` | Center of the collider |
| `e.vx()`, `e.vy()` | Velocity |
| `e.set_position(x, y)` | Moves the entity |
| `e.set_velocity(vx, vy)` | Sets the velocity |
| `e.set_angle(degrees)` | Sets the rotation, drawn by clients |
| `e.on_ground()` | `True` if the entity stands on a `Solid` entity |
| `e.attach_to(parent, dx=0, dy=0)` | Follows `parent` at that offset |
| `e.detach()` | Stops following |
| `e.attached()` | `True` if attached |
| `e.parent()` | The parent handle, or `None` |

### Health and life

| Function | Effect |
|---|---|
| `e.health()`, `e.max_health()` | Hit points |
| `e.heal(amount)` | Adds health, up to the maximum |
| `e.set_health(points, maximum)` | Sets both values (bosses scaled by difficulty) |
| `e.set_invulnerable(seconds)` | Ignores damage for that time (never shortens a longer shield). `0` removes the shield |
| `e.invulnerable_time()` | Seconds of shield left, `0` without shield |
| `e.kill()` | Kills: death event, `on_death`, death effect on clients, life lost for players |
| `e.destroy()` | Removes silently: no death event, no effect, no score |

### Shooting and spawning

| Function | Effect |
|---|---|
| `e.fire(prefab, angle, speed, dx=0, dy=0)` | Spawns `prefab` centered on the entity (plus offset) and moving at `angle` |
| `e.fire_spread(prefab, count, spread, angle, speed)` | `count` shots spread over `spread` degrees around `angle` |
| `e.fire_at_player(prefab, speed)` | One shot toward the nearest player |
| `e.spawn(prefab, dx=0, dy=0)` | Spawns `prefab` at the entity position plus offset, returns its handle |
| `e.set_weapon(prefab, cooldown, count=1, spread=0)` | Changes the player's weapon |

Shots fired by an entity are owned by it: a kill by the shot credits the shooter's player.
`fire` and `fire_spread` do nothing when the entity limit is reached. `spawn` always spawns: check
`world.can_spawn()` first when spawning many entities.

### Players

| Function | Returns |
|---|---|
| `e.is_player()` | `True` for a player-controlled entity |
| `e.player_slot()` | Seat of the owning player, or `-1` |
| `e.nearest_player()` | `(x, y)` of the center of the nearest player avatar, or `None` |
| `e.angle_to_player()` | Angle toward the nearest player, `180` if there is none |
| `e.set_player_value(name, value)` | Sets a value of `playerValues` for the owner (`weapon`, `charge`...) |
| `e.player_value(name)` | Reads back a value of `playerValues` (0 when the entity has no player) |

### Inputs (player avatars only)

| Function | Returns |
|---|---|
| `e.input_held("fire")` | Seconds the fire button has been held, `0` if released |
| `e.input_held("action")` | `1.0` while action is held, else `0` |
| `e.input_pressed("fire")` | `True` while fire is held |
| `e.input_pressed("action")` | `True` only on the tick action was pressed |
| `a == b` | `True` when two handles point to the same entity |
| `e.input_pressed("bomb")` | `True` on the tick the player finished typing the game's `secretCode` |
| `e.input_released("fire")` | On the tick fire is released: how long it was held. Otherwise `0` |

## World (`e.world`)

### Time and randomness

| Function | Returns |
|---|---|
| `world.time()` | Seconds since the simulation started |
| `world.every(name, seconds)` | Like `e.every`, shared by the whole world |
| `world.reset_timers()` | Clears every world timer |
| `world.random(minimum, maximum)` | Random number in the range |

### Values and rules

| Function | Effect |
|---|---|
| `world.set(name, value)` | Sets a world value (integer) |
| `world.get(name)` | Reads a world value |
| `world.set_label(name, label)` | Sets a labelled value by its label: `world.set_label("phase", "boss")` |
| `world.label(name)` | Reads a labelled value as text |
| `world.rule(name)` | Label of a room rule: `world.rule("difficulty")` returns `"HARD"` |

### Entities and match

| Function | Effect |
|---|---|
| `world.spawn(prefab, x, y)` | Spawns a server prefab, returns its handle |
| `world.can_spawn()` | `False` when the entity limit is reached or no match is playing |
| `world.count_scripted(file)` | Number of living entities running that script |
| `world.players()` | Handles of the living player avatars |
| `world.in_layer(layer)` | Handles of the living entities whose collider is in that layer (`"enemy"`...) |
| `world.players_alive()` | Number of living player avatars |
| `world.match_phase()` | `"waiting"`, `"playing"` or `"over"` |
| `world.end_match()` | Ends the match now (victory) |
| `world.width()`, `world.height()` | World size |

### Output

| Function | Effect |
|---|---|
| `world.play_sound(name)` | Plays a sound of `eventSounds` on every client |
| `world.log(text)` | Prints `[script] text` on the server output |

## Example: a scripted boss phase

```python
import common

def on_spawn(e):
    e.phase = 1

def on_update(e, dt):
    if e.phase == 1 and common.health_ratio(e) < 0.5:
        e.phase = 2
        e.world.play_sound("warning")
    if e.every("ring", 2.0 if e.phase == 1 else 1.0):
        common.ring(e, "boss_orb", 12, 300.0)

def on_death(e):
    e.world.set("shake", 14)
```

`common.py` in `game/scripts/` contains helpers shared by the R-Type scripts (`sine`, `bounce`,
`follow_player_y`, `cooldown`, `health_ratio`, `ring`).
