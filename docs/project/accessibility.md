# Accessibility

This page lists, for each category of disability, what the games and the documentation provide, and what is
planned. Features are inspired by settings found in current games: remapping, toggle actions, assist modes,
scalable text, visual cues for sounds.

All options are **client settings**, changed in game from the **SETTINGS** screen (main menu and pause menu) and
saved automatically. They never change the rules of the server, so every player of a room still plays the same
game.

## Physical and motor disabilities

| Feature | How | Status |
|---|---|---|
| Fully remappable keyboard | SETTINGS screen: press Enter on an action, then the new key. Conflicts are swapped | Available |
| Gamepad for menus and game | Stick, D-pad, buttons set in the settings file | Available |
| Adjustable stick dead zone | `gamepad.deadzone` in the settings file (worn sticks, tremors) | Available |
| Toggle fire instead of holding | SETTINGS: AUTO FIRE ON | Available |
| Hold to fire continuously, no rapid tapping | Default behaviour | Available |
| Every menu usable without mouse | Focus navigation with arrows, stick or D-pad, Enter or A to confirm | Available |
| Pause at any time | Escape or Select | Available (solo); online the game keeps running for others |
| Easier difficulty | Room rule `difficulty: EASY` (fewer enemies, weaker bosses) | Available |
| Invulnerability after respawn | 2 seconds, to recover calmly | Available |
| One-button game | Skyhop is played with a single button | Available |
| Adjustable game speed | - | Planned |

## Audio and visual disabilities

| Feature | How | Status |
|---|---|---|
| Scalable text | SETTINGS: TEXT SIZE 0.8 to 1.3, applied at once | Available |
| Window scaled to any size | The 1920 x 1080 world scales to the window | Available |
| Players not told apart by color only | `P1` to `P4` label above each ship (SETTINGS: PLAYER NUMBERS) | Available |
| Hits shown visually | Hit flash on every damaged entity, health bars in the HUD | Available |
| Important sounds have a visual equivalent | Boss arrival: warning sign and "BOSS" bar; power-ups: HUD text of the weapon | Available |
| Separate music and sound volumes | SETTINGS screen, or the console | Available |
| High-contrast HUD | Bright text on dark panels | Available |
| Colorblind-friendly palettes | - | Planned |
| Option to disable screen shake and flashes | Shake strength is set per game in its scene (`Shake` system) | Planned as a player setting |
| Screen reader support for menus | - | Not planned (SFML has no accessibility API) |

## Mental and cognitive disabilities

| Feature | How | Status |
|---|---|---|
| Simple goals | Avoid, shoot, survive; Skyhop: one button | Available |
| Always-visible state | HUD with score, health, lives, weapon, stage progress, boss health | Available |
| Clear phases | "STAGE N" intro, "BOSS", "VICTORY", "GAME OVER" screens | Available |
| On-screen hints | Skyhop shows how to play on the start screen | Available |
| Automatic restart | A new round starts by itself after a game over | Available |
| Readable errors | Lobby errors in plain words ("WRONG CODE", "ROOM FULL") | Available |
| Short in-game tutorial for R-Type | - | Planned |

## Documentation accessibility

- Text documentation in Markdown, rendered as a website with search and navigation, readable by screen readers.
- Clear heading hierarchy, tables instead of text in images.
- Diagrams written as Mermaid text, so their content is readable in the source and by assistive tools.
- Short pages, one topic per page, a [glossary](../reference/glossary.md) and a [FAQ](../getting-started/faq.md).
- The MkDocs Material theme supports keyboard navigation and has a high-contrast color scheme.

## For developers

- Remapping goes through `engine::input::KeyBindings`: game code reads actions, never physical keys.
- Visual options belong to client systems and the renderer, never to gameplay components.
- Any new sound that carries information must have a visual equivalent (a HUD text, an icon or a flash).
- Any new setting is added to the [Settings file reference](../reference/game-file.md#settings-file-reference).
