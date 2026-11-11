"""Helpers shared by the bosses: entering the screen, choosing a phase from health, warning signs."""

import common


def enter(e, speed=220.0):
    """Starts moving left; returns True once the boss reached its `stop_x` param."""
    if not hasattr(e, "arrived"):
        e.arrived = False
        e.set_velocity(-speed, 0.0)
    if not e.arrived and e.x() <= e.number("stop_x", 1400.0):
        e.arrived = True
        e.set_velocity(0.0, 0.0)
    return e.arrived


def phase(e, thresholds):
    """Phase number from health: thresholds (0.6, 0.25) gives 1 above 60 %, 2 above 25 %, else 3."""
    ratio = common.health_ratio(e)
    current = 1
    for threshold in thresholds:
        if ratio <= threshold:
            current += 1
    return current


def burst(e, name, size, gap, every_min, every_max, shoot):
    """Calls `shoot(e)` `size` times, `gap` seconds apart, every few seconds."""
    left = name + "_left"
    if getattr(e, left, 0) == 0 and common.cooldown(e, name, every_min, every_max):
        setattr(e, left, size)
    if getattr(e, left, 0) > 0 and e.every(name + "_gap", gap):
        shoot(e)
        setattr(e, left, getattr(e, left) - 1)


def warn(e, prefab="warning"):
    """Shows a blinking "!" in front of the boss; it follows the boss and disappears by itself."""
    sign = e.world.spawn(prefab, e.x() - 120.0, e.center_y() - 90.0)
    sign.attach_to(e, -120.0, e.height() / 2.0 - 90.0)
