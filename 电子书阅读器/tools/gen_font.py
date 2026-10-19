# -*- coding: utf-8 -*-
"""生成 OLED 用的 6x11 ASCII 点阵表（真正的 1bit 位图字体，来自 Pillow 内置字模）。"""
import sys
from PIL import ImageFont

FONT = ImageFont.load_default_imagefont()
FIRST, LAST = 0x20, 0x7E
CW, CH = FONT.getmask("M").size


def glyph(ch):
    m = FONT.getmask(ch)
    w, h = m.size
    raw = bytes(m)
    one_per_px = len(raw) >= w * h
    grid = [[0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            if one_per_px:
                grid[y][x] = 1 if raw[y * w + x] else 0
            else:
                stride = len(raw) // h
                grid[y][x] = (raw[y * stride + (x >> 3)] >> (7 - (x & 7))) & 1
    cols = []
    for x in range(CW):
        bits = 0
        for y in range(CH):
            if x < w and y < h and grid[y][x]:
                bits |= 1 << y
        cols.append(bits)
    return cols, grid


if len(sys.argv) > 1 and sys.argv[1] == "art":
    for ch in "Il1gjqAWy":
        cols, grid = glyph(ch)
        print(f"-- {ch!r}")
        for row in grid:
            print("   " + "".join("#" if b else "." for b in row))
    sys.exit(0)

out = [
    "#ifndef __OLED_FONT_H",
    "#define __OLED_FONT_H",
    "",
    "/* 6x11 ASCII 点阵字库（1bit 位图字模），字符范围 0x20~0x7E */",
    "/* 每个字符 6 列，每列一个 uint16_t，bit0 在最上面一行 */",
    "/* 由 tools/gen_font.py 生成，不要在 Keil 里手工改 */",
    "",
    "static const unsigned short OLED_Font6x11[95][6] = {",
]
for code in range(FIRST, LAST + 1):
    cols, _ = glyph(chr(code))
    disp = {"\\": "\\\\", "'": "\\'", '"': '\\"'}.get(chr(code), chr(code))
    body = ",".join("0x%04X" % c for c in cols)
    out.append("    {%s},  /* '%s' */" % (body, disp))
out += ["};", "", "#endif", ""]

path = r"E:\STM32_GitCode\电子书阅读器\Hardware\OLED_Font.h"
with open(path, "w", encoding="utf-8", newline="\r\n") as f:
    f.write("\n".join(out))
print("written", path, "cell", CW, "x", CH)
