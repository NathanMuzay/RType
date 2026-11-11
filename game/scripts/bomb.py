"""Secret bomb: typing the room's secret code (EPITECH) in game drops one bomb per match for the
whole room.

A player script calls `request` when its ship sees the "bomb" input, and the campaign director calls
`update` every frame. The world value "bomb" walks READY -> ARMED -> FLASH -> USED: the server is the
one that decides, so a modified client cannot use it twice. The detonation kills every enemy on screen,
wipes enemy shots and only damages the boss.
"""

READY = "READY"
ARMED = "ARMED"
FLASH = "FLASH"
USED = "USED"
FLASH_SECONDS = 0.3
BOSS_DAMAGE = 0.25
SHAKE = 18


def reset(world):
    world.set_label("bomb", READY)


def request(world):
    if world.label("bomb") == READY:
        world.set_label("bomb", ARMED)


def update(d, boss):
    """Called by the director `d` every frame; `boss` is the current boss or None."""
    world = d.world
    state = world.label("bomb")
    if state == ARMED:
        detonate(world, boss)
        world.set_label("bomb", FLASH)
        d.bomb_flash_end = world.time() + FLASH_SECONDS
    elif state == FLASH and world.time() >= d.bomb_flash_end:
        world.set_label("bomb", USED)


def detonate(world, boss):
    for enemy in world.in_layer("enemy"):
        if not on_screen(world, enemy):
            continue
        if enemy.number("bombProof", 0.0) > 0.0 and not (boss is not None and enemy == boss):
            continue
        if boss is not None and enemy == boss:
            hurt_boss(enemy)
        else:
            enemy.kill()
    for shot in world.in_layer("enemy_missile"):
        shot.destroy()
    world.set("shake", SHAKE)
    world.play_sound("bomb")


def hurt_boss(boss):
    damage = max(1, int(boss.max_health() * BOSS_DAMAGE))
    boss.set_health(max(1, boss.health() - damage), boss.max_health())


def on_screen(world, e):
    return e.x() + e.width() > 0.0 and e.x() < world.width() and e.y() + e.height() > 0.0 and e.y() < world.height()
