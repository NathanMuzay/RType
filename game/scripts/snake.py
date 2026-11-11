"""Snake: a head that weaves across the screen, followed by armoured body segments.

Only the head can be destroyed; the whole body explodes with it.
Param segments: number of body parts.
"""

import common

SPACING_FRAMES = 6


def on_spawn(e):
    common.start_moving(e, -260.0)
    e.history = []
    e.body = [e.world.spawn("snake_segment", e.x() + 50.0, e.y()) for _ in range(int(e.number("segments", 8.0)))]


def on_update(e, dt):
    common.sine(e, 260.0, 2.2)
    e.history.append((e.x(), e.y()))
    keep = (len(e.body) + 1) * SPACING_FRAMES
    if len(e.history) > keep:
        e.history = e.history[-keep:]
    for index, segment in enumerate(e.body):
        delay = (index + 1) * SPACING_FRAMES
        if len(e.history) > delay:
            x, y = e.history[-delay - 1]
            segment.set_position(x + 10.0, y)


def on_death(e):
    for segment in e.body:
        segment.kill()
