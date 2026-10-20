#ifndef __BOOK_BUILTIN_H
#define __BOOK_BUILTIN_H

#include <stdint.h>
#include "Book.h"

/* 编译进固件的正文，由 tools/make_builtin.py 生成，别手改 */
extern const uint32_t Book_BuiltinPageCount;
extern const char Book_BuiltinTitle[];
extern const char Book_BuiltinPages[][BOOK_PAGE_CHARS];

#endif
