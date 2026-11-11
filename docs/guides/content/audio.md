# Audio

## Declare the files

In the game file, each sound and music gets a key:

```json
"sounds": {"shot": "sounds/LASER_GUN.mp3", "explosion": "sounds/EXPLOSION.mp3", "click": "sounds/CLICK.mp3"},
"music": {"main": "music/MAIN_THEME.mp3", "title": "music/TITLE_THEME.mp3", "boss": "music/MAIN_THEME.mp3"}
```

Sounds are loaded in memory (short effects). Music is streamed from the file (long tracks, looped).
Formats: `.wav`, `.ogg`, `.flac`, `.mp3`. Add frequently used sounds to `preload` to avoid a hitch the first
time they play.

## Play a sound

| When | How |
|---|---|
| An entity appears on the client | `"SoundCue": {"sound": "shot"}` in its client prefab |
| An entity dies | Put a `SoundCue` in its `DeathEffect` prefab (R-Type: `explosion`) |
| A button is hovered or clicked | `"hoverSound"` and `"clickSound"` of `Button` |
| A game event on the server | `world.play_sound("warning")` in a script, the name must be in `eventSounds` |

`eventSounds` is the list of sounds the server may trigger. They are sent as indexes in `PLAY_SOUND`:

```json
"eventSounds": ["warning", "powerup", "bossDeath"]
```

## Music

Each scene plays its `music`. To change music during a scene, add an entity with `MusicWhen`:

```json
{"prefab": "hud_line", "components": {"MusicWhen": {"key": "world.phase", "value": "boss", "music": "boss"}}},
{"prefab": "hud_line", "components": {"MusicWhen": {"key": "game.phase", "value": "playing", "music": "main"}}}
```

The scene needs the `Music` system.

## Volumes

| Setting | Default | Where |
|---|---|---|
| `musicVolume` | 40 | `<game>_settings.json`, or `set musicVolume 20` in the console |
| `soundVolume` | 70 | `<game>_settings.json`, or `set soundVolume 50` in the console |

Values go from 0 to 100.
