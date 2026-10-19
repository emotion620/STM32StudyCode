#include "stm32f10x.h"
#include "OLED.h"
#include "OLED_Font.h"
#include "Delay.h"

/* 软件 I2C：SCL = PB8，SDA = PB9（和江科大 0.96 寸 OLED 模块接法一致） */
#define OLED_W_SCL(x)	GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x)	GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

/* 72MHz 下这一小段空循环大约几百 ns，够 SSD1306 用 */
#define OLED_I2C_DELAY()	do { volatile uint8_t _d = 20; while (_d--); } while (0)

/* 显存：8 页 x 128 列，画完统一刷屏，比逐字节写快很多 */
static uint8_t OLED_Buffer[8][128];

static void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_I2C_DELAY();
	OLED_W_SDA(0);
	OLED_I2C_DELAY();
	OLED_W_SCL(0);
}

static void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_I2C_DELAY();
	OLED_W_SCL(1);
	OLED_I2C_DELAY();
	OLED_W_SDA(1);
}

static void OLED_I2C_SendByte(uint8_t byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(!!(byte & (0x80 >> i)));
		OLED_I2C_DELAY();
		OLED_W_SCL(1);
		OLED_I2C_DELAY();
		OLED_W_SCL(0);
		OLED_I2C_DELAY();
	}
	OLED_W_SCL(1);		/* 第 9 个时钟，不检查应答 */
	OLED_I2C_DELAY();
	OLED_W_SCL(0);
}

static void OLED_WriteCommand(uint8_t cmd)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);
	OLED_I2C_SendByte(0x00);
	OLED_I2C_SendByte(cmd);
	OLED_I2C_Stop();
}

static void OLED_SetCursor(uint8_t page, uint8_t x)
{
	OLED_WriteCommand(0xB0 | page);
	OLED_WriteCommand(0x10 | ((x & 0xF0) >> 4));
	OLED_WriteCommand(0x00 | (x & 0x0F));
}

static void OLED_I2C_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

void OLED_Init(void)
{
	OLED_I2C_Init();

	Delay_ms(100);			/* 等 OLED 上电稳定 */

	OLED_WriteCommand(0xAE);
	OLED_WriteCommand(0xD5);
	OLED_WriteCommand(0x80);
	OLED_WriteCommand(0xA8);
	OLED_WriteCommand(0x3F);
	OLED_WriteCommand(0xD3);
	OLED_WriteCommand(0x00);
	OLED_WriteCommand(0x40);
	OLED_WriteCommand(0xA1);
	OLED_WriteCommand(0xC8);
	OLED_WriteCommand(0xDA);
	OLED_WriteCommand(0x12);
	OLED_WriteCommand(0x81);
	OLED_WriteCommand(0xCF);
	OLED_WriteCommand(0xD9);
	OLED_WriteCommand(0xF1);
	OLED_WriteCommand(0xDB);
	OLED_WriteCommand(0x30);
	OLED_WriteCommand(0xA4);
	OLED_WriteCommand(0xA6);
	OLED_WriteCommand(0x8D);
	OLED_WriteCommand(0x14);
	OLED_WriteCommand(0xAF);

	OLED_Clear();
	OLED_Update();
}

void OLED_Clear(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j++)
		for (i = 0; i < 128; i++)
			OLED_Buffer[j][i] = 0x00;
}

void OLED_Update(void)
{
	uint8_t page, i;

	for (page = 0; page < 8; page++)
	{
		OLED_SetCursor(page, 0);
		OLED_I2C_Start();
		OLED_I2C_SendByte(0x78);
		OLED_I2C_SendByte(0x40);
		for (i = 0; i < 128; i++)
		{
			OLED_I2C_SendByte(OLED_Buffer[page][i]);
		}
		OLED_I2C_Stop();
	}
}

void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color)
{
	if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
	if (color)
		OLED_Buffer[y >> 3][x] |= (uint8_t)(1 << (y & 0x07));
	else
		OLED_Buffer[y >> 3][x] &= (uint8_t)~(1 << (y & 0x07));
}

void OLED_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
	uint8_t i, j;
	for (j = 0; j < h; j++)
		for (i = 0; i < w; i++)
			OLED_DrawPoint(x + i, y + j, color);
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch)
{
	uint8_t col, row, idx;
	uint16_t bits;

	if (ch < 32 || ch > 126) ch = '?';
	idx = (uint8_t)(ch - 32);

	for (col = 0; col < 6; col++)
	{
		bits = OLED_Font6x11[idx][col];
		for (row = 0; row < 11; row++)
		{
			if (bits & (1u << row))
			{
				OLED_DrawPoint(x + col, y + row, 1);
			}
		}
	}
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
	while (*str != '\0' && x + OLED_CHAR_W <= OLED_WIDTH)
	{
		OLED_ShowChar(x, y, *str);
		x += OLED_CHAR_W;
		str++;
	}
}

void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len)
{
	uint8_t i;
	uint32_t div = 1;

	for (i = 1; i < len; i++) div *= 10;
	for (i = 0; i < len; i++)
	{
		OLED_ShowChar(x + i * OLED_CHAR_W, y, (char)('0' + (num / div) % 10));
		div /= 10;
	}
}
