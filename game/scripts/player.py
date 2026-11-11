"""Player ship: friendly fire rule, charged shot (hold fire, release after CHARGE_SECONDS) and HUD values.

The HUD values are reset here, so a respawned ship starts without its power-ups."""

import bomb
import difficulty
import versus

CHARGE_SECONDS = 1.2
CHARGED_SPEED = 1300.0


def missile(world):
    return "player_missile_ff" if world.rule("friendlyFire") == "ON" else "player_missile"


def back_missile(world):
    return "player_missile_back_ff" if world.rule("friendlyFire") == "ON" else "player_missile_back"


def weapon(ship):
    """Missile prefab of this ship's main weapon, for the current mode."""
    return versus.weapon(ship) if versus.active(ship.world) else missile(ship.world)


def on_spawn(e):
    health = difficulty.get(e.world, "player_health")
    e.set_health(health, health)
    if versus.active(e.world):
        versus.place(e)
    e.set_weapon(weapon(e), 0.2)
    e.set_player_value("weapon", 0)
    e.set_player_value("force", 0)
    e.set_player_value("drone", 0)


def on_update(e, dt):
    in_versus = versus.active(e.world)
    if in_versus:
        versus.keep_on_side(e)
    held = e.input_held("fire")
    e.set_player_value("charge", int(100 * min(1.0, held / CHARGE_SECONDS)))
    if e.input_released("fire") >= CHARGE_SECONDS:
        if in_versus:
            prefab, angle, dx = versus.charged_shot(e)
            e.fire(prefab, angle, CHARGED_SPEED, dx)
        else:
            e.fire("player_missile_charged", 0.0, CHARGED_SPEED)
    if e.input_pressed("bomb") and not in_versus:
        bomb.request(e.world)
