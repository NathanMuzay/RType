#!/usr/bin/env python3
"""Generates the stage backgrounds in game/assets/rtype/backgrounds/.

Each image is a 1920x1080 RGBA layer drawn over the stage's plain background colour and under the
starfield. The noise is filtered in the Fourier domain, so it is periodic: the left and right edges
match and the image can scroll forever. Fixed seeds make every run produce the same files.

    python3 scripts/generate_backgrounds.py

Requires numpy and Pillow.
"""

from pathlib import Path

import numpy as np
from PIL import Image

WIDTH = 1920
HEIGHT = 1080
OUTPUT = Path(__file__).resolve().parent.parent / "game" / "assets" / "rtype" / "backgrounds"


def fractal_noise(seed, roughness, scale=1.0):
    """Periodic noise in [0, 1]: white noise shaped by a 1/f^roughness spectrum."""
    rng = np.random.default_rng(seed)
    white = rng.standard_normal((HEIGHT, WIDTH))
    fy = np.fft.fftfreq(HEIGHT)[:, None] * HEIGHT / scale
    fx = np.fft.fftfreq(WIDTH)[None, :] * WIDTH / scale
    frequency = np.sqrt(fx * fx + fy * fy)
    frequency[0, 0] = 1.0
    field = np.real(np.fft.ifft2(np.fft.fft2(white) / frequency**roughness))
    field -= field.min()
    return field / field.max()


def ramp(values, stops):
    """Maps values in [0, 1] to colours through (position, (r, g, b)) stops."""
    positions = np.array([position for position, _ in stops])
    colours = np.array([colour for _, colour in stops], dtype=float)
    return np.stack([np.interp(values, positions, colours[:, c]) for c in range(3)], axis=-1)


def vertical(top, bottom):
    """A column of weights going from top to bottom, broadcast over the width."""
    return np.linspace(top, bottom, HEIGHT)[:, None]


def periodic_rays(count, sharpness):
    """Soft vertical light shafts whose pattern repeats exactly across the width."""
    x = np.arange(WIDTH)[None, :] / WIDTH
    rays = (0.5 + 0.5 * np.cos(2 * np.pi * count * x)) ** sharpness
    wobble = 0.5 + 0.5 * np.cos(2 * np.pi * (count // 2) * x + 1.3)
    return rays * (0.6 + 0.4 * wobble)


def embers(seed, count, alpha):
    """Small bright dots; their brightness is returned as an extra alpha layer."""
    rng = np.random.default_rng(seed)
    layer = np.zeros((HEIGHT, WIDTH))
    xs = rng.integers(0, WIDTH, count)
    ys = (HEIGHT * (1 - rng.random(count) ** 2)).astype(int).clip(0, HEIGHT - 1)
    layer[ys, xs] = rng.uniform(0.4, 1.0, count)
    for dx, dy in ((1, 0), (0, 1), (-1, 0), (0, -1)):
        layer = np.maximum(layer, 0.5 * np.roll(np.roll(layer, dx, axis=1), dy, axis=0))
    return layer * alpha


def paradise():
    """Stage 1: luminous golden clouds and soft light falling from above."""
    clouds = fractal_noise(101, 2.1)
    detail = fractal_noise(102, 1.7)
    density = np.clip((0.7 * clouds + 0.3 * detail - 0.35) * 2.0, 0, 1)
    rgb = ramp(density, [(0.0, (230, 190, 140)), (0.5, (255, 225, 170)), (1.0, (255, 252, 240))])
    light = periodic_rays(5, 4) * vertical(0.7, 0.0)
    rgb = rgb + light[..., None] * np.array([255, 240, 200]) * 0.3
    glow = vertical(0.35, 0.0)
    alpha = np.clip(density * 0.6 + light * 0.3 + glow, 0, 1)
    return rgb, alpha


def sky():
    """Stage 2: layered white and pale blue clouds, thicker near the bottom."""
    high = fractal_noise(201, 2.1)
    low = fractal_noise(202, 1.7)
    density = np.clip((0.5 * high + 0.5 * low - 0.42) * 2.6, 0, 1) * vertical(0.55, 1.0)
    rgb = ramp(density, [(0.0, (120, 160, 210)), (0.6, (215, 230, 250)), (1.0, (255, 255, 255))])
    haze = vertical(0.0, 0.3)
    alpha = np.clip(density * 0.85 + haze, 0, 1)
    rgb = rgb * (alpha - haze)[..., None] / np.maximum(alpha, 1e-6)[..., None] + (
        np.array([170, 200, 235]) * (haze / np.maximum(alpha, 1e-6))[..., None])
    return rgb, alpha


def hell():
    """Stage 3: smoke and flames rising from a glowing floor, with drifting embers."""
    smoke = fractal_noise(301, 2.0)
    flames = fractal_noise(302, 1.6)
    heat = np.clip(vertical(0.05, 0.85) + 0.7 * (flames - 0.5), 0, 1)
    rgb = ramp(heat, [(0.0, (70, 10, 6)), (0.5, (160, 30, 12)), (0.8, (220, 80, 20)), (1.0, (245, 140, 40))])
    darkness = np.clip((smoke - 0.4) * 1.8, 0, 1)
    rgb = rgb * (1 - 0.65 * darkness[..., None])
    sparks = embers(303, 900, 1.0)
    rgb = np.maximum(rgb, sparks[..., None] * np.array([255, 170, 60]))
    alpha = np.clip(heat * 0.7 + darkness * 0.25 + sparks, 0, 1)
    return rgb, alpha


def save(name, rgb, alpha):
    pixels = np.dstack([np.clip(rgb, 0, 255), np.clip(alpha, 0, 1) * 255]).astype(np.uint8)
    path = OUTPUT / name
    Image.fromarray(pixels, "RGBA").save(path, optimize=True)
    print(f"wrote {path.relative_to(OUTPUT.parent.parent)}")


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    save("stage1_paradise.png", *paradise())
    save("stage2_sky.png", *sky())
    save("stage3_hell.png", *hell())


if __name__ == "__main__":
    main()
