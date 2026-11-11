"""Dobkeratops, boss of stage 1.

Phase 1 (above 60 %): sways slowly, fires bursts of 4 aimed missiles.
Phase 2 (above 25 %): adds wide fans of 9 orbs and calls pods for help.
Phase 3 (rage):       everything fires faster and the fans get denser.
"""

import math

import boss_common
import common
import difficulty


def on_update(e, dt):
    if not boss_common.enter(e):
        return
    e.set_velocity(0.0, 140.0 * math.cos(e.age() * 0.7))
    current = boss_common.phase(e, (0.6, 0.25))
    rage = current == 3
    boss_common.burst(e, "aimed", 4, 0.12 if rage else 0.18, 1.2 if rage else 1.8, 2.4, aimed_shot)
    if current >= 2 and common.cooldown(e, "fan", 1.6 if rage else 2.6, 3.2):
        e.fire_spread("boss_orb", 13 if rage else 9, 110.0, common.LEFT, difficulty.speed(e, 380.0))
    if current >= 2 and common.cooldown(e, "help", 6.0, 8.0) and e.world.can_spawn():
        for offset in (-200.0, 200.0):
            e.world.spawn("pod", e.x() - 80.0, e.center_y() + offset)


def aimed_shot(e):
    e.fire_at_player("boss_missile", difficulty.speed(e, 650.0))
