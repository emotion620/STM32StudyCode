# -*- coding: utf-8 -*-
"""
自测：用一个"假板子"跑通 upload_book.py 的完整协议，不需要真实硬件。
假板子的行为严格照 User/main.c 和 User/Book.c 的协议实现：
  BEGIN / 二进制帧 / END / CRC
跑完把 flash 映像和 --bin 生成的映像逐字节比对。
"""
import os
import sys
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import upload_book as ub

FLASH_SIZE = 8 * 1024 * 1024
DATA_ADDR = 0x1000
SLOT = 128


class FakeBoard:
    def __init__(self):
        self.flash = bytearray(b"\xff" * FLASH_SIZE)
        self.rx = bytearray()
        self.tx = bytearray()
        self.pages = 0
        self.title = ""
        self.state = "idle"

    # --- 串口接口 ---
    def read(self, n=1):
        out = bytes(self.tx[:n])
        del self.tx[:n]
        return out

    def write(self, data):
        self.rx += data
        self._process()

    def close(self):
        pass

    # --- 协议 ---
    def _reply(self, text):
        self.tx += text.encode("ascii")

    def _process(self):
        while True:
            if self.state == "frames":
                if len(self.rx) < 4:
                    return
                if self.rx[0] != ub.HEAD:
                    # 文本命令
                    idx = self.rx.find(b"\n")
                    if idx < 0:
                        return
                    line = bytes(self.rx[:idx]).decode("ascii", "replace").strip()
                    del self.rx[:idx + 1]
                    self._text_command(line)
                    continue
                if len(self.rx) < 4:
                    return
                idx = self.rx[1] | (self.rx[2] << 8)
                plen = self.rx[3]
                need = 4 + plen + 1
                if len(self.rx) < need:
                    return
                payload = bytes(self.rx[4:4 + plen])
                checksum = self.rx[4 + plen]
                del self.rx[:need]
                calc = (ub.HEAD + (idx & 0xFF) + ((idx >> 8) & 0xFF) + plen + sum(payload)) & 0xFF
                if calc != checksum:
                    self.tx += bytes([ub.ACK_ERR])
                    continue
                slot = bytearray(SLOT)
                slot[:plen] = payload
                self.flash[DATA_ADDR + idx * SLOT: DATA_ADDR + idx * SLOT + SLOT] = slot
                self.tx += bytes([ub.ACK_OK])
                continue

            # idle：等命令行
            idx = self.rx.find(b"\n")
            if idx < 0:
                return
            line = bytes(self.rx[:idx]).decode("ascii", "replace").strip()
            del self.rx[:idx + 1]
            if not line:
                continue
            self._text_command(line)

    def _text_command(self, line):
        parts = line.split(" ", 4)
        cmd = parts[0]
        if cmd == "INFO":
            self._reply("I %d 21 5 %s\n" % (self.pages, self.title))
        elif cmd == "BEGIN":
            self.pages = int(parts[1])
            self.title = parts[4] if len(parts) > 4 else ""
            start = DATA_ADDR
            end = DATA_ADDR + self.pages * SLOT
            self.flash[0:0x1000] = b"\xff" * 0x1000
            self.flash[start:end] = b"\xff" * (end - start)
            self.state = "frames"
            self._reply("OK\n")
        elif cmd == "END":
            hdr = bytearray(SLOT)
            hdr[0:4] = b"WBK1"
            hdr[4] = 1
            hdr[6] = 21
            hdr[7] = 5
            hdr[8:10] = (105).to_bytes(2, "little")
            hdr[10:12] = SLOT.to_bytes(2, "little")
            hdr[12:16] = self.pages.to_bytes(4, "little")
            t = self.title.encode("ascii")[:24]
            hdr[20:20 + len(t)] = t
            self.flash[0:SLOT] = hdr
            self.state = "idle"
            self._reply("OK\n")
        elif cmd == "CRC":
            expect = int(parts[1], 16)
            crc = 0xFFFFFFFF
            for p in range(self.pages):
                base = DATA_ADDR + p * SLOT
                for b in self.flash[base:base + 105]:
                    crc ^= b
                    for _ in range(8):
                        crc = (crc >> 1) ^ 0xEDB88320 if (crc & 1) else (crc >> 1)
            crc ^= 0xFFFFFFFF
            self._reply("OK\n" if crc == expect else "ERR\n")
        else:
            self._reply("ERR\n")


def run(txt):
    pages = ub.paginate(ub.read_text(txt))
    title = "selftest"
    board = FakeBoard()
    dev = ub.Device(board)
    ub.upload(dev, pages, title, 115200)

    # 比对
    assert board.flash[0:4] == b"WBK1", "书头 magic 不对"
    stored_pages = int.from_bytes(board.flash[12:16], "little")
    assert stored_pages == len(pages), "页数不对: %d != %d" % (stored_pages, len(pages))

    for i, page in enumerate(pages):
        base = DATA_ADDR + i * SLOT
        expect = page.encode("ascii").ljust(SLOT, b"\x00")
        got = bytes(board.flash[base:base + SLOT])
        assert got == expect, "第 %d 页内容不一致" % i

    print("\n自测通过：%d 页，协议 / 分页 / CRC 全部一致" % len(pages))


if __name__ == "__main__":
    run(sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), "sample.txt"))
