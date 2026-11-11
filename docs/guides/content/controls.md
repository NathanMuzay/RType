# Controls and gamepad

## Settings screen

Every setting can be changed in game: **SETTINGS** in the main menu or in the pause menu (Skyhop: **REGLAGES**).

| Line | Effect |
|---|---|
| Music volume, sound volume | 0 to 100, applied at once |
| Text size | 0.8 to 1.3, for players who need bigger text |
| Auto fire | OFF: hold the fire key to shoot. ON: press it once to start shooting, once to stop (no need to keep a finger on the key). The R-Type charged shot needs a long press, so it is not available with auto fire ON |
| Player numbers | Shows P1, P2, P3, P4 above the ships, so players are not told apart by color only |
| Move up/down/left/right, fire, Force | Enter (or click), then press the new key. Escape cancels |
| Defaults | Back to the game's default settings |

Click the `<` and `>` arrows, or use left and right on the selected line, to change a value. If the new key is already used by another action, the two keys are swapped.

Changes are saved at once in `data/<game>_user_settings.json`, next to the executable. This file overrides
`<game>_settings.json` and is never overwritten by a build. Delete it to go back to the defaults.

## Making a settings screen for a game

The screen is an ordinary scene named `settings`, with the systems `KeyBind` and `SettingSlider` and the
components of the same names ([Component reference](../../reference/components.md)):

```json
{"prefab": "ui_element", "components": {
    "Transform": {"x": 140, "y": 200},
    "RectShape": {"width": 780, "height": 64, "color": [20, 30, 70, 230]},
    "Text": {"font": "hud", "size": 32, "color": [255, 255, 255, 255], "offsetX": 24, "offsetY": 14},
    "BoundText": {"format": "MUSIC VOLUME   < {setting.musicVolume} >"},
    "Button": {"action": "", "width": 780, "height": 64, "normal": [20, 30, 70, 230], "hover": [40, 80, 170, 240], "pressed": [80, 200, 255, 255]},
    "Focusable": {"order": 0},
    "SettingSlider": {"setting": "musicVolume", "min": 0, "max": 100, "step": 10}}}
```

Settings: `musicVolume`, `soundVolume`, `textScale`, `deadzone`, `toggleFire` (`OFF`/`ON`), `showPlayerLabels`
(`ON`/`OFF`), `key.<action>`, `pad.fire`, `pad.action`. A button with the action `settings` opens the scene,
`back` returns to the previous scene, `settings_reset` restores the defaults.

## Settings file

## Keyboard

Keys are set in `data/<game>_settings.json`:

```json
"controls": {"up": "Up", "down": "Down", "left": "Left", "right": "Right", "fire": "Space", "action": "LShift"},
"toggleFire": false
```

Accepted names: `A` to `Z`, `0` to `9`, `Up`, `Down`, `Left`, `Right`, `Space`, `Enter`, `Tab`, `Backspace`,
`LShift`, `RShift`, `LControl`, `RControl`, `LAlt`, `RAlt`. Keys are read by position (scancode), so `W` is the
same physical key on QWERTY and AZERTY keyboards.

With `"toggleFire": true`, fire is switched on and off by a press instead of being held.

Fixed keys:

| Key | Effect |
|---|---|
| Escape | The scene's `escapeAction` (pause, back, quit) |
| Enter | Confirm the focused button |
| Arrows | Move the focus in menus |
| F1 | Console |
| F3 | Debug overlay |

## Gamepad

Any gamepad recognised by the system works (XInput and DirectInput on Windows, joystick devices on Linux).
Gamepad number 0 (the first one connected) is used.

```json
"gamepad": {"fire": 0, "action": 1, "deadzone": 35}
```

| Input | In game | In menus |
|---|---|---|
| Left stick, D-pad | Move | Move the focus |
| Button `fire` (0, A on Xbox pads) | Fire | Confirm |
| Button `action` (1, B on Xbox pads) | Action | - |
| Select / Back (button 6) | Pause | Back |

`deadzone` (0 to 100) is the part of the stick travel ignored, to avoid drifting with worn sticks.
Button numbers depend on the pad: if a button does not respond, try other numbers in the settings file.

## What the server receives

Whatever the device, the client sends the same 6 buttons (up, down, left, right, fire, action) in `INPUT`
messages ([Protocol](../../reference/protocol.md)). What `action` does is decided by the game's scripts.
