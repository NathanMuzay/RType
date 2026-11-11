"""Splitter: a slow ship that breaks into three small fast ones when destroyed."""

import common

PIECES = ((-380.0, -220.0), (-420.0, 0.0), (-380.0, 220.0))


def on_spawn(e):
    common.start_moving(e, -160.0)


def on_update(e, dt):
    common.sine(e, 90.0, 1.5)


def on_death(e):
    for vx, vy in PIECES:
        piece = e.spawn("splitter_small", e.width() / 4.0, e.height() / 4.0)
        piece.set_velocity(vx, vy)
