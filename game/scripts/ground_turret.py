"""Ground turret: a cannon fixed on the floor or the ceiling of the stage 2 tunnel.

It scrolls with the terrain and fires bursts of lasers at the closest player.
"""

import common

SCROLL_SPEED = 120.0


def on_spawn(e):
    common.start_moving(e, -SCROLL_SPEED)


def on_update(e, dt):
    if e.x() < 1800.0 and common.cooldown(e, "shot", 2.0, 3.2):
        common.shoot_at_player(e, 520.0)
