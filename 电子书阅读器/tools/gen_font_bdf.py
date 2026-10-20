# -*- coding: utf-8 -*-
"""
把 BDF 位图字体（TakWolf/retro-pixel-font 之类的像素字体）转成 OLED 用的 C 字模。

用法：
    python gen_font_bdf.py <font.bdf> [输出列数] [行数]
例：
    python gen_font_bdf.py arcade.bdf 16 5
"""
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
FIRST, LAST = 0x20, 0x7E


def parse_bdf(path):
    ascent = descent = None
    chars = {}
    with open(path, "r", encoding="latin-1") as f:
        lines = [ln.strip() for ln in f]

    i = 0
    while i < len(lines):
        ln = lines[i]
        if ln.startswith("FONT_ASCENT"):
            ascent = int(ln.split()[1])
        elif ln.startswith("FONT_DESCENT"):
            descent = int(ln.split()[1])
        elif ln.startswith("STARTCHAR"):
            code = None
            dw = 0
            bbx = (0, 0, 0, 0)
            bitmap = []
            i += 1
            while i < len(lines) and not lines[i].startswith("ENDCHAR"):
                cur = lines[i]
                if cur.startswith("ENCODING"):
                    code = int(cur.split()[1])
                elif cur.startswith("DWIDTH"):
                    dw = int(cur.split()[1])
                elif cur.startswith("BBX"):
                    p = cur.split()
                    bbx = (int(p[1]), int(p[2]), int(p[3]), int(p[4]))
                elif cur.startswith("BITMAP"):
                    i += 1
                    while i < len(lines) and not lines[i].startswith("ENDCHAR"):
                        if lines[i]:
                            bitmap.append(lines[i])
                        i += 1
                    break
                i += 1
            if code is not None:
                chars[code] = (dw, bbx, bitmap)
        i += 1
    return ascent, descent, chars


def build(bdf_path):
    ascent, descent, chars = parse_bdf(bdf_path)
    height = ascent + descent
    width = max(v[0] for v in chars.values())
    table = {}
    for code in range(FIRST, LAST + 1):
        cols = [0] * width
        if code in chars:
            dw, (bw, bh, bx, by), bitmap = chars[code]
            for j, row_hex in enumerate(bitmap):
                val = int(row_hex, 16) if row_hex else 0
                bits = bin(val)[2:].zfill(len(row_hex) * 4)
                y = ascent - (by + bh) + j
                for x in range(min(bw, len(bits))):
                    if bits[x] == "1" and 0 <= y < height and 0 <= bx + x < width:
                        cols[bx + x] |= 1 << y
        table[code] = cols
    return width, height, table


def render_preview(table, w, h, cols, out, text):
    img = Image.new("RGB", (128, 64), "black")
    p = img.load()
    col = row = 0
    for ch in text:
        if col >= cols:
            col, row = 0, row + 1
        if (row + 1) * h > 64:
            break
        if ch == " ":
            col += 1
            continue
        for x in range(w):
            bits = table.get(ord(ch), [0] * w)[x]
            for y in range(h):
                if bits >> y & 1:
                    X, Y = col * w + x, row * h + y
                    if X < 128 and Y < 64:
                        p[X, Y] = (255, 255, 255)
        col += 1
    img.resize((128 * 6, 64 * 6), Image.NEAREST).save(out)


def main():
    bdf = sys.argv[1]
    cols = int(sys.argv[2]) if len(sys.argv) > 2 else 16
    rows = int(sys.argv[3]) if len(sys.argv) > 3 else 5

    w, h, table = build(bdf)
    print("cell %dx%d, %d cols x %d rows" % (w, h, cols, rows))

    lines = [
        "#ifndef __OLED_FONT_H",
        "#define __OLED_FONT_H",
        "",
        "/* %s —— %dx%d 像素字模，字符 0x20~0x7E */" % (os.path.basename(bdf), w, h),
        "/* 每字符 %d 列，每列一个 uint16_t，bit0 在最上面一行 */" % w,
        "/* 由 tools/gen_font_bdf.py 生成；字体授权见 tools/fonts/OFL.txt */",
        "",
        "#define OLED_FONT_W %d" % w,
        "#define OLED_FONT_H %d" % h,
        "",
        "static const unsigned short OLED_Font[95][OLED_FONT_W] = {",
    ]
    for code in range(FIRST, LAST + 1):
        vals = ",".join("0x%04X" % v for v in table[code])
        disp = {"\\": "\\\\", "'": "\\'", '"': '\\"'}.get(chr(code), chr(code))
        lines.append("    {%s},  /* '%s' */" % (vals, disp))
    lines += ["};", "", "#endif", ""]

    out = os.path.join(HERE, "..", "Hardware", "OLED_Font.h")
    with open(out, "w", encoding="utf-8", newline="\r\n") as f:
        f.write("\n".join(lines))
    print("written", os.path.normpath(out))

    preview = os.path.join(HERE, "preview_font.png")
    render_preview(table, w, h, cols,
                   preview,
                   "The Gift of the Magi. One dollar and eighty-seven cents. "
                   "That was all. And sixty cents of it was in pennies.")


if __name__ == "__main__":
    main()
