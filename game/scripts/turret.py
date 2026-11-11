"""Turret: enters, parks near the right edge, fires aimed bursts, then leaves.

Params: stop_x (where it parks), stay (seconds before leaving).
"""

import common
import difficulty

BURST_SIZE = 3
BURST_GAP = 0.15


def on_spawn(e):
    common.start_moving(e, -260.0)
    e.parked_at = None
    e.burst_left = 0


def on_update(e, dt):
    if e.parked_at is None:
        if e.x() <= e.number("stop_x", 1500.0):
            e.parked_at = e.age()
            e.set_velocity(0.0, 0.0)
        return
    if e.age() - e.parked_at > e.number("stay", 8.0):
        e.set_velocity(-260.0, 0.0)
        return
    if e.burst_left == 0 and common.cooldown(e, "burst", 1.8, 2.6):
        e.burst_left = BURST_SIZE
    if e.burst_left > 0 and e.every("burst_gap", BURST_GAP):
        e.fire_at_player(common.shot(e), difficulty.speed(e, 600.0))
        e.burst_left -= 1
