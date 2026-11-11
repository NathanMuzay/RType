"""Chaser: slides to the height of the closest player, then dashes forward once aligned."""

import common

ALIGNED = 30.0
DASH_SPEED = -950.0


def on_spawn(e):
    common.start_moving(e, -180.0)
    e.dashing = False


def on_update(e, dt):
    if e.dashing:
        return
    gap = common.follow_player_y(e, 260.0)
    if e.age() > 0.8 and abs(gap) < ALIGNED and e.x() < 1700.0:
        e.dashing = True
        e.set_velocity(DASH_SPEED, 0.0)
