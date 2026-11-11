"""Meteor thrown by the Vessel at the players; it spins and breaks after three shots."""

SPIN = 60.0


def on_update(e, dt):
    e.set_angle(e.age() * SPIN)
