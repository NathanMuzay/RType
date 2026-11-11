"""Shared movement and timing helpers for the R-Type behaviour scripts.

Every function receives the entity handle `e` given by the engine. Values that a
script needs to remember between two frames are stored directly on `e`
(for example `e.base_y`), the engine keeps one handle per entity.
"""

import math

import difficulty

LEFT = 180.0


def start_moving(e, vx, vy=0.0):
    """Gives the entity its default velocity, unless whoever spawned it already set one."""
    if e.vx() == 0.0 and e.vy() == 0.0:
        e.set_velocity(vx, vy)


def sine(e, amplitude, frequency):
    """Oscillates vertically around the height the entity had when it appeared."""
    e.set_velocity(e.vx(), amplitude * frequency * math.cos(frequency * e.age()))


def bounce(e, speed, top=0.0, bottom=1080.0):
    """Moves up and down at `speed`, turning back at the top and bottom limits."""
    if e.y() <= top:
        e.set_velocity(e.vx(), abs(speed))
    elif e.y() + e.height() >= bottom:
        e.set_velocity(e.vx(), -abs(speed))
    elif e.vy() == 0.0:
        e.set_velocity(e.vx(), speed)


def follow_player_y(e, speed):
    """Moves vertically toward the closest player, at most `speed` pixels per second."""
    target = e.nearest_player()
    if target is None:
        e.set_velocity(e.vx(), 0.0)
        return 0.0
    gap = target[1] - e.center_y()
    e.set_velocity(e.vx(), max(-speed, min(speed, gap * 3.0)))
    return gap


def cooldown(e, name, minimum, maximum):
    """Returns True once every random delay between `minimum` and `maximum` seconds."""
    key = "next_" + name
    scale = difficulty.get(e.world, "fire_delay")
    if not hasattr(e, key):
        setattr(e, key, e.age() + e.world.random(minimum, maximum) * scale)
        return False
    if e.age() < getattr(e, key):
        return False
    setattr(e, key, e.age() + e.world.random(minimum, maximum) * scale)
    return True


def health_ratio(e):
    """Remaining health between 0 and 1."""
    maximum = e.max_health()
    return e.health() / maximum if maximum > 0 else 0.0


def ring(e, prefab, count, speed, offset=0.0):
    """Fires `count` projectiles evenly spread on a full circle."""
    e.fire_spread(prefab, count, 360.0 - 360.0 / count, 180.0 + offset, difficulty.speed(e, speed))


STAGE_SHOTS = {1: "spore", 2: "laser_shot", 3: "homing_missile"}
STAGE_RING_SHOTS = {1: "spore", 2: "laser_shot", 3: "missile"}
BURST_STAGE = 2
BURST_SIZE = 2
BURST_SPACING = 46.0


def stage(e):
    """Number of the stage being played (1, 2 or 3)."""
    return int(e.world.get("stage"))


def shot(e):
    """Projectile of the current stage: spores, then lasers, then homing missiles."""
    return STAGE_SHOTS.get(stage(e), "enemy_bullet")


def ring_shot(e):
    """Projectile used for rings and fans: missiles of stage 3 do not home, or the screen fills up."""
    return STAGE_RING_SHOTS.get(stage(e), "enemy_bullet")


def shoot(e, angle, speed):
    """Fires the stage projectile; in stage 2 the lasers leave in pairs, one behind the other."""
    count = BURST_SIZE if stage(e) == BURST_STAGE else 1
    back_x = -math.cos(math.radians(angle)) * BURST_SPACING
    back_y = -math.sin(math.radians(angle)) * BURST_SPACING
    for index in range(count):
        e.fire(shot(e), angle, difficulty.speed(e, speed), back_x * index, back_y * index)


def shoot_at_player(e, speed):
    """Fires the stage projectile toward the closest player."""
    shoot(e, e.angle_to_player(), speed)


def spread(e, count, angle_range, angle, speed):
    """Fan of stage projectiles."""
    e.fire_spread(ring_shot(e), count, angle_range, angle, difficulty.speed(e, speed))


def stage_ring(e, count, speed, offset=0.0):
    """Full ring of stage projectiles."""
    ring(e, ring_shot(e), count, speed, offset)


BONUSES = (
    ("powerup_double", 3),
    ("powerup_spread", 2),
    ("powerup_heal", 3),
    ("powerup_shield", 2),
    ("powerup_force", 2),
    ("powerup_drone", 2),
)


def drop_bonus(e):
    """Leaves a random power-up where the entity is, unless the "bonuses" room rule is OFF."""
    if e.world.rule("bonuses") == "OFF":
        return
    roll = e.world.random(0.0, sum(weight for _, weight in BONUSES))
    for prefab, weight in BONUSES:
        roll -= weight
        if roll <= 0.0:
            e.spawn(prefab, e.width() / 4.0, e.height() / 4.0)
            return
