"""Laser lock: an invisible marker that blinks a red line, then fires a beam all at once.

Horizontal (default): the line is the marker's row and the beam goes from the marker to the left edge.
Vertical (param "vertical"): the line is the marker's column and the beam goes from top to bottom.
The charge lasts as long as the difficulty says; the beam pieces disappear by themselves (Lifetime).
With the param "quiet" the lock plays no sound (the Vessel plays one for a whole grid).
"""

import difficulty

PIECE = 100.0
THICKNESS = 80.0


def on_spawn(e):
    e.vertical = e.number("vertical", 0.0) > 0.0
    if e.vertical:
        e.line = e.world.spawn("vessel_warn_vertical", e.x() - 5.0, 0.0)
    else:
        e.line = e.world.spawn("vessel_warn", 0.0, e.y() - 5.0)
    e.charge = difficulty.get(e.world, "laser_charge")
    if e.number("quiet", 0.0) <= 0.0:
        e.world.play_sound("laserCharge")


def on_update(e, dt):
    if e.world.label("phase") in ("victory", "defeat"):
        e.line.destroy()
        e.destroy()
        return
    if e.age() < e.charge:
        return
    e.line.destroy()
    if e.vertical:
        y = 0.0
        while y < e.world.height() and e.world.can_spawn():
            e.world.spawn("vessel_beam_vertical", e.x() - THICKNESS / 2.0, y)
            y += PIECE
    else:
        x = e.x() - PIECE
        while x > -PIECE and e.world.can_spawn():
            e.world.spawn("vessel_beam", x, e.y() - THICKNESS / 2.0)
            x -= PIECE
    if e.number("quiet", 0.0) <= 0.0:
        e.world.play_sound("laserFire")
    e.destroy()
