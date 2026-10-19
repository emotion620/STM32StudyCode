# -*- coding: utf-8 -*-
"""挨个打开串口问一句 INFO，看看哪个口上是我们的电子书板子。"""
import sys
import time

sys.path.insert(0, r"E:\STM32_GitCode\电子书阅读器\tools")
import upload_book as ub

for port in ub.list_com_ports():
    try:
        ser = ub.open_port(port, 115200)
    except Exception as exc:
        print(port, "打不开:", exc)
        continue
    try:
        ser.write(b"INFO\n")
        time.sleep(0.4)
        data = b""
        for _ in range(10):
            chunk = ser.read(64)
            if not chunk:
                break
            data += chunk
        print(port, "->", repr(data[:120]))
    finally:
        ser.close()
