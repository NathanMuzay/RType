# System reference

Systems are named in the game file: `serverSystems` for the simulation, `systems` of each scene for the client,
and `renderSystems`. They run **in the listed order**, once per tick (server) or per frame (client).

A system written as a string takes no options. A system with options is written as an object:
`{"system": "Shake", "key": "world.shake", "strength": 1.0}`.

## Simulation systems

| Name | Reads | Does |
|---|---|---|
| `PlayerControl` | `PlayerControl`, `Velocity` | Sets the velocity from the player's buttons, updates fire and action timings for scripts |
| `Weapon` | `Weapon`, `PlayerControl` | Spawns the weapon's prefab while fire is held, respecting the cooldown |
| `Invulnerability` | `Invulnerable` | Counts down and removes the component |
| `Movement` | `Transform`, `Velocity` | Moves entities |
| `Gravity` | `Gravity`, `Velocity` | Accelerates entities downward |
| `Tilemap` | `Tilemap` | Scrolls the map and spawns tiles as their column enters the world |
| `Attachment` | `Attachment` | Keeps attached entities at their offset from the parent |
| `Solid` | `Pushable`, `Solid`, `Collider` | Pushes movers out of walls |
| `Bounce` | `Bounce`, `Velocity` | Bounces on limits and walls |
| `Bounds` | `Bounded` | Clamps positions inside the rectangle |
| `Collision` | `Collider` | Detects overlaps between layers that can hit each other, pushes `CollisionEvent` |
| `Damage` | `ContactDamage`, `Health` | Applies contact damage, flashes the target, pushes `DeathEvent` |
| `HitFlash` | `HitFlash` | Ends the hit flash |
| `Script` | `Script` | Runs the Python hooks |
| `Lives` | `Lives` | Removes a life when a player's avatar dies |
| `Score` | `Score`, `ScoreValue` | Credits the killer's player |
| `Offscreen` | `DespawnOffscreen` | Destroys entities outside their rectangle |
| `Match` | `Match` | Waiting, playing, over, new round |
| `Respawn` | `Respawn` | Spawns avatars at start and after a death |

R-Type order:

```json
"serverSystems": ["PlayerControl", "Weapon", "Invulnerability", "Movement", "Tilemap", "Attachment", "Solid",
                  "Bounds", "Collision", "Damage", "HitFlash", "Script", "Lives", "Score", "Offscreen",
                  "Match", "Respawn"]
```

Skyhop order (no weapons, gravity handled in its script):

```json
"serverSystems": ["Movement", "Collision", "Damage", "Script", "Offscreen", "Match", "Respawn"]
```

**Ordering rules.** Movement before `Solid` and `Bounds` (correct positions after moving), `Collision` before
`Damage` and `Script` (they read collision events), `Damage` before `Lives` and `Score` (they read deaths),
`Match` before `Respawn`.

## Client systems

| Name | Options | Does |
|---|---|---|
| `Pointer` | - | Mouse hover and click on `Button` entities |
| `FocusNavigation` | - | Keyboard and gamepad navigation between `Focusable` entities |
| `ButtonStyle` | - | Applies the normal, hover and pressed colors or frames, plays button sounds |
| `TextField` | - | Text input in `TextField` entities |
| `Choice` | - | Left and right on `Choice` entities |
| `SettingSlider` | - | Left and right on `SettingSlider` entities |
| `KeyBind` | - | Captures the next key or button for a `KeyBind` entity. Put it after `FocusNavigation` |
| `ListView` | - | Rebuilds the rows of a `ListView` when its list changes |
| `BoundText` | - | Fills `BoundText` formats with UI values |
| `Visibility` | - | Shows and hides `VisibleWhen` entities |
| `ProgressBar` | - | Resizes progress bars |
| `Fade` | - | Fades `FadeWhen` rectangles |
| `Parallax` | - | Scrolls and wraps background layers |
| `SpriteAnimation` | - | Advances animations |
| `TiltAnimation` | - | Leans ships with their vertical speed |
| `Anchor` | - | Keeps UI parts at their place relative to a parent |
| `StatusText` | - | Fills per-player texts |
| `PlayerLabel` | - | Draws `P1`, `P2`... above avatars (can be turned off in settings) |
| `DeathEffect` | - | Spawns the death prefab of destroyed entities |
| `Particles` | - | Emits and moves particles |
| `Lifetime` | - | Destroys short-lived entities |
| `Movement` | - | Moves client-only entities that have a `Velocity` |
| `HitFlash` | - | Ends hit flashes |
| `Music` | - | Applies `MusicWhen` |
| `Shake` | `key`, `strength` = 1 | Shakes the camera by the UI value `key` |
| `Audio` | - | Plays `SoundCue` sounds |
| `Cleanup` | - | Removes destroyed entities |

## Plugin systems

| Name | Plugin | Options | Does |
|---|---|---|---|
| `Wobble` | `wobble` | `texture`, `amplitude` = 12, `speed` = 3 | Rocks the sprites of `texture` (all sprites without it) |

## Render system

| Name | Options | Does |
|---|---|---|
| `Render` | `background` (color) | Clears the screen and draws the world, then the foreground, see [Rendering](../architecture/rendering.md) |

## Adding a system

See [Add a system](../guides/engine/add-a-system.md).
