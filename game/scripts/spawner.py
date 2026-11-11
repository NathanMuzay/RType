"""Spawner: brings in the content of the stage being played, and clears it.

A stage module gives:
    DURATION  seconds before the boss arrives
    BOSS      (prefab, x, y)
    RANDOM    (min interval, max interval, [(prefab, weight, min y, max y), ...])
    WAVES     [(time, prefab, count, x, y, spacing x, spacing y), ...]
    events(world, time, director)  optional scripted moments
    take_boss(director)            optional: a boss that already exists, instead of spawning BOSS
Stages add to `director.leftovers` what must disappear when the stage is cleared (a tilemap, the
Vessel). The difficulty and the number of players scale the spawn delay, the size of the waves and
the health of the enemies (see difficulty.py).
"""

import difficulty

SPAWN_X = 1950.0
CLEARED_LAYERS = ("enemy", "enemy_missile", "terrain", "pickup")


def start(d, stage):
    d.stage = stage
    d.waves = sorted(stage.WAVES)
    d.next_wave = 0
    d.next_random = 0.0
    d.done = set()


def update(d, time):
    """Spawns the waves that are due, a random enemy from time to time, and the stage events."""
    world = d.world
    while d.next_wave < len(d.waves) and d.waves[d.next_wave][0] <= time:
        spawn_wave(world, d.waves[d.next_wave])
        d.next_wave += 1
    if time >= d.next_random:
        minimum, maximum, table = d.stage.RANDOM
        delay = difficulty.get(world, "spawn_delay") / difficulty.for_players(world, "spawn_rate")
        d.next_random = time + world.random(minimum, maximum) * delay
        spawn_random(world, table)
    if hasattr(d.stage, "events"):
        d.stage.events(world, time, d)


def boss(d):
    if hasattr(d.stage, "take_boss"):
        return d.stage.take_boss(d)
    prefab, x, y = d.stage.BOSS
    return d.world.spawn(prefab, x, y)


def clear(d):
    """Removes enemies, their shots, pick-ups, terrain and the stage leftovers."""
    for layer in CLEARED_LAYERS:
        for entity in d.world.in_layer(layer):
            entity.destroy()
    for entity in getattr(d, "leftovers", []):
        if entity.exists():
            entity.destroy()
    d.leftovers = []


def spawn_wave(world, wave):
    _, prefab, count, x, y, spacing_x, spacing_y = wave
    for index in range(difficulty.wave_size(world, count)):
        if world.can_spawn():
            difficulty.toughen(world.spawn(prefab, x + index * spacing_x, y + index * spacing_y))


def spawn_random(world, table):
    if not world.can_spawn() or not table:
        return
    roll = world.random(0.0, sum(entry[1] for entry in table))
    for prefab, weight, low, high in table:
        roll -= weight
        if roll <= 0.0:
            difficulty.toughen(world.spawn(prefab, SPAWN_X, world.random(low, high)))
            return
