"""Campaign: plays one stage of R-Type, chosen with the "level" room rule (see spawner.py for stages).

intro -> running -> boss -> victory, or after the last boss ending -> fin -> credits. In victory the room owner clicks NEXT LEVEL or RESTART: those
buttons send the commands "next" and "restart", which set the world value "choice". When every ship
is down at the same time the phase becomes "defeat", the stage is cleared and the owner clicks
PLAY AGAIN. QUIT leaves the room (the owner leaving closes it). In the VERSUS mode the director runs
versus.py instead.

It publishes the world values of the HUD (stage, phase, progress, bossHealth, music) and runs the
secret bomb. difficulty.py scales boss health; the director param "bossRush" skips waves.
"""

import bomb
import common
import difficulty
import spawner
import stage1
import stage2
import stage3
import versus

STAGES = (stage1, stage2, stage3)
INTRO_SECONDS = 3.0
CHOICE_DELAY = 1.0
WIPE_GRACE = 1.0
NONE = 0
NEXT = 1
RESTART = 2
SHAKE_ON_BOSS_DEATH = 14
BLAST_SECONDS = 3.0
BLAST_EVERY = 0.15
WHITE_AT = 1.8
FIN_AT = 3.5
FIN_SECONDS = 5.0


def on_spawn(d):
    d.playing = False
    d.stage_index = None


def on_update(d, dt):
    world = d.world
    if versus.active(world):
        versus.update(d, dt)
        return
    world.set_label("mode", "COOP")
    if world.match_phase() != "playing":
        d.playing = False
        return
    if not d.playing:
        d.playing = True
        bomb.reset(world)
        start_stage(d, first_stage(d))
    d.timer += dt
    decay_shake(world)
    if world.get("skip") > 0:
        world.set("skip", 0)
        skip(d)
    bomb.update(d, d.boss if d.state == "boss" else None)
    if d.state in ("intro", "running", "boss") and d.timer > WIPE_GRACE and world.players_alive() == 0:
        spawner.clear(d)
        d.boss = None
        enter(d, "defeat")
    STATES[d.state](d, dt)


def first_stage(d):
    """The stage being played before, else the stage chosen with the "level" rule."""
    if d.stage_index is not None:
        return d.stage_index
    return max(0, min(len(STAGES) - 1, int(d.world.rule("level")) - 1))


def start_stage(d, index):
    spawner.clear(d)
    spawner.start(d, STAGES[index])
    d.stage_index = index
    d.boss = None
    enter(d, "intro")
    d.world.set("stage", index + 1)
    d.world.set("progress", 0)
    d.world.set("bossHealth", 0)
    d.world.set("whiteout", 0)


def enter(d, state):
    d.state = state
    d.timer = 0.0
    d.world.set_label("phase", state)
    d.world.set_label("music", music_for(d, state))


def music_for(d, state):
    """The boss theme during a boss (BOSS_MUSIC of the stage if it has one), VICTORY after a boss, FIN for
    the end of the game, the stage theme otherwise; a defeat keeps the music."""
    if state == "boss":
        return getattr(d.stage, "BOSS_MUSIC", "boss")
    if state == "victory":
        return "victory"
    if state in ("fin", "credits"):
        return "fin"
    if state in ("defeat", "ending"):
        return d.world.label("music")
    return "stage%d" % (d.stage_index + 1)


def intro(d, dt):
    if d.timer >= INTRO_SECONDS:
        enter(d, "running")


def running(d, dt):
    if d.number("bossRush", 0.0) > 0.0 or d.timer >= d.stage.DURATION:
        start_boss(d)
        return
    d.world.set("progress", int(100 * d.timer / d.stage.DURATION))
    spawner.update(d, d.timer)


def start_boss(d):
    d.world.set("progress", 100)
    d.boss = spawner.boss(d)
    scale = difficulty.get(d.world, "boss_health") * difficulty.for_players(d.world, "boss_health")
    health = max(1, int(d.boss.max_health() * scale))
    d.boss.set_health(health, health)
    enter(d, "boss")


def boss(d, dt):
    world = d.world
    world.set("bossHealth", int(1000 * common.health_ratio(d.boss)) if d.boss.exists() else 0)
    if d.boss.alive():
        d.boss_box = (d.boss.x(), d.boss.y(), d.boss.width(), d.boss.height())
        return
    world.set("shake", SHAKE_ON_BOSS_DEATH)
    world.play_sound("bossDeath")
    enter(d, "ending" if d.stage_index + 1 >= len(STAGES) else "victory")


def ending(d, dt):
    """The last boss blows up in a chain of explosions while the screen turns white, then "FIN"."""
    world = d.world
    if d.timer < BLAST_SECONDS and world.every("ending_blast", BLAST_EVERY):
        x, y, width, height = getattr(d, "boss_box", (1400.0, 400.0, 400.0, 300.0))
        world.spawn("boss_blast", x + world.random(0.0, width) - 128.0, y + world.random(0.0, height) - 128.0)
        world.set("shake", SHAKE_ON_BOSS_DEATH)
    if d.timer >= WHITE_AT:
        world.set("whiteout", 1)
    if d.timer >= FIN_AT:
        spawner.clear(d)
        enter(d, "fin")


def fin(d, dt):
    if d.timer >= FIN_SECONDS:
        d.world.set("whiteout", 0)
        enter(d, "credits")


def credits(d, dt):
    """The credits stay until a player presses a key; the client then goes back to the intro."""


def victory(d, dt):
    choice = take_choice(d)
    if choice == NEXT and d.stage_index + 1 < len(STAGES):
        start_stage(d, d.stage_index + 1)
    elif choice == RESTART:
        start_stage(d, d.stage_index)


def defeat(d, dt):
    if take_choice(d) != NONE:
        start_stage(d, d.stage_index)


def take_choice(d):
    """Reads and forgets the choice sent with the "next" and "restart" commands."""
    choice = d.world.get("choice")
    if choice != NONE:
        d.world.set("choice", NONE)
    return choice if d.timer >= CHOICE_DELAY else NONE


def skip(d):
    if d.state == "running":
        start_boss(d)
    elif d.state == "boss" and d.boss is not None:
        d.boss.kill()


def decay_shake(world):
    if world.get("shake") > 0 and world.every("shake", 0.05):
        world.set("shake", world.get("shake") - 1)


STATES = {"intro": intro, "running": running, "boss": boss, "victory": victory, "defeat": defeat,
          "ending": ending, "fin": fin, "credits": credits}
