"""Player plane: tap to flap, gravity pulls it down, a rock or the ground ends the run.

After a crash, a new tap (once the plane has landed) asks the director for a new run.
"""

FLAP_SPEED = -760.0
GRAVITY = 2300.0
GROUND_Y = 909.0
CEILING_Y = 0.0
START_X = 420.0
START_Y = 420.0
RESTART_DELAY = 0.6


def on_spawn(e):
    e.was_held = False
    reset(e)


def reset(e):
    e.crashed = False
    e.crash_time = 0.0
    e.set_position(START_X, START_Y)
    e.set_velocity(0.0, 0.0)
    e.set_angle(0.0)


def tapped(e):
    held = e.input_pressed("fire") or e.input_pressed("action")
    tap = held and not e.was_held
    e.was_held = held
    return tap


def on_update(e, dt):
    world = e.world
    phase = world.label("phase")
    tap = tapped(e)
    if phase == "ready":
        e.set_velocity(0.0, 0.0)
        if tap:
            world.set_label("phase", "flying")
            flap(e)
        return
    if phase == "over":
        fall(e, dt)
        if tap and e.age() - e.crash_time >= RESTART_DELAY:
            reset(e)
            world.set("restart", 1)
        return
    if tap:
        flap(e)
    fall(e, dt)
    if e.y() < CEILING_Y:
        e.set_position(e.x(), CEILING_Y)
        e.set_velocity(0.0, 0.0)
    if e.y() + e.height() >= GROUND_Y:
        crash(e)


def flap(e):
    e.set_velocity(0.0, FLAP_SPEED)
    e.world.play_sound("flap")


def fall(e, dt):
    if e.y() + e.height() >= GROUND_Y:
        e.set_position(e.x(), GROUND_Y - e.height())
        e.set_velocity(0.0, 0.0)
        return
    e.set_velocity(0.0, e.vy() + GRAVITY * dt)
    e.set_angle(max(-25.0, min(70.0, e.vy() / 12.0)))


def crash(e):
    if e.crashed:
        return
    e.crashed = True
    e.crash_time = e.age()
    e.world.set_label("phase", "over")
    e.world.play_sound("crash")


def on_contact(e, other):
    if other.number("deadly", 0.0) > 0.0 and e.world.label("phase") == "flying":
        crash(e)
