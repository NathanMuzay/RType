"""Bydo Vessel, the giant warship of stage 3.

During the stage it cannot be hurt: it hangs on the right edge, follows the players and every few
seconds lines up with the closest one, stops and fires a laser from its centre (see laser_lock.py).

When the boss phase starts it settles in the middle of the right side and does not move any more:
- every few seconds a grid of lasers blinks (columns and rows across the whole screen), then fires
  all at once; the players hide in a free cell;
- it throws meteors at the players (three shots break one);
- phase 2 (below half health): two flying escorts, far too tough to destroy, hover above and below it and fire three
  lasers every two seconds (see vessel_turret.py).
"""

import boss_common
import common
import difficulty

STALK_X = 1480.0
FIGHT_X = 1150.0
EDGE_MARGIN = 70.0
NOSE = 150.0
STALK_EVERY = 4.5
STALK_FOLLOW = 160.0
SETTLE_SPEED = 200.0
SHIELD_SECONDS = 1000000.0
BEAM_SECONDS = 1.5
GRID_EVERY = 6.0
GRID_COLUMNS = (200.0, 450.0, 700.0, 950.0)
GRID_ROWS = (180.0, 420.0, 660.0, 900.0)
COLUMNS_PER_GRID = 3
ROWS_PER_GRID = 2
GRID_GAP = 160.0
METEOR_SPEED = 380.0
TURRET_DX = 0.45


def on_spawn(e):
    e.set_invulnerable(SHIELD_SECONDS)
    e.fighting = False
    e.next_laser = STALK_EVERY


def on_update(e, dt):
    if not e.fighting and e.world.label("phase") == "boss":
        e.fighting = True
        e.set_invulnerable(0.0)
        e.next_grid = e.age() + 2.0
    if e.fighting:
        fight(e)
    else:
        stalk(e)


def stalk(e):
    e.set_velocity(-260.0 if e.x() > STALK_X else 0.0, e.vy())
    if holding(e):
        return
    follow(e, STALK_FOLLOW)
    if e.age() >= e.next_laser:
        e.next_laser = e.age() + STALK_EVERY * difficulty.get(e.world, "fire_delay")
        lock(e)


def fight(e):
    settle(e)
    if e.age() >= e.next_grid:
        e.next_grid = e.age() + GRID_EVERY * difficulty.get(e.world, "fire_delay")
        grid(e)
    if e.age() >= getattr(e, "fire_sound_at", 1e9):
        e.fire_sound_at = 1e9
        e.world.play_sound("laserFire")
    if common.cooldown(e, "meteor", 1.8, 2.6) and e.world.can_spawn():
        meteor = e.world.spawn("meteor", e.x() + 40.0, e.center_y() - 39.0)
        meteor.set_velocity(0.0, 0.0)
        aim(meteor, difficulty.speed(e, METEOR_SPEED))
    if boss_common.phase(e, (0.5,)) == 2 and not getattr(e, "turrets", None):
        open_turrets(e)


def settle(e):
    """Comes to the middle of the right side, then stays there."""
    middle = (e.world.height() - e.height()) / 2.0
    vy = max(-SETTLE_SPEED, min(SETTLE_SPEED, (middle - e.y()) * 2.0))
    e.set_velocity(-260.0 if e.x() > FIGHT_X else 0.0, vy)


def grid(e):
    """One column and one row on the closest player, the others at random: some cells stay free,
    but standing still is never safe."""
    world = e.world
    target = e.nearest_player()
    if target is None:
        target = (GRID_COLUMNS[0], GRID_ROWS[0])
    columns = [min(max(target[0], GRID_COLUMNS[0]), GRID_COLUMNS[-1])]
    rows = [min(max(target[1], GRID_ROWS[0]), GRID_ROWS[-1])]
    columns += pick(world, [x for x in GRID_COLUMNS if abs(x - columns[0]) > GRID_GAP], COLUMNS_PER_GRID - 1)
    rows += pick(world, [y for y in GRID_ROWS if abs(y - rows[0]) > GRID_GAP], ROWS_PER_GRID - 1)
    for x in columns:
        world.spawn("grid_lock_vertical", x, 0.0)
    for y in rows:
        world.spawn("grid_lock", world.width() + 100.0, y)
    world.play_sound("laserCharge")
    e.fire_sound_at = e.age() + difficulty.get(world, "laser_charge")


def pick(world, choices, count):
    left = list(choices)
    picked = []
    while len(picked) < count and left:
        picked.append(left.pop(int(world.random(0.0, len(left) - 0.001))))
    return picked


def aim(meteor, speed):
    target = meteor.nearest_player()
    if target is None:
        meteor.set_velocity(-speed, 0.0)
        return
    dx = target[0] - meteor.center_x()
    dy = target[1] - meteor.center_y()
    length = max(1.0, (dx * dx + dy * dy) ** 0.5)
    meteor.set_velocity(dx / length * speed, dy / length * speed)


def open_turrets(e):
    x = e.width() * TURRET_DX
    top = e.world.spawn("vessel_turret_top", e.x() + x, e.y() - 120.0)
    top.attach_to(e, x, -120.0)
    bottom = e.world.spawn("vessel_turret", e.x() + x, e.y() + e.height() + 40.0)
    bottom.attach_to(e, x, e.height() + 40.0)
    e.turrets = [top, bottom]


def on_death(e):
    for turret in getattr(e, "turrets", []) or []:
        if turret.exists():
            turret.destroy()


def lock(e):
    """A laser from the centre of the Vessel; it stays still while the laser charges and fires."""
    e.world.spawn("laser_lock", e.x() + NOSE, e.center_y())
    e.hold_until = e.age() + difficulty.get(e.world, "laser_charge") + BEAM_SECONDS
    e.set_velocity(e.vx(), 0.0)


def holding(e):
    return e.age() < getattr(e, "hold_until", 0.0)


def target_y(e):
    target = e.nearest_player()
    return target[1] if target is not None else e.world.height() / 2.0


def follow(e, speed):
    gap = target_y(e) - e.center_y()
    e.set_velocity(e.vx(), max(-speed, min(speed, gap)))
    keep_inside(e)


def keep_inside(e):
    if e.y() < EDGE_MARGIN and e.vy() < 0.0:
        e.set_velocity(e.vx(), 0.0)
    elif e.y() + e.height() > e.world.height() - EDGE_MARGIN and e.vy() > 0.0:
        e.set_velocity(e.vx(), 0.0)
