#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);
	GPIO_WriteBit(GPIOA,GPIO_Pin_1,Bit_SET);
}

uint8_t Get_KeyNum(void)
{
	uint8_t KeyNum=0;
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)==RESET)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)==RESET);
		Delay_ms(20);
		KeyNum=2;
	}
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==RESET)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==RESET);
		Delay_ms(20);
		KeyNum=1;
	}
	return KeyNum;
}
void LED2_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_0)==RESET)
		GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);
	else
		GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);
}

void LED1_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1)==RESET)
		GPIO_WriteBit(GPIOA,GPIO_Pin_1,Bit_SET);
	else
		GPIO_WriteBit(GPIOA,GPIO_Pin_1,Bit_RESET);
}
