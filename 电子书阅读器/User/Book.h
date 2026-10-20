#ifndef __BOOK_H
#define __BOOK_H

#include <stdint.h>

/* 一屏 5 行，每行 16 个字符（8x12 像素字体），共 80 个字符 */
#define BOOK_COLS			16
#define BOOK_LINES			5
#define BOOK_PAGE_CHARS		(BOOK_COLS * BOOK_LINES)
#define BOOK_SLOT_SIZE		128
#define BOOK_TITLE_MAX		24

void Book_Init(void);

uint8_t  Book_Open(void);					/* 读到有效书头返回 1 */
uint32_t Book_GetPageCount(void);
uint32_t Book_GetMaxPages(void);
const char *Book_GetTitle(void);

void Book_ReadPage(uint32_t page, char *buf);		/* buf 至少 128 字节 */
uint8_t Book_WritePage(uint32_t page, const uint8_t *data, uint16_t len);

void Book_EraseAll(uint32_t pages, void (*onPercent)(uint8_t));
void Book_WriteHeader(uint32_t pages, const char *title);
uint32_t Book_ComputeCrc(uint32_t pages);

void Book_SavePosition(uint32_t page);
uint32_t Book_LoadPosition(void);

#endif
