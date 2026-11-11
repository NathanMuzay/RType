"""Drone: a small satellite that trails behind its ship and fires backwards.

Spawned attached to its ship by the "drone" power-up, it detaches on its first update and then
follows a point behind the ship with some lag. While the ship holds fire, it shoots backwards.
It disappears with its ship.
"""

import player

OFFSET_X = -110.0
OFFSET_Y = 10.0
FOLLOW_RATE = 6.0
SHOT_INTERVAL = 0.3
SHOT_SPEED = 1500.0
BACKWARDS = 180.0


def on_spawn(e):
    e.following = False


def on_update(e, dt):
    ship = e.parent()
    if ship is None or not ship.alive():
        e.destroy()
        return
    if not e.following:
        e.detach()
        e.following = True
    follow(e, ship)
    if ship.input_held("fire") > 0.0 and e.every("shot", SHOT_INTERVAL):
        e.fire(player.back_missile(e.world), BACKWARDS, SHOT_SPEED)


def follow(e, ship):
    dx = ship.x() + OFFSET_X - e.x()
    dy = ship.y() + OFFSET_Y - e.y()
    e.set_velocity(dx * FOLLOW_RATE, dy * FOLLOW_RATE)
