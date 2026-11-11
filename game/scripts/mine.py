"""Mine: drifts slowly and blows up in a ring of bullets when a player gets close or shoots it."""

import common

TRIGGER_DISTANCE = 220.0


def on_spawn(e):
    common.start_moving(e, -120.0)


def on_update(e, dt):
    common.sine(e, 40.0, 2.5)
    target = e.nearest_player()
    if target is None:
        return
    dx = target[0] - e.center_x()
    dy = target[1] - e.center_y()
    if dx * dx + dy * dy < TRIGGER_DISTANCE * TRIGGER_DISTANCE:
        explode(e)
        e.kill()


def on_death(e):
    explode(e)


def explode(e):
    if getattr(e, "exploded", False):
        return
    e.exploded = True
    common.stage_ring(e, 12, 340.0)
