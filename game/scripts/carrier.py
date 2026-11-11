"""Carrier: a slow ship that drops a power-up when destroyed, unless the "bonuses" room rule is OFF."""

import common


def on_spawn(e):
    common.start_moving(e, -170.0)


def on_update(e, dt):
    common.sine(e, 70.0, 1.8)


def on_death(e):
    common.drop_bonus(e)
