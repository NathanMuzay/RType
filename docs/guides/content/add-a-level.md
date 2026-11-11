# Add a level or a boss

Levels are Python modules run by `game/scripts/campaign.py` (states) and `spawner.py` (waves,
random enemies, events), attached to the invisible
`director` entity of the server scene.

## A new stage

Create `game/scripts/stage4.py`:

```python
DURATION = 80.0                                   # seconds before the boss arrives
BOSS = ("boss_warship", 2000.0, 324.0)            # prefab, x, y

RANDOM = (0.8, 1.5, [                             # min / max delay between random spawns
    ("pod", 3, 0.0, 1000.0),                      # prefab, weight, min y, max y
    ("hopper", 2, 200.0, 800.0),
])

WAVES = [                                         # time, prefab, count, x, y, spacing x, spacing y
    (5.0, "heavy", 2, 1950.0, 300.0, 0.0, 400.0),
    (20.0, "splitter", 3, 1950.0, 200.0, 220.0, 280.0),
]


def events(world, time, director):                # optional: scripted moments
    if time >= 40.0 and "ambush" not in director.done:  # director.done is emptied at each stage start
        director.done.add("ambush")
        for index in range(4):
            pod = world.spawn("pod", -150.0, 150.0 + index * 220.0)
            pod.set_velocity(420.0, 0.0)
```

Then add it to the list in `campaign.py`:

```python
import stage4
STAGES = (stage1, stage2, stage3, stage4)
```

The campaign handles the rest: the "STAGE N" intro, the progress bar, the boss health bar, the
transition to the next stage and the victory after the last one.

### A boss present during the whole stage

A stage can define `take_boss(director)`: when the boss phase starts, the campaign uses the entity
it returns instead of spawning `BOSS`. Stage 3 spawns the Bydo Vessel from `events`, keeps it
invulnerable while the stage runs, and hands it over as the boss:

```python
def take_boss(director):
    vessel = getattr(director, "vessel", None)
    if vessel is None or not vessel.alive():
        prefab, x, y = BOSS
        vessel = director.world.spawn(prefab, x, y)
    return vessel
```

The Vessel's script (`vessel.py`) switches to its fight when the world value `phase` becomes
`boss`. Its prefab has the param `bombProof`, so the secret bomb ignores it until then.

## Projectiles of each stage

Enemies fire through the helpers of `common.py`, which pick the projectile of the current stage:

| Helper | Stage 1 | Stage 2 | Stage 3 |
|---|---|---|---|
| `common.shoot(e, angle, speed)`, `common.shoot_at_player(e, speed)` | `spore` (splits in two after 0.9 s) | `laser_shot` (in pairs) | `homing_missile` (homes for 1.4 s) |
| `common.spread(...)`, `common.stage_ring(...)` | `spore` | `laser_shot` | `missile` (no homing) |

Change the tables `STAGE_SHOTS` and `STAGE_RING_SHOTS` in `common.py` to give a new stage its own
projectiles.

## A new boss

A boss is an enemy whose script changes behaviour with its health. `boss_common.py` gives:

- `enter(e)`: flies in from the right and stops at the `stop_x` param, returns `True` once arrived;
- `phase(e, (0.6, 0.25))`: 1 above 60 % health, 2 above 25 %, 3 below;
- `burst(e, name, size, gap, every_min, every_max, shoot)`: bursts of shots.

```python
import boss_common
import common


def on_update(e, dt):
    if not boss_common.enter(e):
        return
    if boss_common.phase(e, (0.5,)) == 1:
        common.bounce(e, 150.0)
        if e.every("fan", 2.0):
            e.fire_spread("boss_orb", 7, 80.0, common.LEFT, 400.0)
    elif e.every("ring", 1.2):
        common.ring(e, "enemy_bullet", 16, 380.0)
```

Declare its prefab like any enemy (with `"destroyOnHit": false` in `ContactDamage` and no
`DespawnOffscreen`) and reference it in the `BOSS` line of a stage.
