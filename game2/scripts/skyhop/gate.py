"""Invisible gate between two rocks: crossing it scores one point."""


def on_death(e):
    world = e.world
    world.play_sound("score")
    world.set("score", world.get("score") + 1)
    world.set("best", max(world.get("best"), world.get("score")))
