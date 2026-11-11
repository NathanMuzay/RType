"""Pod: a fast kamikaze that crosses the screen in a straight line, usually in groups."""

import common


def on_spawn(e):
    common.start_moving(e, -e.number("speed", 480.0))
