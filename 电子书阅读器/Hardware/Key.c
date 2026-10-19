#include "stm32f10x.h"
#include "Key.h"

/* 两个按键都接 GND，内部上拉，按下读到 0 */
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Key_GetRaw(void)
{
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)  return KEY_NEXT;
	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0) return KEY_PREV;
	return KEY_NONE;
}
