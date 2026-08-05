#include "stm32f10x.h"                  // Device header

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructureA;
	GPIO_InitStructureA.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructureA.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStructureA.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructureA);
	
	GPIO_InitTypeDef GPIO_InitStructureB;
	GPIO_InitStructureB.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructureB.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructureB.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructureB);
}
