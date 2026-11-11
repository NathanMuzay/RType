"""Versus mode (two players), chosen with the "mode" room rule; rooms in this mode have two seats.

Player 1 (slot 0) stays on the left half and shoots right; player 2 (slot 1) is turned around on the
right half and shoots left. Nobody crosses the centre line. A round is played with one life: when a ship
dies, the other player scores; the first to reach the "rounds" rule wins the match, the result stays
on screen for a few seconds and everybody goes back to the room (world.finish_game). Between rounds, `end_match(delay)` lets the engine clear the world and respawn both ships.
Power-ups (no Force, no drone) appear on the centre line, unless the "bonuses" rule is OFF.
Idea and first version by Origanire, rewritten on the current campaign.

World values for the HUD: mode, duel (WAITING / ROUND / FIGHT / POINT / CHAMPION), round, winsP1,
winsP2, winner (1 or 2, 0 for a draw).
"""

ARENA_STAGE = 2
CENTER_X = 960.0
CENTER_GAP = 20.0
SPAWN_MARGIN = 200.0
SPAWN_Y = 490.0
LEFT_MUZZLE = (-52.0, 19.5)
ROUND_BANNER_SECONDS = 1.5
ROUND_DELAY = 2.5
CHAMPION_DELAY = 5.0
POWERUP_INTERVAL = 8.0
POWERUP_DRIFT = 1.0
POWERUPS = ("powerup_double", "powerup_spread", "powerup_heal", "powerup_shield")
CHARGED_GAP = 4.0


def active(world):
    return world.rule("mode") == "VERSUS"


def faces_left(ship):
    return ship.player_slot() == 1


def weapon(ship):
    return "pvp_missile_left" if faces_left(ship) else "pvp_missile"


def charged_shot(ship):
    """(prefab, angle, dx) of the charged shot, started in front of the ship so it cannot hit it."""
    gap = ship.width() / 2.0 + 80.0 + CHARGED_GAP
    if faces_left(ship):
        return "pvp_charged_left", 180.0, -gap
    return "pvp_charged", 0.0, gap


def place(ship):
    """Puts a freshly spawned ship on its side of the arena, facing the opponent."""
    if faces_left(ship):
        ship.set_position(1920.0 - SPAWN_MARGIN - ship.width(), SPAWN_Y)
        ship.set_angle(180.0)
        ship.set_weapon_offset(*LEFT_MUZZLE)
    else:
        ship.set_position(SPAWN_MARGIN, SPAWN_Y)


def keep_on_side(ship):
    if faces_left(ship):
        if ship.x() < CENTER_X + CENTER_GAP:
            ship.set_position(CENTER_X + CENTER_GAP, ship.y())
    elif ship.x() + ship.width() > CENTER_X - CENTER_GAP:
        ship.set_position(CENTER_X - CENTER_GAP - ship.width(), ship.y())


def update(d, dt):
    """Runs the versus match; called every frame by the director instead of the campaign."""
    world = d.world
    world.set_label("mode", "VERSUS")
    world.set_label("phase", "running")
    world.set_label("music", "versus")
    world.set("stage", ARENA_STAGE)
    if not hasattr(d, "vs_state"):
        reset_match(d)
    if world.match_phase() != "playing":
        d.vs_state = "between"
        return
    if world.player_count() < 2:
        if d.vs_state != "waiting":
            reset_match(d)
        world.set_label("duel", "WAITING")
        return
    d.vs_timer += dt
    STATES[d.vs_state](d, dt)


def reset_match(d):
    world = d.world
    d.vs_state = "waiting"
    d.vs_timer = 0.0
    d.vs_champion = False
    for name in ("round", "winsP1", "winsP2", "winner"):
        world.set(name, 0)


def enter(d, state, label=None):
    d.vs_state = state
    d.vs_timer = 0.0
    if label:
        d.world.set_label("duel", label)


def waiting(d, dt):
    start_round(d)


def between(d, dt):
    if d.vs_champion:
        reset_match(d)
    start_round(d)


def start_round(d):
    d.world.set("round", d.world.get("round") + 1)
    d.next_powerup = POWERUP_INTERVAL
    enter(d, "round", "ROUND")


def round_banner(d, dt):
    """Waits until both ships are in the arena and the banner was shown, then the fight starts."""
    if len(ships(d.world)) >= 2 and d.vs_timer >= ROUND_BANNER_SECONDS:
        enter(d, "fight", "FIGHT")


def fight(d, dt):
    alive = ships(d.world)
    if len(alive) < 2:
        end_round(d, alive)
        return
    if d.vs_timer >= d.next_powerup:
        d.next_powerup = d.vs_timer + POWERUP_INTERVAL
        spawn_powerup(d.world)


def end_round(d, alive):
    world = d.world
    winner = alive[0].player_slot() + 1 if len(alive) == 1 else 0
    world.set("winner", winner)
    if winner:
        key = "winsP%d" % winner
        world.set(key, world.get(key) + 1)
    if winner and world.get("winsP%d" % winner) >= int(world.rule("rounds")):
        d.vs_champion = True
        enter(d, "champion", "CHAMPION")
        world.play_sound("bossDeath")
    else:
        enter(d, "over", "POINT")
        world.end_match(ROUND_DELAY)


def over(d, dt):
    pass


def champion(d, dt):
    """The winner stays on screen, then the room gets back to its waiting screen."""
    if d.vs_timer >= CHAMPION_DELAY:
        d.world.finish_game()


def ships(world):
    return [ship for ship in world.players() if ship.player_slot() in (0, 1)]


def spawn_powerup(world):
    if world.rule("bonuses") == "OFF" or not world.can_spawn():
        return
    index = int(world.random(0.0, len(POWERUPS) - 0.001))
    powerup = world.spawn(POWERUPS[index], CENTER_X - 24.0, world.random(150.0, 880.0))
    powerup.set_velocity(0.0, POWERUP_DRIFT)


STATES = {"waiting": waiting, "between": between, "round": round_banner, "fight": fight, "over": over,
          "champion": champion}
