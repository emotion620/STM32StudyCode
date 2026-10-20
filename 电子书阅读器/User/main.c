#include "stm32f10x.h"
#include <string.h>
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "Serial.h"
#include "W25Q64.h"
#include "Book.h"

/* 串口协议常量，PC 端 tools/upload_book.py 必须保持一致 */
#define UPLOAD_HEAD		0xA5
#define ACK_OK			0x06
#define ACK_ERR			0x15

static uint32_t curPage;
static uint32_t pageCount;
static uint8_t  bookReady;
static uint32_t uploadPages;
static char     uploadTitle[BOOK_TITLE_MAX + 1];

/* ---------------- 小工具 ---------------- */

static void SendU32(uint32_t v)
{
	char buf[11];
	uint8_t i = 0;

	if (v == 0)
	{
		Serial_SendByte('0');
		return;
	}
	while (v > 0 && i < sizeof(buf))
	{
		buf[i++] = (char)('0' + (v % 10));
		v /= 10;
	}
	while (i > 0) Serial_SendByte((uint8_t)buf[--i]);
}

static const char *SkipSpace(const char *p)
{
	while (*p == ' ' || *p == '\t') p++;
	return p;
}

static uint32_t ParseU32(const char **pp)
{
	const char *p = SkipSpace(*pp);
	uint32_t v = 0;

	while (*p >= '0' && *p <= '9')
	{
		v = v * 10 + (uint32_t)(*p - '0');
		p++;
	}
	*pp = p;
	return v;
}

static uint32_t ParseHex(const char **pp)
{
	const char *p = SkipSpace(*pp);
	uint32_t v = 0;
	uint8_t d;

	if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
	while (1)
	{
		if (*p >= '0' && *p <= '9') d = (uint8_t)(*p - '0');
		else if (*p >= 'a' && *p <= 'f') d = (uint8_t)(*p - 'a' + 10);
		else if (*p >= 'A' && *p <= 'F') d = (uint8_t)(*p - 'A' + 10);
		else break;
		v = (v << 4) | d;
		p++;
	}
	*pp = p;
	return v;
}

/* ---------------- 显示 ---------------- */

static void DrawProgressBar(void)
{
	uint32_t w;

	OLED_FillRect(2, 60, 124, 4, 0);
	if (pageCount <= 1)
	{
		OLED_FillRect(2, 60, 124, 4, 1);
		return;
	}
	w = ((curPage + 1) * 124) / pageCount;
	if (w < 2) w = 2;
	OLED_FillRect(2, 60, (uint8_t)w, 4, 1);
}

static void DrawPage(uint32_t page)
{
	char buf[BOOK_SLOT_SIZE];
	uint8_t line, col;
	uint16_t i;

	Book_ReadPage(page, buf);
	OLED_Clear();
	for (line = 0; line < BOOK_LINES; line++)
	{
		for (col = 0; col < BOOK_COLS; col++)
		{
			i = (uint16_t)(line * BOOK_COLS + col);
			if (buf[i] == '\0') break;
			OLED_ShowChar((uint8_t)(col * OLED_CHAR_W), (uint8_t)(line * 12), buf[i]);
		}
	}
	DrawProgressBar();
	OLED_Update();
}

static void ShowNoBook(void)
{
	OLED_Clear();
	OLED_ShowString(0, 0, "NO BOOK IN FLASH");
	OLED_ShowString(0, 20, "SEND VIA UART");
	OLED_ShowString(0, 40, "115200 8N1");
	OLED_Update();
}

static void ShowErasePercent(uint8_t pct)
{
	OLED_Clear();
	OLED_ShowString(0, 0, "ERASING FLASH");
	OLED_ShowNum(0, 20, pct, 3);
	OLED_ShowChar(18, 20, '%');
	OLED_Update();
}

static void ShowReceive(uint32_t got, uint32_t total)
{
	OLED_Clear();
	OLED_ShowString(0, 0, "RECEIVING");
	OLED_ShowNum(0, 20, got, 5);
	OLED_ShowChar(40, 20, '/');
	OLED_ShowNum(48, 20, total, 5);
	OLED_Update();
}

/* ---------------- 翻页 ---------------- */

static void TurnPage(int32_t delta)
{
	if (!bookReady) return;

	if (delta > 0)
	{
		if (curPage + (uint32_t)delta < pageCount) curPage += (uint32_t)delta;
		else curPage = pageCount - 1;
	}
	else
	{
		if (curPage >= (uint32_t)(-delta)) curPage -= (uint32_t)(-delta);
		else curPage = 0;
	}
	DrawPage(curPage);
}

static void KeyScan(uint32_t tick)
{
	static uint8_t  lastRaw;
	static uint8_t  stable;
	static uint8_t  rawCount;
	static uint32_t holdTick;

	uint8_t raw = Key_GetRaw();

	if (raw == lastRaw)
	{
		if (rawCount < 200) rawCount++;
	}
	else
	{
		lastRaw = raw;
		rawCount = 0;
	}

	if (rawCount >= 2 && raw != stable)
	{
		stable = raw;
		if (stable != KEY_NONE)
		{
			TurnPage(stable == KEY_NEXT ? 1 : -1);
			holdTick = tick;
		}
	}

	/* 按住 0.5 秒之后，每 0.1 秒翻一页 */
	if (stable != KEY_NONE && (tick - holdTick) > 50 && ((tick - holdTick) % 10) == 0)
	{
		TurnPage(stable == KEY_NEXT ? 1 : -1);
	}
}

/* ---------------- 上传 ---------------- */

static void Reply(const char *s)
{
	Serial_SendString(s);
}

static void UploadLoop(void)
{
	uint32_t received = 0;
	uint8_t  frame[BOOK_SLOT_SIZE];
	char     line[96];
	uint8_t  len = 0;
	int16_t  c;

	while (1)
	{
		c = Serial_ReadByteTimeout(8000);		/* 8 秒没消息就当掉线 */
		if (c < 0) break;

		if (c == UPLOAD_HEAD)
		{
			uint16_t idx, plen, i;
			uint8_t sum = UPLOAD_HEAD;
			uint8_t ok = 1;

			c = Serial_ReadByteTimeout(1000); if (c < 0) break;
			idx = (uint16_t)c; sum = (uint8_t)(sum + (uint8_t)c);
			c = Serial_ReadByteTimeout(1000); if (c < 0) break;
			idx |= (uint16_t)((uint16_t)c << 8); sum = (uint8_t)(sum + (uint8_t)c);
			c = Serial_ReadByteTimeout(1000); if (c < 0) break;
			plen = (uint16_t)c; sum = (uint8_t)(sum + (uint8_t)c);

			if (plen > BOOK_PAGE_CHARS) ok = 0;
			for (i = 0; i < plen; i++)
			{
				c = Serial_ReadByteTimeout(1000);
				if (c < 0) { ok = 0; break; }
				frame[i] = (uint8_t)c;
				sum = (uint8_t)(sum + (uint8_t)c);
			}
			c = Serial_ReadByteTimeout(1000);
			if (c < 0) ok = 0;
			else if ((uint8_t)c != sum) ok = 0;

			if (ok && Book_WritePage(idx, frame, plen))
			{
				received++;
				Serial_SendByte(ACK_OK);
			}
			else
			{
				Serial_SendByte(ACK_ERR);
			}

			if ((received & 0x1F) == 0) ShowReceive(received, uploadPages);
			continue;
		}

		/* 文本命令 */
		if (c == '\n' || c == '\r')
		{
			const char *p;
			if (len == 0) continue;
			line[len] = '\0';
			len = 0;
			p = line;

			if (strncmp(p, "END", 3) == 0)
			{
				Book_WriteHeader(uploadPages, uploadTitle);
				Reply("OK\n");
				break;
			}
			else if (strncmp(p, "CRC", 3) == 0)
			{
				uint32_t expect, actual;
				p += 3;
				expect = ParseHex(&p);
				actual = Book_ComputeCrc(uploadPages);
				if (actual == expect) Reply("OK\n");
				else Reply("ERR\n");
			}
			else if (strncmp(p, "INFO", 4) == 0)
			{
				Reply("I ");
				SendU32(uploadPages);
				Reply(" ");
				SendU32(BOOK_COLS);
				Reply(" ");
				SendU32(BOOK_LINES);
				Reply(" ");
				Reply(uploadTitle);
				Reply("\n");
			}
			else
			{
				Reply("ERR\n");
			}
		}
		else if (len < sizeof(line) - 1)
		{
			line[len++] = (char)c;
		}
	}

	/* 退出上传模式，重新开书 */
	if (Book_Open())
	{
		bookReady = 1;
		pageCount = Book_GetPageCount();
		curPage = 0;
		DrawPage(curPage);
	}
	else
	{
		bookReady = 0;
		ShowNoBook();
	}
}

static void ExecCommand(char *line)
{
	const char *p = line;

	if (strncmp(p, "BEGIN", 5) == 0)
	{
		uint32_t pages, cols, lines;

		p += 5;
		pages = ParseU32(&p);
		cols  = ParseU32(&p);
		lines = ParseU32(&p);
		p = SkipSpace(p);

		if (cols != BOOK_COLS || lines != BOOK_LINES || pages == 0 ||
		    pages > Book_GetMaxPages())
		{
			Reply("ERR\n");
			return;
		}

		uploadPages = pages;
		memset(uploadTitle, 0x00, sizeof(uploadTitle));
		strncpy(uploadTitle, p, BOOK_TITLE_MAX);
		uploadTitle[BOOK_TITLE_MAX] = '\0';

		Book_EraseAll(pages, ShowErasePercent);
		Reply("OK\n");
		UploadLoop();
	}
	else if (strncmp(p, "INFO", 4) == 0)
	{
		Reply("I ");
		SendU32(bookReady ? pageCount : 0);
		Reply(" ");
		SendU32(BOOK_COLS);
		Reply(" ");
		SendU32(BOOK_LINES);
		Reply(" ");
		Reply(bookReady ? Book_GetTitle() : "");
		Reply("\n");
	}
	else if (strncmp(p, "PAGE", 4) == 0)
	{
		Reply("P ");
		SendU32(curPage);
		Reply(" ");
		SendU32(pageCount);
		Reply("\n");
	}
	else if (strncmp(p, "CRC", 3) == 0)
	{
		/* 写完书之后 PC 会再发一次 CRC 做整本校验 */
		uint32_t expect, actual;

		p += 3;
		expect = ParseHex(&p);
		if (!bookReady) { Reply("ERR\n"); return; }
		actual = Book_ComputeCrc(pageCount);
		Reply(actual == expect ? "OK\n" : "ERR\n");
	}
	else if (strncmp(p, "GOTO", 4) == 0)
	{
		uint32_t n;

		p += 4;
		n = ParseU32(&p);
		if (!bookReady) { Reply("ERR\n"); return; }
		if (n >= pageCount) n = pageCount - 1;
		curPage = n;
		DrawPage(curPage);
		Reply("OK\n");
	}
	else if (strncmp(p, "NEXT", 4) == 0)
	{
		TurnPage(1);
		Reply("OK\n");
	}
	else if (strncmp(p, "PREV", 4) == 0)
	{
		TurnPage(-1);
		Reply("OK\n");
	}
	else
	{
		Reply("ERR\n");
	}
}

static void PollSerial(void)
{
	static char    line[96];
	static uint8_t len;

	int16_t c;

	while ((c = Serial_ReadByte()) >= 0)
	{
		if (c == '\n' || c == '\r')
		{
			if (len > 0)
			{
				line[len] = '\0';
				len = 0;
				ExecCommand(line);
			}
		}
		else if (len < sizeof(line) - 1)
		{
			line[len++] = (char)c;
		}
	}
}

/* ---------------- 主程序 ---------------- */

int main(void)
{
	uint32_t tick = 0;
	uint32_t savedPage = 0xFFFFFFFF;
	uint32_t lastSaveTick = 0;

	Delay_Init();
	OLED_Init();
	Key_Init();
	Serial_Init();
	Book_Init();

	Reply("READY\n");

	if (Book_Open())
	{
		bookReady = 1;
		pageCount = Book_GetPageCount();
		curPage = Book_LoadPosition();
		if (curPage >= pageCount) curPage = 0;
		DrawPage(curPage);
	}
	else
	{
		bookReady = 0;
		ShowNoBook();
	}

	while (1)
	{
		PollSerial();
		KeyScan(tick);

		if (bookReady && curPage != savedPage && (tick - lastSaveTick) > 200)
		{
			Book_SavePosition(curPage);
			savedPage = curPage;
			lastSaveTick = tick;
		}

		Delay_ms(10);
		tick++;
	}
}
