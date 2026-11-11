"""Heavy: a slow armoured ship that fires fans of three bullets.

When it drops below half health it stops for one second and answers with a full
ring of bullets, then goes on a bit faster.
"""

import common

STOP_SECONDS = 1.0


def on_spawn(e):
    common.start_moving(e, -140.0)
    e.enraged = False
    e.stop_until = 0.0


def on_damage(e, amount):
    if not e.enraged and common.health_ratio(e) <= 0.5:
        e.enraged = True
        e.stop_until = e.age() + STOP_SECONDS
        e.set_velocity(0.0, 0.0)
        common.stage_ring(e, 10, 380.0)


def on_update(e, dt):
    if e.age() < e.stop_until:
        return
    if e.enraged and e.vx() == 0.0:
        e.set_velocity(-220.0, 0.0)
    if common.cooldown(e, "fan", 1.0, 1.6):
        common.spread(e, 3, 30.0, e.angle_to_player(), 450.0)
