"""Hopper: waves up and down and shoots straight at the closest player."""

import common


def on_spawn(e):
    common.start_moving(e, -300.0)


def on_update(e, dt):
    common.sine(e, 220.0, 2.0)
    if common.cooldown(e, "shot", 2.0, 3.5):
        common.shoot_at_player(e, 520.0)
