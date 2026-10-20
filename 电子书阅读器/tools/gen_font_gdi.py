# -*- coding: utf-8 -*-
"""
用 Windows 自带的位图字体（Small Fonts / Terminal）生成 OLED 点阵。
比 TTF 缩放清晰得多：GDI 直接给的是 1bit 字形。

用法：python gen_font_gdi.py "Terminal" 12
输出 Hardware/OLED_Font.h 和 tools/preview_font.png
"""
import ctypes
import ctypes.wintypes as wt
import sys
from PIL import Image

gdi32 = ctypes.WinDLL("gdi32", use_last_error=True)
user32 = ctypes.WinDLL("user32", use_last_error=True)


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [("biSize", wt.DWORD), ("biWidth", wt.LONG), ("biHeight", wt.LONG),
                ("biPlanes", wt.WORD), ("biBitCount", wt.WORD),
                ("biCompression", wt.DWORD), ("biSizeImage", wt.DWORD),
                ("biXPelsPerMeter", wt.LONG), ("biYPelsPerMeter", wt.LONG),
                ("biClrUsed", wt.DWORD), ("biClrImportant", wt.DWORD)]


class SIZE(ctypes.Structure):
    _fields_ = [("cx", wt.LONG), ("cy", wt.LONG)]


gdi32.CreateCompatibleDC.restype = wt.HDC
gdi32.CreateCompatibleDC.argtypes = [wt.HDC]
gdi32.CreateFontW.restype = wt.HFONT
gdi32.CreateFontW.argtypes = [ctypes.c_int] * 5 + [wt.DWORD] * 8 + [wt.LPCWSTR]
gdi32.SelectObject.restype = wt.HGDIOBJ
gdi32.SelectObject.argtypes = [wt.HDC, wt.HGDIOBJ]
gdi32.SetTextColor.argtypes = [wt.HDC, wt.COLORREF]
gdi32.SetBkColor.argtypes = [wt.HDC, wt.COLORREF]
gdi32.CreateDIBSection.restype = wt.HBITMAP
gdi32.CreateDIBSection.argtypes = [wt.HDC, ctypes.c_void_p, ctypes.c_uint,
                                   ctypes.POINTER(ctypes.c_void_p), wt.HANDLE, wt.DWORD]
gdi32.GetTextExtentPoint32W.argtypes = [wt.HDC, wt.LPCWSTR, ctypes.c_int,
                                        ctypes.POINTER(SIZE)]
gdi32.TextOutW.argtypes = [wt.HDC, ctypes.c_int, ctypes.c_int, wt.LPCWSTR, ctypes.c_int]
gdi32.DeleteObject.argtypes = [wt.HGDIOBJ]
gdi32.DeleteDC.argtypes = [wt.HDC]

FIRST, LAST = 0x20, 0x7E


def render(family, height):
    screen = user32.GetDC(None)
    mem = gdi32.CreateCompatibleDC(screen)
    font = gdi32.CreateFontW(-height, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 4, 0x30, family)
    old = gdi32.SelectObject(mem, font)
    gdi32.SetTextColor(mem, 0xFFFFFF)
    gdi32.SetBkColor(mem, 0x000000)

    glyphs = {}
    for code in range(FIRST, LAST + 1):
        ch = chr(code)
        size = SIZE()
        gdi32.GetTextExtentPoint32W(mem, ch, 1, ctypes.byref(size))
        w, h = max(size.cx, 1), max(size.cy, 1)
        bi = BITMAPINFOHEADER()
        bi.biSize = ctypes.sizeof(BITMAPINFOHEADER)
        bi.biWidth = w
        bi.biHeight = -h
        bi.biPlanes = 1
        bi.biBitCount = 32
        bits = ctypes.c_void_p()
        dib = gdi32.CreateDIBSection(mem, ctypes.byref(bi), 0, ctypes.byref(bits), None, 0)
        oldb = gdi32.SelectObject(mem, dib)
        buf = (ctypes.c_ubyte * (w * h * 4)).from_address(bits.value)
        for i in range(w * h * 4):
            buf[i] = 0
        if ch != " ":
            gdi32.TextOutW(mem, 0, 0, ch, 1)
        grid = [[1 if buf[(y * w + x) * 4] > 100 else 0 for x in range(w)] for y in range(h)]
        glyphs[ch] = (w, h, grid)
        gdi32.SelectObject(mem, oldb)
        gdi32.DeleteObject(dib)

    gdi32.SelectObject(mem, old)
    gdi32.DeleteObject(font)
    gdi32.DeleteDC(mem)
    user32.ReleaseDC(None, screen)
    return glyphs


def build_table(family="Terminal", height=12, gap=1):
    """返回 (cell_w, cell_h, {code: [每列的 uint16 位图]})，供固件预览复用。"""
    glyphs = render(family, height)
    cw = max(g[0] for g in glyphs.values()) + gap
    ch = max(g[1] for g in glyphs.values())
    table = {}
    for code in range(FIRST, LAST + 1):
        w, h, grid = glyphs[chr(code)]
        cols = []
        for x in range(cw):
            bits = 0
            for y in range(ch):
                if x < w and y < h and grid[y][x]:
                    bits |= 1 << y
            cols.append(bits)
        table[code] = cols
    return cw, ch, table


def main():
    family = sys.argv[1] if len(sys.argv) > 1 else "Terminal"
    height = int(sys.argv[2]) if len(sys.argv) > 2 else 12
    gap = int(sys.argv[3]) if len(sys.argv) > 3 else 1

    cw, ch, cols_table = build_table(family, height, gap)
    print("family=%s height=%d -> cell %dx%d" % (family, height, cw, ch))

    # 预览：用生成的表画一段英文
    img = Image.new("RGB", (128, 64), "black")
    p = img.load()
    text = ("The Gift of the Magi. One dollar and eighty-seven cents. "
            "That was all. And sixty cents of it was in pennies, saved one "
            "and two at a time.")
    col = row = 0
    per_line = 128 // cw
    for chx in text:
        if col >= per_line:
            col, row = 0, row + 1
        if (row + 1) * ch > 60:
            break
        if chx == " ":
            col += 1
            continue
        for x in range(cw):
            for y in range(ch):
                if cols_table[ord(chx)][x] >> y & 1:
                    if col * cw + x < 128 and row * ch + y < 64:
                        p[col * cw + x, row * ch + y] = (255, 255, 255)
        col += 1
    img.resize((128 * 6, 64 * 6), Image.NEAREST).save(
        r"E:\STM32_GitCode\电子书阅读器\tools\preview_font.png")

    lines = [
        "#ifndef __OLED_FONT_H",
        "#define __OLED_FONT_H",
        "",
        "/* %s %dpx 位图字模，cell %dx%d，字符 0x20~0x7E */" % (family, height, cw, ch),
        "/* 每字符 %d 列，每列一个 uint16_t，bit0 在最上面一行 */" % cw,
        "/* 由 tools/gen_font_gdi.py 生成，别手改 */",
        "",
        "#define OLED_FONT_W %d" % cw,
        "#define OLED_FONT_H %d" % ch,
        "",
        "static const unsigned short OLED_Font[95][OLED_FONT_W] = {",
    ]
    for code in range(FIRST, LAST + 1):
        vals = ",".join("0x%04X" % v for v in cols_table[code])
        disp = {"\\": "\\\\", "'": "\\'", '"': '\\"'}.get(chr(code), chr(code))
        lines.append("    {%s},  /* '%s' */" % (vals, disp))
    lines += ["};", "", "#endif", ""]

    path = r"E:\STM32_GitCode\电子书阅读器\Hardware\OLED_Font.h"
    with open(path, "w", encoding="utf-8", newline="\r\n") as f:
        f.write("\n".join(lines))
    print("written", path)


if __name__ == "__main__":
    main()
