"""Flying escort of the Vessel in phase 2: it hovers above or below it, is far too tough to destroy,
and fires three lasers at the players every two seconds."""

import boss_common
import difficulty

SHOTS = 3
GAP = 0.15
EVERY = 2.0
SPEED = 700.0


def on_update(e, dt):
    boss_common.burst(e, "lasers", SHOTS, GAP, EVERY, EVERY, shoot)


def shoot(e):
    e.fire_at_player("laser_shot", difficulty.speed(e, SPEED))
