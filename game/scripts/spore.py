"""Spore: stage 1 projectile. A slow organic seed that bursts into two smaller spores after a moment."""

import math

SPLIT_AFTER = 0.9
SPLIT_ANGLE = 28.0
SPEED_FACTOR = 1.25


def on_update(e, dt):
    if e.age() < SPLIT_AFTER:
        return
    heading = math.degrees(math.atan2(e.vy(), e.vx()))
    speed = math.hypot(e.vx(), e.vy()) * SPEED_FACTOR
    for side in (-1.0, 1.0):
        if e.world.can_spawn():
            angle = math.radians(heading + side * SPLIT_ANGLE)
            small = e.world.spawn("spore_small", e.x(), e.y())
            small.set_velocity(speed * math.cos(angle), speed * math.sin(angle))
    e.destroy()
