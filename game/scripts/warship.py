"""Warship, boss of stage 2.

Phase 1 (above 50 %): stays at its post and sends walls of lasers covering the whole
                      screen, with one gap the players must reach in time.
Phase 2:              follows a player, then charges with a blinking "!" for as long as the
                      LASER_CHARGE sound lasts, still drifting toward the player but slower and
                      slower; then a beam appears at once from its nose to the left edge of the
                      screen (LASER_FIRE), stays while the sound plays, and the boss rests.
"""

import math

import boss_common
import common
import difficulty

AIM_SECONDS = 1.6
CHARGE_SECONDS = 2.43
BEAM_SECONDS = 1.5
BEAM_WIDTH = 256.0
BEAM_HEIGHT = 200.0
NOSE_INSET = 140.0
FOLLOW_SPEED = 220.0
CHARGE_END_SPEED = 25.0
REST_SECONDS = 1.8
WALL_EVERY = 2.2
WALL_SPEED = 800.0
ROW_SPACING = 50.0
GAP_ROWS = 4


def on_update(e, dt):
    if not boss_common.enter(e):
        return
    if boss_common.phase(e, (0.5,)) == 1:
        e.set_velocity(0.0, 50.0 * math.cos(e.age()))
        if e.every("wall", WALL_EVERY):
            laser_wall(e)
        return
    beam_attack(e)


def laser_wall(e):
    world = e.world
    rows = int(world.height() // ROW_SPACING)
    gap_start = int(world.random(1.0, float(rows - GAP_ROWS)))
    for row in range(rows):
        if gap_start <= row < gap_start + GAP_ROWS:
            continue
        if world.can_spawn():
            laser = world.spawn("boss_laser", e.x(), row * ROW_SPACING + ROW_SPACING / 2.0)
            laser.set_velocity(-difficulty.speed(e, WALL_SPEED), 0.0)


def beam_attack(e):
    if not hasattr(e, "beam_state"):
        switch(e, "aim")
    elapsed = e.age() - e.beam_since
    if e.beam_state == "aim":
        common.follow_player_y(e, FOLLOW_SPEED)
        if elapsed >= AIM_SECONDS:
            e.world.play_sound("laserCharge")
            boss_common.warn(e, "charge_warning")
            switch(e, "charge")
    elif e.beam_state == "charge":
        progress = min(1.0, elapsed / CHARGE_SECONDS)
        common.follow_player_y(e, FOLLOW_SPEED * (1.0 - progress) ** 2 + CHARGE_END_SPEED * progress)
        if elapsed >= CHARGE_SECONDS:
            e.set_velocity(0.0, 0.0)
            e.world.play_sound("laserFire")
            show_beam(e)
            switch(e, "fire")
    elif e.beam_state == "fire":
        if elapsed >= BEAM_SECONDS:
            switch(e, "rest")
    elif elapsed >= REST_SECONDS:
        switch(e, "aim")


def show_beam(e):
    """The whole beam at once, from inside the boss to the left edge; the pieces vanish by themselves."""
    y = e.center_y() - BEAM_HEIGHT / 2.0
    x = e.x() + NOSE_INSET - BEAM_WIDTH
    while x > -BEAM_WIDTH and e.world.can_spawn():
        e.world.spawn("boss_beam", x, y)
        x -= BEAM_WIDTH


def switch(e, state):
    e.beam_state = state
    e.beam_since = e.age()
