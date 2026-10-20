#include "stm32f10x.h"
#include <string.h>
#include "Book.h"
#include "Book_Builtin.h"
#include "W25Q64.h"

/*
 * W25Q64 里的布局
 *   0x000000 ~ 0x000FFF : 书头（magic / 每屏参数 / 页数 / 标题）
 *   0x001000 ~ 0x7FEFFF : 正文，每页固定占 128 字节
 *   0x7FF000 ~ 0x7FFFFF : 阅读进度
 *
 * W25Q64 里没有书时，退回到编译进固件的那份正文（Book_Builtin.c），
 * 这样只用 ST-Link 烧程序也能直接读。
 */
#define BOOK_HEADER_ADDR	0x000000UL
#define BOOK_DATA_ADDR		0x001000UL
#define BOOK_POS_ADDR		0x7FF000UL

#define BOOK_MAX_PAGES		((BOOK_POS_ADDR - BOOK_DATA_ADDR) / BOOK_SLOT_SIZE)

static uint32_t bookPages;
static char bookTitle[BOOK_TITLE_MAX + 1];
static uint8_t bookBuiltin;
static uint8_t flashPresent;

void Book_Init(void)
{
	uint8_t mid = 0, type = 0, capacity = 0;

	W25Q64_Init();
	W25Q64_ReadID(&mid, &type, &capacity);
	/* W25Q64 的 JEDEC ID：EF 40 17。没接 flash 时读回来全是 0xFF */
	flashPresent = (mid == 0xEF && type == 0x40) ? 1 : 0;
}

/* 用固件里内置的那份正文 */
static uint8_t Book_UseBuiltin(void)
{
	if (Book_BuiltinPageCount == 0 || Book_BuiltinPageCount > BOOK_MAX_PAGES)
	{
		return 0;
	}
	bookPages = Book_BuiltinPageCount;
	memset(bookTitle, 0x00, sizeof(bookTitle));
	strncpy(bookTitle, Book_BuiltinTitle, BOOK_TITLE_MAX);
	bookTitle[BOOK_TITLE_MAX] = '\0';
	bookBuiltin = 1;
	return 1;
}

uint8_t Book_Open(void)
{
	uint8_t buf[64];
	uint16_t pageChars, slot;
	uint32_t pages;

	bookPages = 0;
	bookTitle[0] = '\0';
	bookBuiltin = 0;

	if (!flashPresent)
	{
		return Book_UseBuiltin();
	}

	W25Q64_ReadData(BOOK_HEADER_ADDR, buf, sizeof(buf));

	if (buf[0] != 'W' || buf[1] != 'B' || buf[2] != 'K' || buf[3] != '1')
	{
		return Book_UseBuiltin();
	}

	pageChars = (uint16_t)(buf[8] | (buf[9] << 8));
	slot      = (uint16_t)(buf[10] | (buf[11] << 8));
	pages     = (uint32_t)buf[12] | ((uint32_t)buf[13] << 8) |
	            ((uint32_t)buf[14] << 16) | ((uint32_t)buf[15] << 24);

	if (buf[6] != BOOK_COLS || buf[7] != BOOK_LINES) return Book_UseBuiltin();
	if (pageChars != BOOK_PAGE_CHARS || slot != BOOK_SLOT_SIZE) return Book_UseBuiltin();
	if (pages == 0 || pages > BOOK_MAX_PAGES) return Book_UseBuiltin();

	memcpy(bookTitle, &buf[20], BOOK_TITLE_MAX);
	bookTitle[BOOK_TITLE_MAX] = '\0';
	bookPages = pages;
	return 1;
}

uint32_t Book_GetPageCount(void)
{
	return bookPages;
}

uint32_t Book_GetMaxPages(void)
{
	return BOOK_MAX_PAGES;
}

const char *Book_GetTitle(void)
{
	return bookTitle;
}

void Book_ReadPage(uint32_t page, char *buf)
{
	uint8_t raw[BOOK_SLOT_SIZE];

	if (page >= bookPages)
	{
		memset(buf, 0, BOOK_PAGE_CHARS);
		return;
	}

	if (bookBuiltin)
	{
		memcpy(buf, Book_BuiltinPages[page], BOOK_PAGE_CHARS);
		return;
	}
	W25Q64_ReadData(BOOK_DATA_ADDR + page * BOOK_SLOT_SIZE, raw, BOOK_SLOT_SIZE);
	memcpy(buf, raw, BOOK_PAGE_CHARS);
}

uint8_t Book_WritePage(uint32_t page, const uint8_t *data, uint16_t len)
{
	uint8_t raw[BOOK_SLOT_SIZE];

	if (page >= BOOK_MAX_PAGES || len > BOOK_PAGE_CHARS) return 0;

	memset(raw, 0x00, sizeof(raw));
	memcpy(raw, data, len);

	W25Q64_PageProgram(BOOK_DATA_ADDR + page * BOOK_SLOT_SIZE, raw, BOOK_SLOT_SIZE);
	return 1;
}

void Book_EraseAll(uint32_t pages, void (*onPercent)(uint8_t))
{
	uint32_t sectors, i, addr;

	if (pages > BOOK_MAX_PAGES) pages = BOOK_MAX_PAGES;
	sectors = (pages * BOOK_SLOT_SIZE + W25Q64_SECTOR_SIZE - 1) / W25Q64_SECTOR_SIZE;

	W25Q64_SectorErase(BOOK_HEADER_ADDR);		/* 先把书头擦掉，表示"没有书" */

	for (i = 0; i < sectors; i++)
	{
		addr = BOOK_DATA_ADDR + i * W25Q64_SECTOR_SIZE;
		W25Q64_SectorErase(addr);
		if (onPercent != 0 && (i % 4 == 0))
		{
			onPercent((uint8_t)((i * 100) / sectors));
		}
	}
	if (onPercent != 0) onPercent(100);
}

void Book_WriteHeader(uint32_t pages, const char *title)
{
	uint8_t buf[BOOK_SLOT_SIZE];
	uint16_t i;

	memset(buf, 0x00, sizeof(buf));
	buf[0] = 'W'; buf[1] = 'B'; buf[2] = 'K'; buf[3] = '1';
	buf[4] = 1; buf[5] = 0;						/* version 1 */
	buf[6] = BOOK_COLS;
	buf[7] = BOOK_LINES;
	buf[8] = (uint8_t)(BOOK_PAGE_CHARS & 0xFF);
	buf[9] = (uint8_t)(BOOK_PAGE_CHARS >> 8);
	buf[10] = (uint8_t)(BOOK_SLOT_SIZE & 0xFF);
	buf[11] = (uint8_t)(BOOK_SLOT_SIZE >> 8);
	buf[12] = (uint8_t)pages;
	buf[13] = (uint8_t)(pages >> 8);
	buf[14] = (uint8_t)(pages >> 16);
	buf[15] = (uint8_t)(pages >> 24);
	for (i = 0; i < BOOK_TITLE_MAX && title[i] != '\0'; i++)
	{
		buf[20 + i] = (uint8_t)title[i];
	}

	W25Q64_SectorErase(BOOK_HEADER_ADDR);
	W25Q64_PageProgram(BOOK_HEADER_ADDR, buf, BOOK_SLOT_SIZE);
}

static uint32_t Crc32Update(uint32_t crc, uint8_t byte)
{
	uint8_t i;
	crc ^= byte;
	for (i = 0; i < 8; i++)
	{
		if (crc & 1)
			crc = (crc >> 1) ^ 0xEDB88320UL;
		else
			crc >>= 1;
	}
	return crc;
}

uint32_t Book_ComputeCrc(uint32_t pages)
{
	uint8_t raw[BOOK_SLOT_SIZE];
	uint32_t page, i;
	uint32_t crc = 0xFFFFFFFFUL;

	for (page = 0; page < pages; page++)
	{
		W25Q64_ReadData(BOOK_DATA_ADDR + page * BOOK_SLOT_SIZE, raw, BOOK_SLOT_SIZE);
		for (i = 0; i < BOOK_PAGE_CHARS; i++)
		{
			crc = Crc32Update(crc, raw[i]);
		}
	}
	return crc ^ 0xFFFFFFFFUL;
}

void Book_SavePosition(uint32_t page)
{
	uint8_t buf[16];
	uint8_t i;

	if (!flashPresent) return;		/* 没接 W25Q64 就不存进度，避免空等超时 */

	memset(buf, 0x00, sizeof(buf));
	buf[0] = 'P'; buf[1] = 'O'; buf[2] = 'S'; buf[3] = '1';
	buf[4] = (uint8_t)page;
	buf[5] = (uint8_t)(page >> 8);
	buf[6] = (uint8_t)(page >> 16);
	buf[7] = (uint8_t)(page >> 24);
	buf[8] = (uint8_t)(buf[4] ^ buf[5] ^ buf[6] ^ buf[7] ^ 0xA5);
	for (i = 9; i < 16; i++) buf[i] = 0xFF;

	W25Q64_SectorErase(BOOK_POS_ADDR);
	W25Q64_PageProgram(BOOK_POS_ADDR, buf, 16);
}

uint32_t Book_LoadPosition(void)
{
	uint8_t buf[16];
	uint32_t page;

	if (!flashPresent) return 0;

	W25Q64_ReadData(BOOK_POS_ADDR, buf, sizeof(buf));
	if (buf[0] != 'P' || buf[1] != 'O' || buf[2] != 'S' || buf[3] != '1') return 0;

	page = (uint32_t)buf[4] | ((uint32_t)buf[5] << 8) |
	       ((uint32_t)buf[6] << 16) | ((uint32_t)buf[7] << 24);
	if ((uint8_t)(buf[4] ^ buf[5] ^ buf[6] ^ buf[7] ^ 0xA5) != buf[8]) return 0;
	if (page >= bookPages) return 0;
	return page;
}
