#include "stm32f10x.h"
#include "Delay.h"

/*
 * SysTick 做延时：时钟源取 HCLK/8，72MHz 下就是 9MHz，
 * 所以 1us = 9 个计数。这里用查询方式，不开中断。
 */
void Delay_Init(void)
{
	SysTick->CTRL = 0x00000000;
	SysTick->LOAD = 0x00FFFFFF;
	SysTick->VAL  = 0x00000000;
	SysTick->CTRL = 0x00000005;		/* 使能 + HCLK/8 */
}

void Delay_us(uint32_t us)
{
	uint32_t start, ticks;

	if (us == 0) return;
	ticks = us * 9;
	start = SysTick->VAL;
	while (((start - SysTick->VAL) & 0x00FFFFFF) < ticks)
	{
	}
}

void Delay_ms(uint32_t ms)
{
	while (ms--)
	{
		Delay_us(1000);
	}
}
