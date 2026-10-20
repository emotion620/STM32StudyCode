# -*- coding: utf-8 -*-
"""
把书正文编译进固件：生成 User/Book_Builtin.c / .h。
这样只用 ST-Link 烧一次程序就能直接读，不需要 USB-TTL 往 W25Q64 里写。

页格式和 upload_book.py 完全一致（每页 105 个字符，补空格），
固件里 Book_Open() 在 W25Q64 没书时就用这份内置数据。
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import upload_book as ub

SRC = os.path.join(HERE, "gift_of_the_magi.txt")
TITLE = "Gift of the Magi"
USER_DIR = os.path.normpath(os.path.join(HERE, "..", "User"))


def main():
    text = ub.read_text(SRC)
    pages = ub.paginate(text)
    cols, lines = ub.COLS, ub.LINES

    h_path = os.path.join(USER_DIR, "Book_Builtin.h")
    c_path = os.path.join(USER_DIR, "Book_Builtin.c")

    with open(h_path, "w", encoding="utf-8", newline="\r\n") as f:
        f.write(
            "#ifndef __BOOK_BUILTIN_H\n"
            "#define __BOOK_BUILTIN_H\n\n"
            "#include <stdint.h>\n"
            '#include "Book.h"\n\n'
            "/* 编译进固件的正文，由 tools/make_builtin.py 生成，别手改 */\n"
            "extern const uint32_t Book_BuiltinPageCount;\n"
            "extern const char Book_BuiltinTitle[];\n"
            "extern const char Book_BuiltinPages[][BOOK_PAGE_CHARS];\n\n"
            "#endif\n"
        )

    with open(c_path, "w", encoding="utf-8", newline="\r\n") as f:
        f.write('#include "Book_Builtin.h"\n\n')
        f.write("/* 《%s》 %d 页，每页 %d 字符 */\n" % (TITLE, len(pages), cols * lines))
        f.write('const char Book_BuiltinTitle[] = "%s";\n\n' % TITLE)
        f.write("const uint32_t Book_BuiltinPageCount = %d;\n\n" % len(pages))
        f.write("const char Book_BuiltinPages[%d][BOOK_PAGE_CHARS] = {\n" % len(pages))
        for page in pages:
            vals = ",".join(str(ord(c)) for c in page)
            f.write("    {%s},\n" % vals)
        f.write("};\n")

    print("written", h_path)
    print("written", c_path)
    print("pages", len(pages), "bytes", len(pages) * cols * lines)


if __name__ == "__main__":
    main()
