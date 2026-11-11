"""Stage 1: teaches the basics with pods, bydos, hoppers and a first carrier, then the Dobkeratops."""

DURATION = 60.0
BOSS = ("boss_dobkeratops", 2000.0, 285.0)

RANDOM = (1.2, 2.4, [
    ("bydo_straight", 3, 0.0, 1000.0),
    ("bydo_sine", 2, 150.0, 850.0),
    ("carrier", 1, 150.0, 850.0),
])

WAVES = [
    (4.0, "pod", 5, 1950.0, 200.0, 140.0, 0.0),
    (15.0, "hopper", 4, 1950.0, 150.0, 0.0, 220.0),
    (28.0, "pod", 6, 1950.0, 800.0, 120.0, -100.0),
    (40.0, "hopper", 5, 1950.0, 100.0, 160.0, 180.0),
    (10.0, "carrier", 1, 1950.0, 300.0, 0.0, 0.0),
    (20.0, "carrier", 1, 1950.0, 500.0, 0.0, 0.0),
    (45.0, "carrier", 1, 1950.0, 700.0, 0.0, 0.0),
    (52.0, "bydo_zigzag", 3, 1950.0, 700.0, 150.0, 0.0),
]
