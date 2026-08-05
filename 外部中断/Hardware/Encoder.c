#include "stm32f10x.h"                  // Device header

int32_t EncoderNum;
void Encoder_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource10);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource11);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line=EXTI_Line10 | EXTI_Line11;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;

	EXTI_Init(&EXTI_InitStructure);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
}

int32_t Get_EncoderNum(void)
{
	int32_t temp;
	temp=EncoderNum;
	EncoderNum=0;
	return temp;	
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line11)==SET)
	{
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10)==RESET)
			EncoderNum++;
		EXTI_ClearITPendingBit(EXTI_Line11);
	}
	
	if(EXTI_GetITStatus(EXTI_Line10)==SET)
	{
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)==RESET)
			EncoderNum--;
		EXTI_ClearITPendingBit(EXTI_Line10);
	}
}




