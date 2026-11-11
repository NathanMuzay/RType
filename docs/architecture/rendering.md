# Rendering and audio

## Rendering

`engine_render` hides SFML behind small classes:

| Class | Role |
|---|---|
| `Window` | Opens the window, reads keyboard, mouse and gamepad events, turns them into `UiInput` (confirm, back, up, down, left, right, debug, console) |
| `IRenderer` | Interface used by every system: draw a sprite, a rectangle, a text, set a camera offset |
| `SfmlRenderer` | The SFML implementation. Draws in **world units** (1920 x 1080) and scales to the window size |
| `TextureManager`, `FontManager` | Load each file once and keep it |

Systems only know `IRenderer`, so another graphics library can replace SFML by writing one new class
(see [Replace a library](../guides/engine/replace-a-library.md)).

### What gets drawn

`RenderSystem` draws, each frame:

1. the background color of the game (`renderSystems` in the game file);
2. every `RectShape` and `Sprite`, with their `Rotation` (around their center) and their hit flash;
3. every `Text`;
4. the same again for entities tagged `Foreground` (HUD, menus, pause), drawn above everything.

The screen shake (`ShakeSystem`) moves the camera for the world layer only, so the HUD stays still.
Entities hidden by `VisibleWhen` or `Hidden` are skipped.

### Text scale

The `textScale` setting of `<game>_settings.json` multiplies every text size, for players who need bigger text.

## Audio

`engine_audio` has `IAudio` and its SFML implementation `SfmlAudio`:

| Feature | Driven by |
|---|---|
| Sound effects | `SoundCue` on a client prefab (played when the entity appears), button `hoverSound` and `clickSound`, server `PLAY_SOUND` events |
| Music | `music` of the current scene, or `MusicWhen` (for example the boss theme while `world.phase` is `boss`) |
| Volumes | `musicVolume` and `soundVolume` in the settings, or `set musicVolume 20` in the console |

Supported files are the ones SFML reads: `.wav`, `.ogg`, `.flac`, `.mp3`.

## Assets

Asset paths in the game file are relative to the `assets` folder copied next to the executable
(`game/assets/rtype/` for R-Type, `game2/assets/skyhop/` for Skyhop). `AssetPath` finds it next to the executable, so the game can
be started from any working directory. See [Asset pipeline](../guides/content/assets.md).
