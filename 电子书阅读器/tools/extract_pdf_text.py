# -*- coding: utf-8 -*-
"""
从 PDF 里抽文字（不依赖任何第三方库）。
这个 PDF 用的是子集字体 + 自定义编码，必须靠 ToUnicode 映射把字形号翻回字符。
只处理这种"文字型、单栏、页内绝对定位"的 PDF，够用就行。
"""
import re
import sys
import zlib

TWO = re.compile(rb"(\d+)\s+\d+\s+obj")


class Pdf:
    def __init__(self, path):
        self.data = open(path, "rb").read()
        self.objs = {}
        for m in TWO.finditer(self.data):
            num = int(m.group(1))
            start = m.end()
            end = self.data.find(b"endobj", start)
            if end < 0:
                continue
            self.objs[num] = self.data[start:end]

    def body(self, n):
        return self.objs.get(n, b"")

    def stream(self, n):
        b = self.body(n)
        i = b.find(b"stream")
        if i < 0:
            return None
        j = i + 6
        if b[j:j + 2] == b"\r\n":
            j += 2
        elif b[j:j + 1] in (b"\n", b"\r"):
            j += 1
        k = b.find(b"endstream", j)
        raw = b[j:k]
        if b"/FlateDecode" in b[:i]:
            try:
                return zlib.decompress(raw)
            except zlib.error:
                try:
                    return zlib.decompressobj().decompress(raw)
                except zlib.error:
                    return None
        return raw

    def pages(self):
        """按 /Pages 的 Kids 顺序返回各页的对象号。"""
        root = None
        for num, b in self.objs.items():
            if re.search(rb"/Type\s*/Pages", b) and b"/Kids" in b:
                root = num
                break

        if root is None:
            return [n for n, b in self.objs.items()
                    if re.search(rb"/Type\s*/Page(?![s])", b)]

        order = []

        def kids_of(b):
            m = re.search(rb"/Kids\s*(\[(.*?)\]|(\d+)\s+\d+\s+R)", b, re.S)
            if not m:
                return []
            if m.group(1).startswith(b"["):
                return [int(x) for x in re.findall(rb"(\d+)\s+\d+\s+R", m.group(2))]
            return [int(m.group(3))]

        def walk(n, depth=0):
            if depth > 8:
                return
            b = self.body(n).lstrip()
            if b.startswith(b"["):          # Kids 指向一个数组对象
                for kid in re.findall(rb"(\d+)\s+\d+\s+R", b):
                    walk(int(kid), depth + 1)
                return
            kids = kids_of(b)
            if kids:
                for kid in kids:
                    walk(kid, depth + 1)
            elif re.search(rb"/Type\s*/Page(?![s])", b):
                order.append(n)

        walk(root)
        return order

    def cmap(self, font_obj):
        b = self.body(font_obj)
        m = re.search(rb"/ToUnicode\s+(\d+)\s+\d+\s+R", b)
        if not m:
            return None
        raw = self.stream(int(m.group(1)))
        if not raw:
            return None
        table = {}
        for block in re.findall(rb"beginbfchar(.*?)endbfchar", raw, re.S):
            for src, dst in re.findall(rb"<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>", block):
                table[int(src, 16)] = hex_to_text(dst)
        for block in re.findall(rb"beginbfrange(.*?)endbfrange", raw, re.S):
            for lo, hi, dst in re.findall(
                    rb"<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>\s*<([0-9A-Fa-f]+)>", block):
                lo, hi = int(lo, 16), int(hi, 16)
                base = int(dst, 16)
                for code in range(lo, hi + 1):
                    table[code] = chr(base + code - lo)
        return table

    def fonts_for_page(self, page_obj):
        b = self.body(page_obj)
        m = re.search(rb"/Font\s*<<(.*?)>>", b, re.S)
        fonts = {}
        if m:
            for name, num in re.findall(rb"/(\w+)\s+(\d+)\s+\d+\s+R", m.group(1)):
                fonts[name.decode()] = int(num)
        if not fonts:
            m = re.search(rb"/Resources\s+(\d+)\s+\d+\s+R", b)
            if m:
                return self.fonts_for_page(int(m.group(1)))
        return fonts

    def content(self, page_obj):
        b = self.body(page_obj)
        m = re.search(rb"/Contents\s+(\d+)\s+\d+\s+R", b)
        if m:
            return self.stream(int(m.group(1))) or b""
        m = re.search(rb"/Contents\s*\[(.*?)\]", b, re.S)
        if m:
            out = b""
            for num in re.findall(rb"(\d+)\s+\d+\s+R", m.group(1)):
                out += (self.stream(int(num)) or b"") + b"\n"
            return out
        return b""


def hex_to_text(hex_bytes):
    raw = bytes.fromhex(hex_bytes.decode())
    if len(raw) % 2:
        return raw.decode("latin-1", "replace")
    try:
        return raw.decode("utf-16-be")
    except UnicodeDecodeError:
        return raw.decode("latin-1", "replace")


TOKEN = re.compile(
    rb"/(\w+)\s+[-\d.]+\s+Tf"                      # 1 字体
    rb"|([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)\s+Tm"  # 2..7
    rb"|<([0-9A-Fa-f]+)>\s*Tj"                     # 8
    rb"|\[(.*?)\]\s*TJ"                            # 9
    rb"|(T\*)"                                     # 10
    rb"|([-\d.]+)\s+([-\d.]+)\s+(Td|TD)"           # 11,12,13 相对移动
)


def decode_run(hexstr, table, fallback):
    if table:
        out = []
        for i in range(0, len(hexstr) - 1, 4):
            code = int(hexstr[i:i + 4], 16)
            out.append(table.get(code, ""))
        return "".join(out)
    return hexstr


def extract(path):
    pdf = Pdf(path)
    pages = pdf.pages()
    all_lines = []                     # (页号, y, x, 文本)

    for pno, page_obj in enumerate(pages):
        fonts = pdf.fonts_for_page(page_obj)
        maps = {name: pdf.cmap(num) for name, num in fonts.items()}
        raw = pdf.content(page_obj)
        if not raw:
            continue

        cur_font = None
        x = y = 0.0
        runs = []                      # (y, x, text)

        for m in TOKEN.finditer(raw):
            if m.group(1):
                cur_font = m.group(1).decode()
            elif m.group(2) is not None:
                x = float(m.group(6))
                y = float(m.group(7))
            elif m.group(11) is not None:
                x += float(m.group(11))
                y += float(m.group(12))
            elif m.group(8):
                text = decode_run(m.group(8).decode(), maps.get(cur_font), cur_font)
                if text:
                    runs.append((y, x, text))
            elif m.group(9) is not None:
                parts = []
                for hx in re.findall(rb"<([0-9A-Fa-f]+)>", m.group(9)):
                    parts.append(decode_run(hx.decode(), maps.get(cur_font), cur_font))
                text = "".join(parts)
                if text:
                    runs.append((y, x, text))

        if not runs:
            continue

        runs.sort(key=lambda r: (round(r[0], 1), r[1]))
        # 同一 y 的碎块拼成一行
        clusters = []
        for ry, rx, text in runs:
            if clusters and abs(ry - clusters[-1][0]) <= 3.0:
                clusters[-1][1].append((rx, text))
            else:
                clusters.append((ry, [(rx, text)]))

        for cy, items in clusters:
            items.sort(key=lambda it: it[0])
            line = "".join(t for _, t in items)
            line = " ".join(line.split())
            if line:
                all_lines.append((pno, cy, items[0][0], line))

    # 行距中位数，用来识别段落之间的额外间隔
    gaps = [all_lines[i][1] - all_lines[i - 1][1]
            for i in range(1, len(all_lines))
            if all_lines[i][0] == all_lines[i - 1][0] and all_lines[i][1] > all_lines[i - 1][1]]
    gaps.sort()
    typical = gaps[len(gaps) // 2] if gaps else 20.0

    page_min_x = {}
    for pno, y, x, text in all_lines:
        page_min_x[pno] = min(page_min_x.get(pno, 1e9), x)

    paras = []
    cur = []
    prev = None
    for pno, y, x, text in all_lines:
        if re.fullmatch(r"\d{1,3}", text):          # 页码
            continue

        new_para = not cur
        if cur and prev is not None:
            ppno, py, px, ptext = prev
            indented = x > page_min_x.get(pno, 0.0) + 8.0
            big_gap = (pno == ppno) and (y - py) > typical * 1.55
            if indented or big_gap:
                new_para = True

        if new_para and cur:
            paras.append(cur)
            cur = []

        if cur and cur[-1].endswith("-") and text[:1].islower():
            cur[-1] = cur[-1] + text            # 行尾本来就是真连字符，直接接上
        else:
            cur.append(text)
        prev = (pno, y, x, text)

    if cur:
        paras.append(cur)

    return [" ".join(" ".join(p).split()) for p in paras]


if __name__ == "__main__":
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    text = "\n\n".join(extract(sys.argv[1]))
    if len(sys.argv) > 2:
        open(sys.argv[2], "w", encoding="utf-8", newline="\n").write(text + "\n")
        print("written", sys.argv[2], len(text), "chars")
    else:
        print(text)
