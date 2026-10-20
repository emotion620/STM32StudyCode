# -*- coding: utf-8 -*-
"""按固件里真实使用的字体渲染一整页，输出 PNG 预览。"""
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import gen_font_gdi
import upload_book as ub

COLS, LINES = ub.COLS, ub.LINES


def render_page(page_text, page_no, total, out_path, cw=6, ch=12):
    _, _, table = gen_font_gdi.build_table("Terminal", 12, cw - 5)
    img = Image.new("RGB", (128, 64), "black")
    p = img.load()
    for i, chx in enumerate(page_text[:COLS * LINES]):
        row, col = divmod(i, COLS)
        if chx == " ":
            continue
        cols = table.get(ord(chx))
        if not cols:
            continue
        for x in range(min(cw, len(cols))):
            bits = cols[x]
            for y in range(ch):
                if bits >> y & 1:
                    X, Y = col * cw + x, row * 12 + y
                    if X < 128 and Y < 64:
                        p[X, Y] = (255, 255, 255)
    w = max(2, int((page_no + 1) * 124 / max(total, 1)))
    for y in range(60, 64):
        for x in range(2, 2 + w):
            p[x, y] = 255, 255, 255
    img.resize((128 * 6, 64 * 6), Image.NEAREST).save(out_path)


if __name__ == "__main__":
    txt = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "gift_of_the_magi.txt")
    page = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    out = sys.argv[3] if len(sys.argv) > 3 else os.path.join(HERE, "preview_oled.png")
    pages = ub.paginate(ub.read_text(txt))
    render_page(pages[page], page, len(pages), out)
    print("saved", out, "page", page, "/", len(pages))
