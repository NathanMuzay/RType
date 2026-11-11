# Tilemaps and terrain

A `Tilemap` draws a level as rows of characters. Each character is a prefab, and the map scrolls to the left.
R-Type's stage 2 uses it for a short tunnel, with cannons fixed on its floor and ceiling.

## Writing a map

Shortened from `game/data/rtype_server_prefabs.json` (the real map has 13 rows, 11 of them empty):

```json
"terrain_stage2": {
    "Transform": {},
    "Tilemap": {
        "rows": [
            "##x###xx####x###x####xx###x##########x###",
            "",
            "",
            "#####x####xx#####x#####x#####xx####x#####"
        ],
        "legend": {"#": "tile_wall", "x": "tile_break"},
        "tileSize": 80,
        "speed": 120
    }
}
```

| Field | Meaning |
|---|---|
| `rows` | One string per row, from top to bottom. Each character is one tile |
| `legend` | Character to server prefab. Characters not in the legend are empty |
| `tileSize` | Size of a tile in world units. Row `r`, column `c` is at `(c * tileSize, r * tileSize)` from the map position |
| `speed` | Scrolling speed to the left |

Empty rows (`""`) are allowed. Rows may have different lengths.

Spawn the map like any prefab, from a script or the server scene:

```python
world.spawn("terrain_stage2", 1920.0, 0.0)
```

## How it runs

The `Tilemap` system spawns each column only when it is about to enter the world, gives every tile the
scrolling velocity, and tiles leave through their `DespawnOffscreen`. A long map therefore costs only the
visible tiles. The `Tilemap` system must be in `serverSystems`.

## Tile prefabs

```json
"tile_wall": {
    "Transform": {}, "Velocity": {"x": 0, "y": 0},
    "Collider": {"width": 80, "height": 80, "layer": "terrain", "hits": ["player"]},
    "Solid": {},
    "DespawnOffscreen": {"minX": -500, "minY": -400, "maxX": 2500, "maxY": 1480},
    "Replicated": {"kind": 33}
}
```

| Component | Effect |
|---|---|
| `Solid` | Entities with `Pushable` (R-Type ships) are pushed out instead of passing through |
| `Health` + `ScoreValue` | A breakable tile (R-Type `tile_break`) |
| `ContactDamage` | A dangerous tile (spikes) |

## Platforms and ground

Scripts can test whether an entity stands on a `Solid` tile:

```python
if e.on_ground() and e.input_pressed("fire"):
    e.set_velocity(e.vx(), -900.0)
```

`on_ground()` checks a thin box just below the entity's collider against every `Solid` entity, and returns
`True` only when the contact is from above. It is the base of any platform jump.
