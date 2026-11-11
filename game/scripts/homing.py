"""Homing missile: stage 3 projectile. Turns toward the closest player for a short while, then flies straight.

The sprite points left, so its rotation is the heading minus 180 degrees.
"""

import math

import difficulty

HOMING_SECONDS = 1.0
TURN_SPEED = 110.0


def on_update(e, dt):
    heading = math.degrees(math.atan2(e.vy(), e.vx()))
    target = e.nearest_player()
    if target is not None and e.age() < HOMING_SECONDS:
        wanted = math.degrees(math.atan2(target[1] - e.center_y(), target[0] - e.center_x()))
        turn = (wanted - heading + 180.0) % 360.0 - 180.0
        step = TURN_SPEED * difficulty.get(e.world, "homing") * dt
        heading += max(-step, min(step, turn))
        speed = math.hypot(e.vx(), e.vy())
        e.set_velocity(speed * math.cos(math.radians(heading)), speed * math.sin(math.radians(heading)))
    e.set_angle(heading - 180.0)
