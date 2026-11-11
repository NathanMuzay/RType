# Asset pipeline

## Where assets live

| Game | Source folder | Copied to |
|---|---|---|
| R-Type | `game/assets/rtype/` (`sprites/`, `backgrounds/`, `sounds/`, `music/`, `fonts/`) | `<build>/bin/assets/rtype/` |
| Skyhop | `game2/assets/skyhop/` | `<build>/bin/assets/skyhop/` |

`engine_add_game` copies the `ASSETS` folder next to the executables at every build. Paths in the game file are
relative to that copied `assets` folder. Put a new game's assets in a subfolder named after the game, so that
two games never overwrite each other's files.

## Formats

| Kind | Formats | Notes |
|---|---|---|
| Images | `.png`, `.gif`, `.jpg`, `.bmp` | Prefer PNG with transparency. Sprite sheets are cut with `Sprite.frame` |
| Sounds | `.wav`, `.ogg`, `.flac`, `.mp3` | Short effects, loaded in memory |
| Music | `.wav`, `.ogg`, `.flac`, `.mp3` | Streamed, OGG keeps files small |
| Fonts | `.ttf`, `.otf` | One key per font |

## Adding an image

1. Copy it into the assets folder.
2. Declare it: `"textures": {"pods": "sprites/enemies/r-typesheet11.gif"}`.
3. Use it: `"Sprite": {"texture": "pods", "frame": [0, 0, 33, 36], "scale": 3}`.
4. Optional: add `"pods"` to `preload`.

For animations, place the frames side by side with the same size and use `SpriteAnimation` with
`stepX` = frame width (plus spacing, if any).

## Licenses

Only use assets you are allowed to use, and keep their license next to them.

| Source | License | Use |
|---|---|---|
| [Kenney](https://kenney.nl) | CC0 (public domain) | Skyhop graphics and sounds |
| [OpenGameArt](https://opengameart.org) | Varies (CC0, CC-BY, GPL): check each asset | - |
| Assets made by the team | Owned by the team | - |

Rules:

- Never commit ripped assets from commercial games other than the ones the subject provides.
- For CC-BY assets, credit the author in the README and in the game folder.
- Keep the original license file (`LICENSE_*.txt`) in the asset folder.
