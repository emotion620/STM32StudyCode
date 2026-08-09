#include "stm32f10x.h"                  // Device header

void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);	//TIM2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//PA0输出PWM
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	TIM_InternalClockConfig(TIM2);
	TIM_TimeBaseInitTypeDef TIM_TimeInitStruct;
	TIM_TimeInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;//定时器内部时钟分频不是PSC
	TIM_TimeInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeInitStruct.TIM_Period=100-1;				//ARR的值
	TIM_TimeInitStruct.TIM_Prescaler=720-1;				//PSC
	TIM_TimeInitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeInitStruct);
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse=50;					//CCR,UINT16_T
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);
	
	TIM_Cmd(TIM2,ENABLE);
	
}

void TIM_SetCompare1Value(uint16_t Compare1Value)
{
	TIM_SetCompare1(TIM2,Compare1Value);
}

void TIM_SetPerscale(uint16_t PerscaleValue)
{
	TIM_PrescalerConfig(TIM2,PerscaleValue ,TIM_PSCReloadMode_Immediate);
}
