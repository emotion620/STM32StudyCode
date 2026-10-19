# -*- coding: utf-8 -*-
"""把某一页渲染成 128x64 的 PNG，用来预览 OLED 上实际会是什么样。"""
import argparse
import sys
from PIL import Image, ImageFont

FS = ImageFont.load_default_imagefont()
CW, CH = FS.getmask("M").size


def glyph(ch):
    m = FS.getmask(ch)
    w, h = m.size
    raw = bytes(m)
    one = len(raw) >= w * h
    g = [[0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            if one:
                g[y][x] = 1 if raw[y * w + x] else 0
            else:
                stride = len(raw) // h
                g[y][x] = (raw[y * stride + (x >> 3)] >> (7 - (x & 7))) & 1
    return g


def render(page_text, page, total, path):
    img = Image.new("RGB", (128, 64), "black")
    p = img.load()
    for i, ch in enumerate(page_text[:105]):
        line, col = divmod(i, 21)
        if ch == " ":
            continue
        g = glyph(ch)
        for y in range(min(11, len(g))):
            for x in range(min(CW, len(g[y]))):
                if g[y][x]:
                    X, Y = col * 6 + x, line * 12 + y
                    if X < 128 and Y < 64:
                        p[X, Y] = (255, 255, 255)
    # 进度条
    w = max(2, int((page + 1) * 124 / max(total, 1)))
    for y in range(60, 64):
        for x in range(2, 2 + w):
            p[x, y] = (255, 255, 255)
    img.resize((128 * 6, 64 * 6), Image.NEAREST).save(path)
    print("saved", path)


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("txt")
    ap.add_argument("--page", type=int, default=0)
    ap.add_argument("--out", default="preview_page.png")
    a = ap.parse_args()

    sys.path.insert(0, ".")
    import upload_book as ub

    pages = ub.paginate(ub.read_text(a.txt))
    render(pages[a.page], a.page, len(pages), a.out)
    print("page %d / %d" % (a.page, len(pages)))
