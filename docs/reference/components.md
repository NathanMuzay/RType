# Component reference

Every component that can be written in a prefab or a scene entity. Field names are the JSON keys read by the
loaders (`engine/scene/src/EngineComponentLoaders*.cpp`). A field with a default value is optional.

**Side**: *S* = used in server prefabs (simulation), *C* = used in client prefabs and scenes, *S/C* = both.

Common value formats:

| Format | Example |
|---|---|
| color | `[r, g, b, a]`, each 0 to 255: `[80, 200, 255, 255]` |
| frame | `[x, y, width, height]` in pixels of the texture: `[66, 0, 33, 17]` |
| layer list | `["player", "enemy"]`, names from the game's `layers` |

## Physics

| Component | Side | Fields | Description |
|---|---|---|---|
| `Transform` | S/C | `x` = 0, `y` = 0 | Position of the top-left corner, world units |
| `Velocity` | S/C | `x` = 0, `y` = 0 | Speed in world units per second, applied by `Movement` |
| `Collider` | S | `width`, `height`, `layer`, `hits` = [] | Box used for collisions. `layer` is what the entity is, `hits` what it can touch |
| `Bounded` | S | `minX`, `minY`, `maxX`, `maxY` | Keeps the entity inside this rectangle (`Bounds` system) |
| `DespawnOffscreen` | S | `minX`, `minY`, `maxX`, `maxY` | Destroys the entity when it leaves this rectangle (`Offscreen` system) |
| `Solid` | S | - | Wall: pushes out entities that have `Pushable`, and counts as ground for `on_ground()` |
| `Pushable` | S | - | Can be pushed out of `Solid` entities |
| `Gravity` | S | `force` | Adds `force` to the vertical velocity every second |
| `Bounce` | S | `restitution` = 1 | Bounces on its `Bounded` limits and on solids, keeping `restitution` of the speed |
| `Rotation` | S/C | `degrees` = 0 | Rotation around the center. Sent to clients in the `visual` byte |

`Attachment` (stick to a parent entity) has no JSON loader: scripts create it with `e.attach_to(...)`.

## Combat

| Component | Side | Fields | Description |
|---|---|---|---|
| `Health` | S | `points` | Hit points. The entity dies at 0 |
| `ContactDamage` | S | `amount`, `destroyOnHit` = false | Damage dealt to the entities it hits. `destroyOnHit` destroys the source too (bullets) |
| `Invulnerable` | S | `seconds` | Ignores damage for that time |
| `ScoreValue` | S | `points` | Points given to the killer's player |
| `Weapon` | S | `prefab`, `cooldown`, `offsetX` = 0, `offsetY` = 0, `count` = 1, `spread` = 0 | Fires `prefab` while the player holds fire. `count` bullets spread over `spread` degrees |

`Destroyed`, `Owner` and `HitFlash` are added by the engine at run time.

## Gameplay

| Component | Side | Fields | Description |
|---|---|---|---|
| `PlayerControl` | S | `speed` | Moves the entity with the player's buttons, and gives scripts access to the inputs |
| `Lives` | S | `count` | Lives of a player entity |
| `Score` | S | - | Score of a player entity |
| `Respawn` | S | `prefab`, `delay` = 0, `invulnerability` = 0, `x`, `y`, `spacingX` = 0, `spacingY` = 0 | Spawns the player's avatar, and again after its death while lives remain. Player N appears at `x + N*spacingX`, `y + N*spacingY` |
| `Match` | S | `restartDelay` = 0 | Follows the match state; when everyone is out, a new round starts after the delay |
| `Persistent` | S | - | Survives the world reset between rounds |
| `Script` | S | `file`, `params` = {} | Python behaviour, see [Scripting](scripting-api.md) |
| `Tilemap` | S | `rows`, `legend`, `tileSize`, `speed` | Scrolling level made of text rows, see [Tilemaps](../guides/content/tilemaps.md) |
| `Replicated` | S | `kind` | Sent to clients. `kind` selects the client prefab in the game's `kinds` table |

`Player`, `PlayerIndex` and `PlayerValues` are added by the simulation for each player.

## Presentation

| Component | Side | Fields | Description |
|---|---|---|---|
| `Sprite` | C | `texture`, `frame`, `scale` = 1 | Draws a part of a texture |
| `SpriteAnimation` | C | `frame`, `frameCount`, `stepX`, `framesPerSecond`, `loop` = false | Moves the frame `stepX` pixels to the right `frameCount` times |
| `TiltAnimation` | C | `frame`, `row` = 0, `flatColumn`, `maxTilt`, `framesPerSecond` | Picks a frame from the vertical speed (ship leaning up or down) |
| `RectShape` | C | `width`, `height`, `color`, `outline`, `thickness` = 1 | Filled rectangle (UI panels, bars, stars). With `outline`, a border of that color is drawn |
| `Text` | C | `font`, `size`, `color`, `content` = "", `offsetX` = 0, `offsetY` = 0 | Draws text |
| `Parallax` | C | `speed`, `wrapWidth` | Scrolls to the left and wraps around. Needs a `RectShape` (its width is used) |
| `Foreground` | C | - | Drawn above the world (HUD, menus) and not moved by the screen shake |
| `SoundCue` | C | `sound` | Plays a sound when the entity appears |
| `DeathEffect` | C | `prefab` | Client prefab spawned where the entity dies (explosion) |
| `ParticleEmitter` | C | `count`, `speed`, `lifetime`, `size` = 4, `color` | Throws particles once when the entity appears |
| `Lifetime` | C | `seconds` | Destroys the entity after that time |
| `MusicWhen` | C | `key`, `value`, `music` | Plays `music` while UI value `key` equals `value` |
| `PlayerLabel` | C | `font`, `format`, `size`, `offsetX` = 0, `offsetY` = 0, `colors` | Label above each player's avatar (`"P{slot}"`), one color per player |
| `StatusText` | C | `format`, `slot`, `visibleWhen` | Text with `{slot}`, `{score}`... of one player. `visibleWhen` is a match phase: `waiting`, `playing` or `over` |

## Interface

| Component | Side | Fields | Description |
|---|---|---|---|
| `Button` | C | `action`, `width`, `height`, `normal`, `hover`, `pressed`, `hoverSound`, `clickSound` | Clickable area. Colors are applied to the `RectShape`. `normalFrame`, `hoverFrame`, `pressedFrame` do the same for a `Sprite` |
| `Focusable` | C | `order` | Can be selected with the keyboard or gamepad, in `order` |
| `TextField` | C | `name`, `maxLength`, `width`, `height`, `masked` = false, `placeholder`, `submit` | Text input. Its text is stored as UI value `field.<name>`. `submit` is the UI action sent when Enter is pressed (R-Type chat: `send_chat`) |
| `Choice` | C | `name`, `options` | Left/right selector. Its value is stored as `field.<name>` |
| `VisibleWhen` | C | `key`, `value`, `unless` = none | Shown only while UI value `key` equals `value`, except while `unless` (`{key, value}`) also matches |
| `Hidden` | C | - | Not drawn (set and removed by `Visibility`) |
| `BoundText` | C | `format` | Text built from UI values: `"STAGE {world.stage}"` |
| `ProgressBar` | C | `key`, `width`, `max` = 1, `vertical` = false | Resizes the `RectShape` to `value / max` |
| `FadeWhen` | C | `key`, `value`, `seconds` = 0 | Fades the `RectShape` in over `seconds` while `key` equals `value`, and out otherwise |
| `SettingSlider` | C | `setting`, then `min`, `max`, `step` or `options`, `direction` = 0 | With `direction` 0: left/right on the focused row changes the player setting `setting.<setting>` (numbers stop at the ends, options wrap). With `-1` or `1`: a clickable arrow button that lowers or raises it. See [Controls](../guides/content/controls.md#settings-screen) |
| `KeyBind` | C | `action`, `device` = keyboard, `prompt` = "..." | Enter or click, then the next key (or gamepad button with `"device": "gamepad"`) becomes the binding of `action`: `up`, `down`, `left`, `right`, `fire`, `action` |
| `ListView` | C | `source`, `spacing`, `maxRows`, `parts` | Spawns one row per entry of the UI list `source` (rooms, members). Each row is made of `parts`: `{prefab, offsetX = 0, offsetY = 0}`, rows are `spacing` apart |

## Example

A complete R-Type enemy, server side and client side:

```json
// rtype_server_prefabs.json
"bydo_sine": {
    "Transform": {}, "Velocity": {"x": 0, "y": 0},
    "Collider": {"width": 66, "height": 72, "layer": "enemy", "hits": ["player"]},
    "Health": {"points": 1},
    "ContactDamage": {"amount": 25, "destroyOnHit": true},
    "ScoreValue": {"points": 100},
    "Script": {"file": "bydo.py", "params": {"pattern": "sine", "amplitude": 150, "frequency": 3}},
    "DespawnOffscreen": {"minX": -500, "minY": -400, "maxX": 2500, "maxY": 1480},
    "Replicated": {"kind": 3}
}
```

The client draws kind `3` with the prefab named in `kinds` (`"3": "bydo"`) of `rtype_client_prefabs.json`.
