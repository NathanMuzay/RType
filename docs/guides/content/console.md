# Console and cheats

Press **F1** in a room or in game to open the console, type a command and press Enter. F1 again closes it.

## Who can use it

| Command type | Allowed for |
|---|---|
| Local commands (below) | Everyone |
| Server commands | The **room owner** only, and only if the room was created with **CHEATS: ON** |

Server commands typed in the room before the game starts are queued and run when it starts.
Every answer is printed in the console, prefixed with the player number (`[P1] god mode on`).

## Local commands

| Command | Effect |
|---|---|
| `help` | Lists the commands |
| `clear` | Clears the console |
| `cvars` | Lists the settings that can be changed |
| `get <cvar>` | Shows a setting |
| `set <cvar> <value>` | Changes a setting: `musicVolume`, `soundVolume` (0 to 100), `showMetrics` (0 or 1) |
| `plugins` | Lists the loaded plugins and their systems |
| `reload` | Reloads the plugins now |

## R-Type server commands

Defined in `game/scripts/console.py`:

| Command | Effect |
|---|---|
| `help` | Lists the commands |
| `god` | Turns invulnerability on, or off when it is already on |
| `heal` | Restores your ship's health |
| `give <double\|spread\|heal\|shield\|force>` | Gives a power-up |
| `spawn <prefab>` | Spawns any server prefab in front of you (`spawn boss_vessel`) |
| `skip` | Skips to the next step of the stage (boss, then next stage) |

## Writing commands for a game

The game file names the console script and the rule that enables it:

```json
"files": {"...": "...", "console": "console.py"},
"consoleRule": "cheats",
"freeCommands": ["help", "next", "restart"]
```

When the room's `consoleRule` is OFF the server refuses every command except the ones listed in
`freeCommands`. R-Type uses them for the end-of-stage panel: a UI button whose action is
`command:<text>` sends `<text>` like a typed command (`"action": "command:next"`), and only the room
owner's commands are accepted.

The script defines one function, called for every command:

```python
def run(world, ship, text):
    words = text.split()
    if words[0] == "god" and ship is not None:
        ship.set_invulnerable(100000.0)
        return "god mode on"
    return "unknown command, try help"
```

| Argument | Value |
|---|---|
| `world` | The world object of the room |
| `ship` | The sender's avatar, or `None` if it is not in play |
| `text` | The command line |

The returned text is sent back to the sender. Without `consoleRule`, commands are always allowed for the owner.
