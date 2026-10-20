# -*- coding: utf-8 -*-
"""按固件里真实使用的字体渲染一整页，输出 PNG 预览。"""
import glob
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import gen_font_bdf
import upload_book as ub

COLS, LINES = ub.COLS, ub.LINES


def load_font():
    bdfs = glob.glob(os.path.join(HERE, "fonts", "*.bdf"))
    if not bdfs:
        raise SystemExit("tools/fonts 里没有 .bdf 字体")
    return gen_font_bdf.build(bdfs[0])


def render_page(page_text, page_no, total, out_path, w, h, table):
    img = Image.new("RGB", (128, 64), "black")
    p = img.load()
    for i, chx in enumerate(page_text[:COLS * LINES]):
        row, col = divmod(i, COLS)
        if chx == " ":
            continue
        cols = table.get(ord(chx))
        if not cols:
            continue
        for x in range(w):
            bits = cols[x]
            for y in range(h):
                if bits >> y & 1:
                    X, Y = col * w + x, row * h + y
                    if X < 128 and Y < 64:
                        p[X, Y] = (255, 255, 255)
    wpx = max(2, int((page_no + 1) * 124 / max(total, 1)))
    for y in range(60, 64):
        for x in range(2, 2 + wpx):
            p[x, y] = 255, 255, 255
    img.resize((128 * 6, 64 * 6), Image.NEAREST).save(out_path)


if __name__ == "__main__":
    txt = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "gift_of_the_magi.txt")
    page = int(sys.argv[2]) if len(sys.argv) > 2 else 6
    out = sys.argv[3] if len(sys.argv) > 3 else os.path.join(HERE, "preview_oled.png")
    w, h, table = load_font()
    pages = ub.paginate(ub.read_text(txt))
    render_page(pages[page], page, len(pages), out, w, h, table)
    print("saved", out, "page", page, "/", len(pages), "cell", w, "x", h)
