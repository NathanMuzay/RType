"""Skyhop director: spawns rock pairs while the plane flies and resets the run on demand.

World values: phase (ready, flying, over), score, best.
"""

SPAWN_X = 1960.0
GROUND_Y = 909.0
ROCK_WIDTH = 280.8
ROCK_HEIGHT = 621.4
HITBOX_WIDTH = 90.0
GAP = 300.0
MIN_TOP = 140.0
MAX_TOP = 500.0
SPEED = 380.0
SPACING = 700.0


def on_spawn(d):
    d.objects = []
    d.next_spawn = 0.0
    d.world.set_label("phase", "ready")


def on_update(d, dt):
    world = d.world
    if world.get("restart") > 0:
        world.set("restart", 0)
        restart(d)
    phase = world.label("phase")
    if phase == "over":
        for item in d.objects:
            if item.exists():
                item.set_velocity(0.0, 0.0)
        return
    if phase != "flying":
        return
    d.next_spawn -= dt
    if d.next_spawn <= 0.0:
        spawn_pair(d)
        d.next_spawn = SPACING / SPEED


def restart(d):
    for item in d.objects:
        if item.exists():
            item.destroy()
    d.objects = []
    d.next_spawn = 0.0
    d.world.set("score", 0)
    d.world.set_label("phase", "ready")


def spawn_pair(d):
    world = d.world
    top = world.random(MIN_TOP, MAX_TOP)
    bottom = top + GAP
    hitbox_x = SPAWN_X + (ROCK_WIDTH - HITBOX_WIDTH) / 2.0
    pieces = (
        ("rock_down", SPAWN_X, top - ROCK_HEIGHT),
        ("rock", SPAWN_X, bottom),
        ("hitbox", hitbox_x, top - ROCK_HEIGHT),
        ("hitbox", hitbox_x, bottom),
        ("gate", SPAWN_X + ROCK_WIDTH / 2.0, 0.0),
    )
    d.objects = [item for item in d.objects if item.exists()]
    for prefab, x, y in pieces:
        if world.can_spawn():
            item = world.spawn(prefab, x, y)
            item.set_velocity(-SPEED, 0.0)
            d.objects.append(item)
