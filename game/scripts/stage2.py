"""Stage 2: inside a Bydo warship.

The whole stage is a corridor: a metal ceiling and floor scroll by, with cannons fixed on them.
Heavies, turrets, chasers, a snake and an ambush from behind come through; the corridor ends
just as the Warship arrives, and the fight takes place in the open hangar behind it."""

DURATION = 75.0
BOSS = ("boss_warship", 2000.0, 324.0)
AMBUSH_TIME = 52.0
TUNNEL_TIME = 3.0
CANNON_STOP = 8.0
CANNON_EVERY = 5.0
FLOOR_Y = 900.0
CEILING_Y = 80.0

RANDOM = (0.9, 1.8, [
    ("bydo_straight", 2, 120.0, 880.0),
    ("bydo_sine", 2, 250.0, 750.0),
    ("pod", 2, 150.0, 850.0),
    ("chaser", 1, 150.0, 850.0),
    ("carrier", 1, 200.0, 780.0),
])

WAVES = [
    (3.0, "heavy", 2, 1950.0, 300.0, 0.0, 400.0),
    (20.0, "chaser", 4, 1950.0, 200.0, 200.0, 200.0),
    (38.0, "snake_head", 1, 1950.0, 500.0, 0.0, 0.0),
    (46.0, "turret", 2, 1950.0, 200.0, 0.0, 550.0),
    (58.0, "carrier", 2, 1950.0, 250.0, 300.0, 500.0),
    (62.0, "heavy", 3, 1950.0, 150.0, 200.0, 350.0),
    (68.0, "turret", 3, 1950.0, 120.0, 120.0, 380.0),
]


def events(world, time, director):
    done = director.done
    if time >= TUNNEL_TIME and "tunnel" not in done:
        done.add("tunnel")
        director.leftovers.append(world.spawn("terrain_stage2", 1920.0, 0.0))
    in_tunnel = TUNNEL_TIME + 2.0 <= time <= DURATION - CANNON_STOP
    if in_tunnel and world.every("cannon", CANNON_EVERY) and world.can_spawn():
        on_floor = int(time / CANNON_EVERY) % 2 == 0
        world.spawn("ground_turret" if on_floor else "ceiling_turret", 1950.0, FLOOR_Y if on_floor else CEILING_Y)
    if time >= AMBUSH_TIME and "ambush" not in done:
        done.add("ambush")
        for index in range(5):
            pod = world.spawn("pod", -150.0 - index * 90.0, 150.0 + index * 180.0)
            pod.set_velocity(420.0, 0.0)
