"""Difficulty: every setting of the "difficulty" room rule, in one table.

    shot_speed      multiplies the speed of every enemy projectile
    fire_delay      multiplies the waiting time of enemies between two attacks
    spawn_delay     multiplies the delay between two random enemies
    boss_health     multiplies the health of the bosses
    player_health   health of a player ship
    homing          multiplies how fast homing missiles turn
    laser_charge    seconds the Vessel's laser lines blink before the beam

The coop game also gets harder with the number of players (idea by Origanire): see PLAYERS below,
applied on top of the rule when enemies and bosses appear.
"""

TABLE = {
    "EASY": {"shot_speed": 0.55, "fire_delay": 2.0, "spawn_delay": 1.7, "boss_health": 0.6,
             "player_health": 220, "homing": 0.35, "laser_charge": 2.6},
    "NORMAL": {"shot_speed": 0.8, "fire_delay": 1.35, "spawn_delay": 1.25, "boss_health": 0.85,
               "player_health": 140, "homing": 0.7, "laser_charge": 2.2},
    "HARD": {"shot_speed": 1.15, "fire_delay": 0.8, "spawn_delay": 0.8, "boss_health": 1.3,
             "player_health": 90, "homing": 1.2, "laser_charge": 1.8},
}


PLAYERS = {
    "enemy_health": (1.0, 1.25, 1.5, 1.75),
    "spawn_rate": (1.0, 1.15, 1.3, 1.45),
    "boss_health": (1.0, 1.5, 2.0, 2.5),
}
EXTRA_WAVE_PER_PLAYER = 0.25


def players(world):
    return max(1, world.player_count())


def for_players(world, name):
    table = PLAYERS[name]
    return table[min(players(world), len(table)) - 1]


def wave_size(world, count):
    """A wave gets 25 % more enemies for each player after the first."""
    return max(count, round(count * (1.0 + EXTRA_WAVE_PER_PLAYER * (players(world) - 1))))


def toughen(enemy):
    """Gives a freshly spawned enemy more health when several players are in the game."""
    factor = for_players(enemy.world, "enemy_health")
    if factor > 1.0 and enemy.alive() and enemy.max_health() > 0:
        health = round(enemy.max_health() * factor)
        enemy.set_health(health, health)


def get(world, name):
    return TABLE.get(world.rule("difficulty"), TABLE["NORMAL"])[name]


def speed(e, value):
    """An enemy projectile speed, scaled by the difficulty."""
    return value * get(e.world, "shot_speed")
