"""Basic Bydo: flies left following the pattern given in its prefab params.

Params: pattern ("straight", "sine" or "zigzag"), speed, amplitude, frequency,
vertical (zigzag speed), shoots (1 to fire, 0 to stay silent).
"""

import common


def on_spawn(e):
    e.pattern = e.text("pattern", "straight")
    vertical = e.number("vertical", 250.0)
    common.start_moving(e, -e.number("speed", 250.0), vertical if e.pattern == "zigzag" else 0.0)


def on_update(e, dt):
    if e.pattern == "sine":
        common.sine(e, e.number("amplitude", 150.0), e.number("frequency", 3.0))
    elif e.pattern == "zigzag":
        common.bounce(e, e.number("vertical", 250.0))
    if e.number("shoots", 1.0) > 0.0 and common.cooldown(e, "shot", 1.5, 3.0):
        common.shoot(e, common.LEFT, 600.0)
