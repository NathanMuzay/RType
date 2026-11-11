# Running

Executables are in `build/bin/` (Linux) or `out\windows\bin\Release\` (Windows). They find their `data`,
`scripts` and `assets` folders next to themselves, so they can be started from any directory.

## R-Type on one machine

Start the server, then one client per player, each in its own terminal:

```sh
./r-type_server
./r-type_client
./r-type_client
```

On Linux, `scripts/play.sh 2` builds and starts a server and two clients.

In the client: **CONNECT**, create a room (name, optional code, rules), the other players join it from the
room list, the owner presses **START GAME**.

## R-Type on a local network

```sh
./r-type_server -p 4242                 # on the host, prints the address to use
./r-type_client -h 192.168.1.67         # on each player's machine
```

Firewall and WSL notes: [Network play](network-play.md).

## Skyhop

```sh
./skyhop
```

No server needed. **JOUER** (play) starts, Space or gamepad A flies, Escape pauses.

## Command-line options

| Program | Option | Default | Meaning |
|---|---|---|---|
| server | `-p <port>` | 4242 | UDP port |
| client | `-h <address>` | `serverAddress` of the settings | Server IPv4 address |
| client | `-p <port>` | 4242 | Server port |
| all | `-g <game>` | the game it was built for | Load another game from the same `data` folder |
| all | `--help` | - | Print the usage |

Invalid arguments make the program exit with code 84.

## Controls

| Action | Keyboard | Gamepad |
|---|---|---|
| Move / menu focus | Arrow keys | Left stick, D-pad |
| Fire / confirm | Space / Enter | A (button 0) |
| Action (R-Type: Force) | Left Shift | B (button 1) |
| Pause / back | Escape | Select (button 6) |
| Console | F1 | - |
| Debug overlay | F3 | - |

Keys, volumes and text size: [Controls](../guides/content/controls.md) and the
[Settings file reference](../reference/game-file.md#settings-file-reference).

## R-Type in short

- Three stages, each ending with a boss. Stage 3 has terrain.
- Bydos, pods, hoppers, turrets, mines, snakes... each with its own pattern.
- Power-ups: double shot, spread, heal, shield, and the Force (Action launches and recalls it).
- Hold fire to charge a big shot.
- Room rules: difficulty, friendly fire, cheats.
- When every player is out of lives: game over, then a new round.
- `Ctrl+C` stops the server cleanly and tells the connected clients.
