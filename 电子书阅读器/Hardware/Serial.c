#include "stm32f10x.h"
#include "Serial.h"
#include "Delay.h"

/*
 * USART1：PA9 = TX，PA10 = RX，115200 8N1
 * 接收走中断 + 环形缓冲，主循环慢慢取，不会丢字节。
 */
#define SERIAL_RX_SIZE		512

static volatile uint8_t  Serial_RxBuf[SERIAL_RX_SIZE];
static volatile uint16_t Serial_RxHead;
static volatile uint16_t Serial_RxTail;

void Serial_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART1, ENABLE);

	Serial_RxHead = 0;
	Serial_RxTail = 0;
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		uint8_t data = (uint8_t)USART_ReceiveData(USART1);
		uint16_t next = (Serial_RxHead + 1) % SERIAL_RX_SIZE;
		if (next != Serial_RxTail)		/* 满了就丢，不覆盖 */
		{
			Serial_RxBuf[Serial_RxHead] = data;
			Serial_RxHead = next;
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

void Serial_SendByte(uint8_t byte)
{
	USART_SendData(USART1, byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{
	}
}

void Serial_SendString(const char *str)
{
	while (*str != '\0')
	{
		Serial_SendByte((uint8_t)*str);
		str++;
	}
}

void Serial_SendBuf(const uint8_t *buf, uint16_t len)
{
	uint16_t i;
	for (i = 0; i < len; i++)
	{
		Serial_SendByte(buf[i]);
	}
}

uint16_t Serial_Available(void)
{
	return (uint16_t)((Serial_RxHead + SERIAL_RX_SIZE - Serial_RxTail) % SERIAL_RX_SIZE);
}

int16_t Serial_ReadByte(void)
{
	int16_t data;
	if (Serial_RxHead == Serial_RxTail) return -1;
	data = Serial_RxBuf[Serial_RxTail];
	Serial_RxTail = (Serial_RxTail + 1) % SERIAL_RX_SIZE;
	return data;
}

int16_t Serial_ReadByteTimeout(uint32_t ms)
{
	int16_t data;
	while (1)
	{
		data = Serial_ReadByte();
		if (data >= 0) return data;
		if (ms == 0) return -1;
		Delay_ms(1);
		ms--;
	}
}
