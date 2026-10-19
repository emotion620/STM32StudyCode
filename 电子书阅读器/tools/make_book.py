# -*- coding: utf-8 -*-
"""
生成可以写进 STM32 电子书的纯文本：
  1. 从 PDF 抽正文段落
  2. 砍掉 Feedbooks 的版权页和结尾的"推荐书"广告
  3. 标点统一成 ASCII（和 OLED 字库一致）
输出 tools/gift_of_the_magi.txt
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import extract_pdf_text as EP
import upload_book as UB

PDF = r"E:\Z-library\O. Henry\The Gift of the Magi (3)\The Gift of the Magi - O. Henry.pdf"
OUT = os.path.join(HERE, "gift_of_the_magi.txt")

TITLE = "The Gift of the Magi"
AUTHOR = "by O. Henry"

paras = EP.extract(PDF)

# 1) 正文从 "ONE dollar and eighty-seven cents" 开始
start = None
for i, p in enumerate(paras):
    m = re.search(r"ONE dollar and eighty-seven cents", p)
    if m:
        start = i
        paras = [p[m.start():]] + paras[i + 1:]
        break
assert start is not None, "没找到正文开头"

# 2) 结尾停在 "They are the magi."
end = None
for i in range(len(paras) - 1, -1, -1):
    if "They are the magi." in paras[i]:
        cut = paras[i].index("They are the magi.") + len("They are the magi.")
        paras = paras[:i] + [paras[i][:cut]]
        end = i
        break
assert end is not None, "没找到正文结尾"

# 3) 标点转 ASCII
paras = [UB.to_ascii(p).strip() for p in paras]
paras = [re.sub(r"\s+", " ", p) for p in paras if p.strip()]
# 破折号转换后可能留下 "men- who" 这种空格，收紧一下
paras = [re.sub(r"([A-Za-z])-\s+([A-Za-z])", r"\1-\2", p) for p in paras]

text = TITLE + "\n\n" + AUTHOR + "\n\n" + "\n\n".join(paras) + "\n"

with open(OUT, "w", encoding="utf-8", newline="\n") as f:
    f.write(text)

chars = len(re.sub(r"\s", "", text))
print("written", OUT)
print("段落", len(paras), "字符(不含空白)", chars)

pages = UB.paginate(text)
print("分页", len(pages), "页，占 flash", len(pages) * 128, "字节")
print("--- 第 0 页 ---")
for i in range(5):
    print("|" + pages[0][i * 21:(i + 1) * 21] + "|")
