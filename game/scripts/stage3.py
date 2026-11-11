"""Stage 3: the Bydo Vessel hunts the players during the whole stage.

The giant warship stays on the right edge and cannot be hurt: every few seconds it
lines up with the closest player, stops and fires a laser. Splitters, mine fields and a heavy squad come
through meanwhile; when the stage ends, the Vessel comes forward and becomes the boss.
"""

DURATION = 85.0
BOSS = ("boss_vessel", 2000.0, 400.0)
BOSS_MUSIC = "boss3"
VESSEL_TIME = 2.0
MINE_FIELD_TIME = 30.0
SQUAD_TIME = 55.0

RANDOM = (1.4, 2.4, [
    ("bydo_sine", 2, 150.0, 850.0),
    ("pod", 3, 100.0, 950.0),
    ("hopper", 2, 200.0, 800.0),
    ("chaser", 2, 100.0, 900.0),
    ("splitter", 1, 150.0, 850.0),
])

WAVES = [
    (6.0, "heavy", 2, 1700.0, 150.0, 0.0, 600.0),
    (15.0, "splitter", 3, 1700.0, 200.0, 160.0, 280.0),
    (42.0, "chaser", 4, 1700.0, 150.0, 100.0, 220.0),
    (68.0, "splitter", 3, 1700.0, 150.0, 140.0, 280.0),
    (76.0, "hopper", 4, 1700.0, 200.0, 90.0, 180.0),
]


def events(world, time, director):
    done = director.done
    if time >= VESSEL_TIME and "vessel" not in done:
        done.add("vessel")
        prefab, x, y = BOSS
        director.vessel = world.spawn(prefab, x, y)
        director.leftovers.append(director.vessel)
    if time >= MINE_FIELD_TIME and "mines" not in done:
        done.add("mines")
        for row in range(3):
            for column in range(2):
                world.spawn("mine", 1500.0 + column * 260.0, 180.0 + row * 300.0 + column * 80.0)
    if time >= SQUAD_TIME and "squad" not in done:
        done.add("squad")
        for index in range(2):
            world.spawn("heavy", 1650.0 + index * 160.0, 250.0 + index * 450.0)


def take_boss(director):
    """The boss is the Vessel that hunted the players; it is created now if the stage was skipped."""
    vessel = getattr(director, "vessel", None)
    if vessel is None or not vessel.alive():
        prefab, x, y = BOSS
        vessel = director.world.spawn(prefab, x, y)
    director.vessel = None
    return vessel
