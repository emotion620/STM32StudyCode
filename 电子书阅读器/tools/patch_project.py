# -*- coding: utf-8 -*-
"""把新建的源文件挂进 Keil 工程：加 Hardware/System/User 文件、改 include 路径和输出名。
可以重复执行：已经改过的部分会跳过。
"""
import os

PROJ = r"E:\STM32_GitCode\电子书阅读器\project.uvprojx"

HARDWARE = [
    ("OLED.c", 1), ("OLED.h", 5), ("OLED_Font.h", 5),
    ("MySPI.c", 1), ("MySPI.h", 5),
    ("W25Q64.c", 1), ("W25Q64.h", 5), ("W25Q64_Ins.h", 5),
    ("Key.c", 1), ("Key.h", 5),
    ("Serial.c", 1), ("Serial.h", 5),
]
SYSTEM = [("Delay.c", 1), ("Delay.h", 5)]
USER_EXTRA = [("Book.c", 1), ("Book.h", 5)]


def file_xml(fn, ft, folder):
    return "\n".join([
        "            <File>",
        "              <FileName>%s</FileName>" % fn,
        "              <FileType>%d</FileType>" % ft,
        "              <FilePath>.\\%s\\%s</FilePath>" % (folder, fn),
        "            </File>",
    ])


def group(name, files, folder):
    out = ["        <Group>", "          <GroupName>%s</GroupName>" % name, "          <Files>"]
    for fn, ft in files:
        out.append(file_xml(fn, ft, folder))
    out += ["          </Files>", "        </Group>"]
    return "\n".join(out)


raw = open(PROJ, "rb").read()
try:
    text = raw.decode("utf-8")
    enc = "utf-8"
except UnicodeDecodeError:
    text = raw.decode("gbk")
    enc = "gbk"

changed = []

old_inc = r"<IncludePath>.\User;.\Startup;.\Library</IncludePath>"
new_inc = r"<IncludePath>.\User;.\Startup;.\Library;.\Hardware;.\System</IncludePath>"
if old_inc in text:
    text = text.replace(old_inc, new_inc)
    changed.append("include path")

if "<OutputName>project</OutputName>" in text:
    text = text.replace("<OutputName>project</OutputName>", "<OutputName>ebook</OutputName>")
    changed.append("output name")

if "<TargetName>Target 1</TargetName>" in text:
    text = text.replace("<TargetName>Target 1</TargetName>", "<TargetName>EBook</TargetName>")
    changed.append("target name")

if "<GroupName>Hardware</GroupName>" not in text:
    addition = group("Hardware", HARDWARE, "Hardware") + "\n" + group("System", SYSTEM, "System") + "\n"
    text = text.replace("      </Groups>", addition + "      </Groups>", 1)
    changed.append("Hardware/System groups")

if "<FileName>Book.c</FileName>" not in text:
    anchor = file_xml("stm32f10x_it.h", 5, "User")
    assert anchor in text
    extra = "\n".join(file_xml(fn, ft, "User") for fn, ft in USER_EXTRA)
    text = text.replace(anchor, anchor + "\n" + extra, 1)
    changed.append("Book.c/h")

if "<FileName>Book_Builtin.c</FileName>" not in text:
    anchor = file_xml("Book.h", 5, "User")
    assert anchor in text, "先要有 Book.h 才能插 Book_Builtin"
    extra = "\n".join(file_xml(fn, ft, "User") for fn, ft in [("Book_Builtin.c", 1), ("Book_Builtin.h", 5)])
    text = text.replace(anchor, anchor + "\n" + extra, 1)
    changed.append("Book_Builtin.c/h")

open(PROJ, "wb").write(text.encode(enc))
print("encoding", enc, "changed:", changed or "nothing")

for f in ("project.uvoptx", "project.uvguix.xueluzhou"):
    p = os.path.join(os.path.dirname(PROJ), f)
    if os.path.exists(p):
        os.remove(p)
        print("removed", f)
