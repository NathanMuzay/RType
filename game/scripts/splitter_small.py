"""Splitter piece: keeps the direction it was thrown in and slowly curves toward the player."""

import common


def on_spawn(e):
    common.start_moving(e, -400.0)


def on_update(e, dt):
    if e.age() > 0.6:
        common.follow_player_y(e, 120.0)
