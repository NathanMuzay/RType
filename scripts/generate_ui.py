#!/usr/bin/env python3
"""Generates R-Type's interface art: logo, buttons, HUD frames, menu backdrop and intro chimes.

Everything is drawn from code, so the style stays consistent and every run gives the same files:

    game/assets/rtype/sprites/ui/logo.png          "R-TYPE" title, chrome letters with a cyan glow
    game/assets/rtype/sprites/ui/buttons.png       buttons, one row per size of BUTTON_SIZES, columns normal / hover / pressed
    game/assets/rtype/sprites/ui/hud.png           player panel, bar frames and a 4-frame twinkling star
    game/assets/rtype/backgrounds/menu_space.png   dark nebula with stars and a planet, for the title screen
    game/assets/rtype/backgrounds/stage2_interior.png  inside of a Bydo warship for stage 2, tiles horizontally
    game/assets/rtype/sprites/projectiles/boss_beam.png  3 frames of a plasma beam that tiles horizontally
    game/assets/rtype/sprites/ui/warnings.png  blinking warnings: a red laser line and a "!" (2 frames each)
    game/assets/rtype/sprites/ui/warnings_vertical.png  the red laser line, standing up
    game/assets/rtype/sounds/INTRO_CHIMES.wav      star chimes timed on the intro credits

    python3 scripts/generate_ui.py

Requires Pillow. The logo uses the game font, game/assets/rtype/fonts/bytebounce.medium.ttf.
"""

import math
import random
import struct
import wave
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent / "game" / "assets" / "rtype"
UI = ROOT / "sprites" / "ui"
FONT = ROOT / "fonts" / "bytebounce.medium.ttf"
LOGO_SLANT = 0.18

CYAN = (80, 200, 255)
DEEP = (10, 16, 44)

BUTTON_SIZES = ((600, 80), (500, 80), (700, 80), (700, 70), (900, 64), (520, 80), (200, 80))
BUTTON_CUT = 18

PANEL = (440, 122)
BAR_FRAMES = {"health": (300, 14), "progress": (600, 14), "boss": (1000, 20)}
BAR_BORDER = 4
STAR_SIZE = 32

INTRO_CHIMES = (0.8, 1.8, 2.6, 3.4, 4.2)
INTRO_TITLE = 5.2
SAMPLE_RATE = 22050


def chamfer(width, height, cut):
    """Polygon of a rectangle with its top-left and bottom-right corners cut."""
    return [(cut, 0), (width - 1, 0), (width - 1, height - 1 - cut), (width - 1 - cut, height - 1), (0, height - 1), (0, cut)]


def vertical_gradient(width, height, top, bottom):
    image = Image.new("RGBA", (width, height))
    draw = ImageDraw.Draw(image)
    for y in range(height):
        t = y / max(1, height - 1)
        colour = tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(len(top)))
        draw.line([(0, y), (width, y)], fill=colour)
    return image


def button(width, height, state):
    """One button: dark glass body with a neon border; brighter when hovered, lit when pressed."""
    border = {"normal": CYAN + (150,), "hover": CYAN + (255,), "pressed": (220, 245, 255, 255)}[state]
    top = {"normal": (24, 34, 78, 235), "hover": (34, 64, 140, 245), "pressed": (70, 170, 235, 255)}[state]
    bottom = {"normal": (8, 12, 34, 235), "hover": (14, 28, 80, 245), "pressed": (30, 90, 170, 255)}[state]
    shape = Image.new("L", (width, height), 0)
    ImageDraw.Draw(shape).polygon(chamfer(width, height, BUTTON_CUT), fill=255)
    body = vertical_gradient(width, height, top, bottom)
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    image.paste(body, (0, 0), shape)
    draw = ImageDraw.Draw(image)
    draw.line(chamfer(width, height, BUTTON_CUT) + [(BUTTON_CUT, 0)], fill=border, width=3)
    draw.line([(BUTTON_CUT + 6, 6), (width - 40, 6)], fill=(255, 255, 255, 60), width=1)
    draw.rectangle([width - 34, height - 14, width - 22, height - 10], fill=border)
    draw.rectangle([width - 18, height - 14, width - 10, height - 10], fill=border)
    if state != "normal":
        glow = image.filter(ImageFilter.GaussianBlur(4))
        image = Image.alpha_composite(glow, image)
    return image


def buttons():
    """Returns {(width, height): (normal, hover, pressed)} frames of the sheet."""
    states = ("normal", "hover", "pressed")
    width = max(size[0] for size in BUTTON_SIZES) * 3
    sheet = Image.new("RGBA", (width, sum(size[1] for size in BUTTON_SIZES)), (0, 0, 0, 0))
    frames = {}
    y = 0
    for button_width, button_height in BUTTON_SIZES:
        row = []
        for column, state in enumerate(states):
            sheet.alpha_composite(button(button_width, button_height, state), (column * button_width, y))
            row.append([column * button_width, y, button_width, button_height])
        frames[(button_width, button_height)] = row
        y += button_height
    sheet.save(UI / "buttons.png")
    return frames


def panel():
    width, height = PANEL
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    shape = Image.new("L", (width, height), 0)
    ImageDraw.Draw(shape).polygon(chamfer(width, height, 14), fill=255)
    image.paste(vertical_gradient(width, height, (12, 20, 52, 190), (4, 6, 20, 150)), (0, 0), shape)
    draw = ImageDraw.Draw(image)
    draw.line(chamfer(width, height, 14) + [(14, 0)], fill=CYAN + (170,), width=2)
    draw.rectangle([14, 0, 150, 3], fill=CYAN + (255,))
    draw.rectangle([width - 60, height - 4, width - 18, height - 1], fill=CYAN + (255,))
    return image


def bar_frame(inner_width, inner_height, accent):
    width = inner_width + 2 * BAR_BORDER
    height = inner_height + 2 * BAR_BORDER
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.rectangle([0, 0, width - 1, height - 1], outline=(10, 14, 30, 255), width=BAR_BORDER)
    draw.rectangle([1, 1, width - 2, height - 2], outline=accent + (200,), width=1)
    for x in range(BAR_BORDER + inner_width // 10, BAR_BORDER + inner_width, inner_width // 10):
        draw.line([(x, BAR_BORDER), (x, BAR_BORDER + 2)], fill=(10, 14, 30, 200))
    draw.rectangle([0, 0, 6, height - 1], fill=accent + (255,))
    draw.rectangle([width - 7, 0, width - 1, height - 1], fill=accent + (255,))
    return image


def star(frame):
    """Four-point star; the frames grow then shrink so a looping animation twinkles."""
    size = STAR_SIZE
    reach = (6, 11, 15, 10)[frame]
    image = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    centre = size // 2
    draw.polygon([(centre, centre - reach), (centre + 2, centre), (centre, centre + reach), (centre - 2, centre)], fill=(255, 255, 255, 255))
    draw.polygon([(centre - reach, centre), (centre, centre - 2), (centre + reach, centre), (centre, centre + 2)], fill=(255, 255, 255, 255))
    glow = image.filter(ImageFilter.GaussianBlur(2))
    tint = Image.new("RGBA", image.size, CYAN + (0,))
    tint.putalpha(glow.getchannel("A"))
    return Image.alpha_composite(tint, image)


def hud():
    width, height = PANEL
    frames = {name: bar_frame(w, h, (255, 70, 70) if name == "boss" else CYAN) for name, (w, h) in BAR_FRAMES.items()}
    sheet_width = max(width, max(frame.size[0] for frame in frames.values()), STAR_SIZE * 4)
    sheet_height = height + sum(frame.size[1] for frame in frames.values()) + STAR_SIZE
    sheet = Image.new("RGBA", (sheet_width, sheet_height), (0, 0, 0, 0))
    sheet.alpha_composite(panel(), (0, 0))
    y = height
    layout = {"panel": (0, 0, width, height)}
    for name, frame in frames.items():
        sheet.alpha_composite(frame, (0, y))
        layout[name] = (0, y) + frame.size
        y += frame.size[1]
    for index in range(4):
        sheet.alpha_composite(star(index), (index * STAR_SIZE, y))
    layout["star"] = (0, y, STAR_SIZE, STAR_SIZE)
    sheet.save(UI / "hud.png")
    return layout


def logo():
    font = ImageFont.truetype(str(FONT), 230)
    text = "R-TYPE"
    left, top, right, bottom = font.getbbox(text)
    pad = 40
    text_height = bottom - top
    width = right - left + 2 * pad + int(text_height * LOGO_SLANT)
    height = text_height + 2 * pad + 30
    mask = Image.new("L", (width, height), 0)
    ImageDraw.Draw(mask).text((pad - left + int(text_height * LOGO_SLANT), pad - top), text, font=font, fill=255)
    mask = mask.transform(mask.size, Image.AFFINE, (1, LOGO_SLANT, 0, 0, 1, 0), Image.NEAREST)
    letters = vertical_gradient(width, height, (240, 250, 255, 255), (40, 90, 200, 255))
    band = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    middle = pad + text_height // 2
    ImageDraw.Draw(band).rectangle([0, middle - 3, width, middle + 3], fill=(255, 255, 255, 110))
    outline = mask.filter(ImageFilter.MaxFilter(9))
    glow = outline.filter(ImageFilter.GaussianBlur(16))
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    image.paste(Image.new("RGBA", (width, height), CYAN + (255,)), (0, 0), glow.point(lambda value: int(value * 0.7)))
    image.paste(Image.new("RGBA", (width, height), DEEP + (255,)), (0, 0), outline)
    image.paste(letters, (0, 0), mask)
    image.paste(band, (0, 0), ImageChops.multiply(mask, band.getchannel("A")))
    draw = ImageDraw.Draw(image)
    line_y = pad + text_height + 16
    draw.polygon([(pad + 30, line_y), (width - pad, line_y), (width - pad - 10, line_y + 7), (pad + 20, line_y + 7)], fill=(255, 70, 70, 255))
    draw.rectangle([pad + 10, line_y + 12, width // 2, line_y + 14], fill=CYAN + (255,))
    image.save(UI / "logo.png")
    return image.size


def nebula(seed, scale, colour, strength):
    rng = random.Random(seed)
    small = Image.new("L", (1920 // scale + 2, 1080 // scale + 2))
    small.putdata([rng.randrange(256) for _ in range(small.size[0] * small.size[1])])
    cloud = small.resize((1920 + 2 * scale, 1080 + 2 * scale), Image.BICUBIC).crop((scale, scale, 1920 + scale, 1080 + scale))
    cloud = cloud.filter(ImageFilter.GaussianBlur(scale / 3)).point(lambda value: max(0, value - 110) * strength)
    layer = Image.new("RGBA", (1920, 1080), colour + (255,))
    layer.putalpha(cloud.point(lambda value: min(255, value)))
    return layer


def menu_backdrop():
    image = vertical_gradient(1920, 1080, (3, 3, 12, 255), (8, 5, 24, 255))
    for seed, scale, colour, strength in ((1, 320, (60, 26, 120), 1.0), (2, 200, (18, 60, 140), 0.8), (3, 120, (120, 30, 100), 0.5)):
        image = Image.alpha_composite(image, nebula(seed, scale, colour, strength))
    rng = random.Random(7)
    draw = ImageDraw.Draw(image)
    for _ in range(1100):
        x, y = rng.randrange(1920), rng.randrange(1080)
        level = rng.choice((70, 110, 160, 220, 255))
        draw.point((x, y), fill=(level, level, min(255, level + 30), 255))
    for _ in range(40):
        x, y = rng.randrange(1920), rng.randrange(1080)
        draw.rectangle([x, y, x + 1, y + 1], fill=(220, 235, 255, 255))
    image = Image.alpha_composite(image, planet(1700, 930, 210))
    vignette = Image.new("L", (1920, 1080), 0)
    ImageDraw.Draw(vignette).ellipse([-300, -250, 2220, 1330], fill=255)
    vignette = vignette.filter(ImageFilter.GaussianBlur(160))
    dark = Image.new("RGBA", (1920, 1080), (0, 0, 0, 255))
    dark.putalpha(vignette.point(lambda value: int((255 - value) * 0.8)))
    image = Image.alpha_composite(image, dark)
    image.convert("RGB").save(ROOT / "backgrounds" / "menu_space.png")


def planet(centre_x, centre_y, radius):
    """A planet lit from the upper left, with soft cloud bands and a thin cyan atmosphere."""
    size = radius * 2
    body = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    pixels = body.load()
    rng = random.Random(11)
    bands = [rng.uniform(-0.15, 0.15) for _ in range(24)]
    for y in range(size):
        for x in range(size):
            dx = (x - radius) / radius
            dy = (y - radius) / radius
            distance = dx * dx + dy * dy
            if distance > 1.0:
                continue
            depth = math.sqrt(1.0 - distance)
            light = max(0.0, -0.55 * dx - 0.45 * dy + 0.7 * depth)
            band = bands[int((dy + 1.0) / 2.0 * (len(bands) - 1))] * 0.4
            shade = max(0.03, min(1.0, light**1.6 + band * light))
            pixels[x, y] = (int(40 + 90 * shade), int(26 + 50 * shade), int(70 + 120 * shade), 255)
    layer = Image.new("RGBA", (1920, 1080), (0, 0, 0, 0))
    disc = Image.new("L", (1920, 1080), 0)
    ImageDraw.Draw(disc).ellipse([centre_x - radius, centre_y - radius, centre_x + radius, centre_y + radius], fill=255)
    halo = Image.new("RGBA", (1920, 1080), CYAN + (0,))
    halo.putalpha(ImageChops.subtract(disc.filter(ImageFilter.GaussianBlur(14)), disc).point(lambda value: min(255, value * 2)))
    layer = Image.alpha_composite(layer, halo)
    layer.alpha_composite(body, (centre_x - radius, centre_y - radius))
    return layer


def interior_backdrop():
    """Hull of a warship seen from inside, built from the corridor tiles so both match.

    Back wall of plates and vents, pillars of pipes, two pipe runs with elbows and warning lights,
    darker far layers and a darker middle band so ships and shots stay readable. Every pattern repeats
    every 480 pixels, so the image scrolls without a seam.
    """
    width, height, cell = 1920, 1080, 96
    sheet = Image.open(ROOT / "sprites" / "terrain" / "techblocks.png").convert("RGBA")

    def tile(column, row, light, columns=1, rows=1):
        piece = sheet.crop((column * 32, row * 32, (column + columns) * 32, (row + rows) * 32))
        piece = piece.resize((piece.size[0] * 3, piece.size[1] * 3), Image.NEAREST)
        shade = Image.new("RGBA", piece.size, (0, 0, 0, int(255 * (1.0 - light))))
        piece = Image.alpha_composite(piece, shade)
        tint = Image.new("RGBA", piece.size, (30, 10, 60, 40))
        return Image.alpha_composite(piece, tint)

    image = Image.new("RGBA", (width, height), (8, 9, 16, 255))
    for x in range(0, width, cell):
        for y in range(0, height, cell):
            column = x // cell
            row = y // cell
            if column % 5 == 2 and row % 3 == 1:
                piece = tile(4, 2, 0.32)
            elif column % 5 == 4 and row % 4 == 2:
                piece = tile(7, 2, 0.32)
            else:
                piece = tile(3 + (column + row) % 2, 3, 0.26)
            image.alpha_composite(piece, (x, y))
    for x in range(0, width, 480):
        for y in range(0, height, cell):
            image.alpha_composite(tile(2, 0, 0.5, 2, 1), (x + 96, y))
    for top in (150, 834):
        for x in range(0, width, cell * 2):
            image.alpha_composite(tile(0, 0, 0.58, 2, 1), (x, top))
        for x in range(0, width, 480):
            image.alpha_composite(tile(4, 0, 0.62, 2, 2) if top < 500 else tile(4, 4, 0.62, 2, 2), (x + 96, top - 48))
    lights = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(lights)
    for x in range(48, width, 480):
        for y, colour in ((330, (255, 70, 60)), (760, (80, 200, 255))):
            draw.rectangle([x - 6, y - 6, x + 6, y + 6], fill=colour + (255,))
    image = Image.alpha_composite(image, lights.filter(ImageFilter.GaussianBlur(9)))
    image = Image.alpha_composite(image, lights)
    band = Image.new("L", (width, height), 0)
    ImageDraw.Draw(band).rectangle([0, 260, width, 820], fill=150)
    darken = Image.new("RGBA", (width, height), (4, 4, 10, 255))
    darken.putalpha(band.filter(ImageFilter.GaussianBlur(90)))
    image = Image.alpha_composite(image, darken)
    image.convert("RGB").save(ROOT / "backgrounds" / "stage2_interior.png")


BEAM_SIZE = (256, 200)


def beam_frame(seed):
    """A horizontal plasma beam: white core, cyan glow, faint streaks; left and right edges match."""
    width, height = BEAM_SIZE
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    pixels = image.load()
    rng = random.Random(seed)
    streaks = [rng.uniform(-1.0, 1.0) for _ in range(height)]
    centre = height / 2.0
    for y in range(height):
        distance = abs(y - centre) / centre
        glow = max(0.0, 1.0 - distance) ** 0.7
        core = max(0.0, 1.0 - distance * 1.25)
        for x in range(width):
            wave = 0.08 * math.sin(2 * math.pi * (x / width) * 4 + streaks[y] * 3.0)
            level = min(1.0, glow * (0.85 + wave + 0.1 * streaks[y]))
            red = int(80 + 175 * core)
            green = int(200 + 55 * core)
            pixels[x, y] = (red, green, 255, int(255 * min(1.0, level + core)))
    return image


def boss_beam():
    """The beam sheet, and a vertical copy (frames turned a quarter, still side by side)."""
    width, height = BEAM_SIZE
    sheet = Image.new("RGBA", (width * 3, height), (0, 0, 0, 0))
    vertical = Image.new("RGBA", (height * 3, width), (0, 0, 0, 0))
    for index in range(3):
        frame = beam_frame(index + 1)
        sheet.alpha_composite(frame, (index * width, 0))
        vertical.alpha_composite(frame.rotate(90, expand=True), (index * height, 0))
    sheet.save(ROOT / "sprites" / "projectiles" / "boss_beam.png")
    vertical.save(ROOT / "sprites" / "projectiles" / "boss_beam_vertical.png")


WARN_LINE = (1920, 10)
WARN_MARK = (100, 180)


def warnings():
    """Two-frame warnings that blink on the client: frame 0 is lit, frame 1 is faint."""
    line_width, line_height = WARN_LINE
    mark_width, mark_height = WARN_MARK
    sheet = Image.new("RGBA", (line_width * 2, line_height + mark_height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(sheet)
    draw.rectangle([0, 2, line_width - 1, line_height - 3], fill=(255, 50, 50, 230))
    draw.rectangle([0, 4, line_width - 1, line_height - 5], fill=(255, 200, 200, 255))
    draw.rectangle([line_width, 3, 2 * line_width - 1, line_height - 4], fill=(255, 50, 50, 70))
    font = ImageFont.truetype(str(FONT), 170)
    for frame, alpha in ((0, 255), (1, 70)):
        mark = Image.new("RGBA", WARN_MARK, (0, 0, 0, 0))
        ImageDraw.Draw(mark).text((mark_width // 2, mark_height // 2), "!", font=font, anchor="mm", fill=(255, 60, 60, alpha))
        glow = mark.filter(ImageFilter.GaussianBlur(6))
        sheet.alpha_composite(Image.alpha_composite(glow, mark), (frame * mark_width, line_height))
    sheet.save(UI / "warnings.png")
    column = Image.new("RGBA", (line_height * 2, 1080), (0, 0, 0, 0))
    draw = ImageDraw.Draw(column)
    draw.rectangle([2, 0, line_height - 3, 1079], fill=(255, 50, 50, 230))
    draw.rectangle([4, 0, line_height - 5, 1079], fill=(255, 200, 200, 255))
    draw.rectangle([line_height + 3, 0, 2 * line_height - 4, 1079], fill=(255, 50, 50, 70))
    column.save(UI / "warnings_vertical.png")


def chime(samples, start, base, loud):
    """Adds a sparkling arpeggio (three bell tones) starting at `start` seconds."""
    for step, ratio in enumerate((1.0, 1.5, 2.0)):
        offset = int((start + step * 0.06) * SAMPLE_RATE)
        frequency = base * ratio
        for index in range(int(0.9 * SAMPLE_RATE)):
            if offset + index >= len(samples):
                break
            t = index / SAMPLE_RATE
            envelope = math.exp(-5.0 * t) * loud
            tone = math.sin(2 * math.pi * frequency * t) + 0.35 * math.sin(2 * math.pi * frequency * 2.76 * t)
            samples[offset + index] += envelope * tone


def intro_chimes():
    length = INTRO_TITLE + 1.6
    samples = [0.0] * int(length * SAMPLE_RATE)
    notes = (1046.5, 1174.7, 1318.5, 1396.9, 1568.0)
    for start, note in zip(INTRO_CHIMES, notes):
        chime(samples, start, note, 0.22)
    chime(samples, INTRO_TITLE, 523.25, 0.35)
    chime(samples, INTRO_TITLE + 0.12, 784.0, 0.25)
    peak = max(abs(value) for value in samples) or 1.0
    with wave.open(str(ROOT / "sounds" / "INTRO_CHIMES.wav"), "wb") as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(SAMPLE_RATE)
        output.writeframes(b"".join(struct.pack("<h", int(32000 * value / peak * 0.8)) for value in samples))


def main():
    UI.mkdir(parents=True, exist_ok=True)
    frames = buttons()
    layout = hud()
    size = logo()
    menu_backdrop()
    interior_backdrop()
    boss_beam()
    warnings()
    intro_chimes()
    print("logo", size)
    for button_size, row in frames.items():
        print("button", button_size, row)
    for name, box in layout.items():
        print(name, box)


if __name__ == "__main__":
    main()
