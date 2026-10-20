# -*- coding: utf-8 -*-
"""
upload_book.py —— 把一部 txt 小说分页、打包、通过串口写进 STM32 + W25Q64 的电子书。

用法：
    python upload_book.py book.txt --port COM5
    python upload_book.py book.txt --port COM5 --title "Wonder"
    python upload_book.py book.txt --dry-run          # 只看会分成多少页
    python upload_book.py book.txt --bin book.bin     # 生成镜像文件，不连板子

依赖：只用 Python 标准库（Windows 下用 Win32 API 直接开串口，不需要 pyserial）。
"""

import argparse
import os
import re
import sys
import time
import zlib

COLS = 16
LINES = 5
PAGE_CHARS = COLS * LINES

# 常见“智能标点”换成 ASCII，字库只认 ASCII
PUNCT = {
    "\u2018": "'", "\u2019": "'", "\u201a": "'", "\u201b": "'",
    "\u201c": '"', "\u201d": '"', "\u201e": '"', "\u00ab": '"', "\u00bb": '"',
    "\u2013": "-", "\u2014": "-", "\u2012": "-", "\u2015": "-", "\u2212": "-",
    "\u2026": "...", "\u00b7": ".", "\u2022": "*",
    "\u00a0": " ", "\u2002": " ", "\u2003": " ", "\u2009": " ", "\u200b": "",
    "\u00e9": "e", "\u00e8": "e", "\u00ea": "e", "\u00eb": "e",
    "\u00e0": "a", "\u00e1": "a", "\u00e2": "a", "\u00e4": "a", "\u00e5": "a",
    "\u00e7": "c", "\u00f1": "n", "\u00f6": "o", "\u00fc": "u", "\u00df": "ss",
    "\u00c9": "E", "\u00c0": "A",
}
DROP = set("\u00ad\ufeff\r")


def read_text(path):
    raw = open(path, "rb").read()
    for enc in ("utf-8-sig", "utf-8", "gbk", "cp1252", "latin-1"):
        try:
            return raw.decode(enc)
        except UnicodeDecodeError:
            continue
    return raw.decode("utf-8", "ignore")


def to_ascii(text):
    out = []
    for ch in text:
        if ch in DROP:
            continue
        if ch == "\t":
            out.append(" ")
            continue
        if ch == "\n":
            out.append("\n")
            continue
        if ord(ch) < 0x80:
            out.append(ch)
            continue
        out.append(PUNCT.get(ch, "?"))
    return "".join(out)


def paragraphs(text):
    text = to_ascii(text).replace("\r\n", "\n").replace("\r", "\n")
    blocks = re.split(r"\n\s*\n", text)
    result = []
    for block in blocks:
        words = block.split()
        if words:
            result.append(words)
    return result


def wrap_words(words, width=COLS):
    lines, cur = [], ""
    for word in words:
        while len(word) > width:          # 超长单词硬切
            if cur:
                lines.append(cur)
                cur = ""
            lines.append(word[:width])
            word = word[width:]
        if not cur:
            cur = word
        elif len(cur) + 1 + len(word) <= width:
            cur += " " + word
        else:
            lines.append(cur)
            cur = word
    if cur:
        lines.append(cur)
    return lines


def paginate(text):
    lines = []
    for words in paragraphs(text):
        for line in wrap_words(words):
            lines.append(line)
        if lines and lines[-1] != "":
            lines.append("")             # 段间空一行
    while lines and lines[-1] == "":
        lines.pop()

    pages = []
    for i in range(0, len(lines), LINES):
        chunk = lines[i:i + LINES]
        while len(chunk) < LINES:
            chunk.append("")
        page = "".join(line.ljust(COLS) for line in chunk)
        pages.append(page[:PAGE_CHARS].ljust(PAGE_CHARS))
    return pages


# ---------------------------------------------------------------- 串口

class WinSerial:
    """只依赖 ctypes 的最小串口实现，避免装 pyserial。"""

    def __init__(self, port, baud):
        import ctypes
        import ctypes.wintypes as wt
        self.ctypes = ctypes
        self.wt = wt
        k32 = ctypes.WinDLL("kernel32", use_last_error=True)
        self.k32 = k32

        k32.CreateFileW.restype = wt.HANDLE
        k32.CreateFileW.argtypes = [wt.LPCWSTR, wt.DWORD, wt.DWORD, wt.LPVOID,
                                    wt.DWORD, wt.DWORD, wt.HANDLE]
        k32.ReadFile.argtypes = [wt.HANDLE, wt.LPVOID, wt.DWORD,
                                 ctypes.POINTER(wt.DWORD), wt.LPVOID]
        k32.WriteFile.argtypes = [wt.HANDLE, wt.LPVOID, wt.DWORD,
                                  ctypes.POINTER(wt.DWORD), wt.LPVOID]

        GENERIC_READ, GENERIC_WRITE = 0x80000000, 0x40000000
        OPEN_EXISTING = 3
        self.h = k32.CreateFileW("\\\\.\\" + port, GENERIC_READ | GENERIC_WRITE,
                                 0, None, OPEN_EXISTING, 0, None)
        if self.h == ctypes.c_void_p(-1).value:
            raise OSError("打不开 %s（COM 口被占用，或者板子没插）" % port)

        class DCB(ctypes.Structure):
            _pack_ = 1
            _fields_ = [
                ("DCBlength", wt.DWORD), ("BaudRate", wt.DWORD), ("flags", wt.DWORD),
                ("wReserved", wt.WORD), ("XonLim", wt.WORD), ("XoffLim", wt.WORD),
                ("ByteSize", wt.BYTE), ("Parity", wt.BYTE), ("StopBits", wt.BYTE),
                ("XonChar", ctypes.c_char), ("XoffChar", ctypes.c_char),
                ("ErrorChar", ctypes.c_char), ("EofChar", ctypes.c_char),
                ("EvtChar", ctypes.c_char), ("wReserved1", wt.WORD),
            ]

        class COMMTIMEOUTS(ctypes.Structure):
            _fields_ = [("ReadIntervalTimeout", wt.DWORD),
                        ("ReadTotalTimeoutMultiplier", wt.DWORD),
                        ("ReadTotalTimeoutConstant", wt.DWORD),
                        ("WriteTotalTimeoutMultiplier", wt.DWORD),
                        ("WriteTotalTimeoutConstant", wt.DWORD)]

        dcb = DCB()
        dcb.DCBlength = ctypes.sizeof(DCB)
        k32.BuildCommDCBW(ctypes.c_wchar_p("baud=%d parity=N data=8 stop=1" % baud),
                          ctypes.byref(dcb))
        if not k32.SetCommState(self.h, ctypes.byref(dcb)):
            raise OSError("串口参数设置失败")

        to = COMMTIMEOUTS()
        to.ReadIntervalTimeout = 0xFFFFFFFF
        to.ReadTotalTimeoutMultiplier = 0
        to.ReadTotalTimeoutConstant = 100
        to.WriteTotalTimeoutMultiplier = 0
        to.WriteTotalTimeoutConstant = 2000
        k32.SetCommTimeouts(self.h, ctypes.byref(to))
        k32.PurgeComm(self.h, 0x0004 | 0x0008)     # 清掉收发缓冲

    def read(self, n=1):
        buf = self.ctypes.create_string_buffer(n)
        got = self.wt.DWORD(0)
        ok = self.k32.ReadFile(self.h, buf, n, self.ctypes.byref(got), None)
        if not ok or got.value == 0:
            return b""
        return buf.raw[:got.value]

    def write(self, data):
        off = 0
        while off < len(data):
            chunk = data[off:]
            buf = self.ctypes.create_string_buffer(chunk)
            wrote = self.wt.DWORD(0)
            ok = self.k32.WriteFile(self.h, buf, len(chunk),
                                    self.ctypes.byref(wrote), None)
            if not ok or wrote.value == 0:
                raise OSError("串口写失败")
            off += wrote.value

    def close(self):
        if self.h:
            self.k32.CloseHandle(self.h)
            self.h = None


class PySerialWrapper:
    """有 pyserial 就用它，接口和 WinSerial 一样。"""

    def __init__(self, port, baud):
        import serial
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.ser.reset_input_buffer()

    def read(self, n=1):
        return self.ser.read(n)

    def write(self, data):
        self.ser.write(data)

    def close(self):
        self.ser.close()


def open_port(port, baud):
    try:
        import serial  # noqa: F401
        return PySerialWrapper(port, baud)
    except Exception:
        return WinSerial(port, baud)


def list_com_ports():
    try:
        import winreg
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"HARDWARE\DEVICEMAP\SERIALCOMM")
        ports = []
        i = 0
        while True:
            try:
                name, value, _ = winreg.EnumValue(key, i)
            except OSError:
                break
            ports.append(value)
            i += 1
        return ports
    except Exception:
        return []


# ---------------------------------------------------------------- 协议

HEAD = 0xA5
ACK_OK = 0x06
ACK_ERR = 0x15


class Device:
    def __init__(self, ser):
        self.ser = ser
        self.buf = b""

    def read_exact(self, n, timeout):
        deadline = time.time() + timeout
        out = b""
        while len(out) < n:
            chunk = self.ser.read(n - len(out))
            if chunk:
                out += chunk
            elif time.time() > deadline:
                raise TimeoutError("等数据超时")
            else:
                time.sleep(0.002)
        return out

    def read_line(self, timeout=10.0):
        deadline = time.time() + timeout
        line = b""
        while True:
            chunk = self.ser.read(1)
            if chunk:
                if chunk == b"\n":
                    return line.decode("ascii", "replace").strip()
                if chunk != b"\r":
                    line += chunk
                if len(line) > 200:
                    return line.decode("ascii", "replace")
            elif time.time() > deadline:
                raise TimeoutError("等应答超时（%s）" % line)
            else:
                time.sleep(0.002)

    def send_line(self, text):
        self.ser.write(text.encode("ascii", "replace") + b"\n")

    def drain(self, seconds=0.3):
        end = time.time() + seconds
        while time.time() < end:
            if not self.ser.read(256):
                time.sleep(0.01)


def expect_ok(dev, timeout, what):
    line = dev.read_line(timeout)
    if line != "OK":
        raise RuntimeError("%s 失败，板子回的是：%r" % (what, line))


def upload(dev, pages, title, baud):
    total = len(pages)
    crc = zlib.crc32("".join(pages).encode("ascii")) & 0xFFFFFFFF

    print("开始擦除并写入：%d 页，%.1f KB" % (total, total * PAGE_CHARS / 1024.0))
    dev.send_line("BEGIN %d %d %d %s" % (total, COLS, LINES, title))
    expect_ok(dev, 120, "开始上传（擦除 flash）")

    t0 = time.time()
    for idx, page in enumerate(pages):
        payload = page.encode("ascii")
        for attempt in range(4):
            frame = bytes([HEAD, idx & 0xFF, (idx >> 8) & 0xFF, len(payload)]) + payload
            frame += bytes([sum(frame) & 0xFF])
            dev.ser.write(frame)
            ack = dev.read_exact(1, 3)
            if ack == bytes([ACK_OK]):
                break
            if attempt == 3:
                raise RuntimeError("第 %d 页写入失败" % idx)
        if idx % 50 == 0 or idx == total - 1:
            done = idx + 1
            speed = done / max(time.time() - t0, 1e-6)
            eta = (total - done) / max(speed, 1e-6)
            print("\r  写入 %5d/%d  %5.1f%%  %5.1f 页/秒  剩余 %3ds"
                  % (done, total, done * 100.0 / total, speed, eta), end="")
    print()

    dev.send_line("END")
    expect_ok(dev, 20, "写书头")

    print("校验 CRC32 = %08X ..." % crc, end="")
    dev.send_line("CRC %08X" % crc)
    expect_ok(dev, 120, "CRC 校验")
    print(" 通过")
    return crc


def main():
    ap = argparse.ArgumentParser(description="把 txt 小说写进 STM32 电子书")
    ap.add_argument("txt", help="小说文本文件（UTF-8 或 GBK 都行）")
    ap.add_argument("--port", help="串口，例如 COM5")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--title", help="书名，默认用文件名（最多 24 个 ASCII 字符）")
    ap.add_argument("--bin", help="只生成二进制镜像，不连串口")
    ap.add_argument("--dry-run", action="store_true", help="只分页，不写设备")
    ap.add_argument("--show", type=int, default=0, help="打印第 N 页的样子")
    args = ap.parse_args()

    pages = paginate(read_text(args.txt))
    title = (args.title or os.path.splitext(os.path.basename(args.txt))[0])[:24]
    title = "".join(c if 32 <= ord(c) < 127 else "?" for c in title)

    print("书名: %s" % title)
    print("总页数: %d（每页 %d 行 x %d 字符 = %d 字符，屏幕 5 行）"
          % (len(pages), LINES, COLS, PAGE_CHARS))
    print("每页占 128 字节，共占 flash %.1f KB" % (len(pages) * 128 / 1024.0))

    if args.show:
        n = args.show
        if 0 <= n < len(pages):
            print("\n--- 第 %d 页 ---" % n)
            for i in range(LINES):
                print("|" + pages[n][i * COLS:(i + 1) * COLS] + "|")
            print("---")

    if args.dry_run:
        return

    if args.bin:
        with open(args.bin, "wb") as f:
            for page in pages:
                f.write(page.encode("ascii").ljust(128, b"\x00"))
        print("已生成 %s（%d 字节）" % (args.bin, len(pages) * 128))
        return

    port = args.port
    if not port:
        ports = list_com_ports()
        if len(ports) == 1:
            port = ports[0]
            print("自动选串口:", port)
        else:
            print("请用 --port 指定串口，当前系统里的串口:", ports or "没找到")
            sys.exit(1)

    print("连接 %s @ %d ..." % (port, args.baud))
    dev = Device(open_port(port, args.baud))
    try:
        dev.drain(0.5)
        dev.send_line("INFO")
        print("板子:", dev.read_line(5))
        upload(dev, pages, title, args.baud)
        print("完成！按板子上的按键就能开始读了。")
    finally:
        dev.ser.close()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n已取消")
    except Exception as exc:
        print("\n出错：%s" % exc)
        sys.exit(1)
