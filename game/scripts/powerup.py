"""Power-up: floats to the left; the first player who touches it gets its effect.

Param effect: "double", "spread", "heal", "shield", "force" or "drone".
A second drone for the same ship heals instead.
"""

import common
import player

FORCE_OFFSET_X = 100.0
FORCE_OFFSET_Y = 0.0
DRONE_OFFSET_X = -110.0
DRONE_OFFSET_Y = 10.0


def on_spawn(e):
    common.start_moving(e, -150.0)


def on_update(e, dt):
    common.sine(e, 40.0, 3.0)


def on_contact(e, other):
    if not other.is_player() or not e.alive():
        return
    apply(e.text("effect", "heal"), other)
    e.world.play_sound("powerup")
    e.destroy()


def apply(effect, ship):
    world = ship.world
    if effect == "double":
        ship.set_weapon(player.weapon(ship), 0.25, 2, 4.0)
        ship.set_player_value("weapon", 1)
    elif effect == "spread":
        ship.set_weapon(player.weapon(ship), 0.3, 3, 20.0)
        ship.set_player_value("weapon", 2)
    elif effect == "shield":
        ship.set_invulnerable(3.0)
    elif effect == "force" and world.count_scripted("force.py") < len(world.players()):
        force = world.spawn("force", ship.x() + FORCE_OFFSET_X, ship.y())
        force.attach_to(ship, FORCE_OFFSET_X, FORCE_OFFSET_Y)
        ship.set_player_value("force", 1)
    elif effect == "drone" and ship.player_value("drone") == 0:
        drone = world.spawn("drone", ship.x() + DRONE_OFFSET_X, ship.y() + DRONE_OFFSET_Y)
        drone.attach_to(ship, DRONE_OFFSET_X, DRONE_OFFSET_Y)
        ship.set_player_value("drone", 1)
    else:
        ship.heal(30)
