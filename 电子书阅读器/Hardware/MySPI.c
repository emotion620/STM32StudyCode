#include "stm32f10x.h"
#include "MySPI.h"

/* 软件 SPI（模式 0）：CS=PA4，SCK=PA5，MISO=PA6，MOSI=PA7 */
#define MySPI_W_CS(x)	GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)(x))
#define MySPI_W_SCK(x)	GPIO_WriteBit(GPIOA, GPIO_Pin_5, (BitAction)(x))
#define MySPI_W_MOSI(x)	GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)(x))
#define MySPI_R_MISO()	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)

/* 空循环一点点，保证 W25Q64 的建立/保持时间；72MHz 下约 0.1us */
#define MySPI_DELAY()	do { volatile uint8_t _d = 4; while (_d--); } while (0)

void MySPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	MySPI_W_CS(1);
	MySPI_W_SCK(0);
}

void MySPI_Start(void)
{
	MySPI_W_CS(0);
}

void MySPI_Stop(void)
{
	MySPI_W_CS(1);
}

uint8_t MySPI_SwapByte(uint8_t byte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		MySPI_W_MOSI(byte & 0x80);
		byte <<= 1;
		MySPI_DELAY();
		MySPI_W_SCK(1);
		MySPI_DELAY();
		if (MySPI_R_MISO() == 1)
		{
			byte |= 0x01;
		}
		MySPI_W_SCK(0);
	}
	return byte;
}
