"""The Force: an indestructible satellite that blocks enemy bullets.

- Attached in front of the ship it fires forward with it; attached behind, it fires backwards.
- The action key launches it (forward from the front, backward from the back). Once it slows down
  it acts on its own: it hovers at the height of its ship and keeps firing forward when the ship fires.
- Pressing the action key again recalls it; it docks on the side it comes from. Flying through
  the free Force docks it too.
- When the ship releases a charged shot, an attached Force adds a fan of three charged shots.
"""

import player

LAUNCH_SPEED = 900.0
RECALL_SPEED = 1100.0
FREE_SPEED = 60.0
HOVER_SPEED = 200.0
DOCK_DISTANCE = 70.0
DOCK_DELAY = 0.6
OFFSET_FRONT = 100.0
OFFSET_BACK = -75.0
OFFSET_Y = 0.0
SHOT_EVERY = 0.5
SHOT_SPEED = 1500.0
CHARGED_SPREAD = 24.0
HUD = {"front": 1, "free": 2, "back": 3}


def on_update(e, dt):
    if not hasattr(e, "ship"):
        e.ship = e.parent()
        e.mode = "front"
        e.launched_at = 0.0
    ship = e.ship
    if ship is None or not ship.alive():
        e.destroy()
        return
    if ship.input_pressed("action"):
        toggle(e, ship)
    if e.mode == "free":
        hover(e, ship)
    elif e.mode == "recall":
        recall(e, ship)
    shoot(e, ship)


def toggle(e, ship):
    if e.mode in ("front", "back"):
        e.detach()
        e.set_velocity(LAUNCH_SPEED if e.mode == "front" else -LAUNCH_SPEED, 0.0)
        e.mode = "free"
        e.launched_at = e.age()
        ship.set_player_value("force", HUD["free"])
    else:
        e.mode = "recall"


def hover(e, ship):
    """Slows down after the launch, then follows the height of its ship; docks if the ship touches it."""
    vx = e.vx() * 0.96 if abs(e.vx()) > FREE_SPEED else 0.0
    gap = ship.center_y() - e.center_y()
    e.set_velocity(vx, max(-HOVER_SPEED, min(HOVER_SPEED, gap * 3.0)))
    if e.age() - e.launched_at > DOCK_DELAY and distance_to_side(e, ship) < DOCK_DISTANCE:
        dock(e, ship)


def recall(e, ship):
    side = side_of(e, ship)
    dx = ship.x() + offset(side) - e.x()
    dy = ship.y() + OFFSET_Y - e.y()
    length = (dx * dx + dy * dy) ** 0.5
    if length < DOCK_DISTANCE:
        dock(e, ship)
        return
    e.set_velocity(dx / length * RECALL_SPEED, dy / length * RECALL_SPEED)


def dock(e, ship):
    e.mode = side_of(e, ship)
    e.set_velocity(0.0, 0.0)
    e.attach_to(ship, offset(e.mode), OFFSET_Y)
    ship.set_player_value("force", HUD[e.mode])


def shoot(e, ship):
    backwards = e.mode == "back"
    angle = 180.0 if backwards else 0.0
    if ship.input_held("fire") > 0.0 and e.every("shot", SHOT_EVERY):
        e.fire(player.back_missile(e.world) if backwards else player.missile(e.world), angle, SHOT_SPEED)
    if e.mode in ("front", "back") and ship.input_released("fire") >= player.CHARGE_SECONDS:
        e.fire_spread("player_missile_charged", 3, CHARGED_SPREAD, angle, player.CHARGED_SPEED)


def side_of(e, ship):
    """The side the Force is on: behind the ship when its centre is left of the ship's centre."""
    return "back" if e.center_x() < ship.center_x() else "front"


def offset(side):
    return OFFSET_BACK if side == "back" else OFFSET_FRONT


def distance_to_side(e, ship):
    dx = ship.x() + offset(side_of(e, ship)) - e.x()
    dy = ship.y() + OFFSET_Y - e.y()
    return (dx * dx + dy * dy) ** 0.5
