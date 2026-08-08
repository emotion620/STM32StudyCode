#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void PWM_Init(void)
{	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;		
	TIM_TimeBaseInitStructure.TIM_Period=100-1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler=72-1;		//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);

	TIM_OCInitTypeDef TIM_OCStructInitStructure_3;
	TIM_OCStructInit(&TIM_OCStructInitStructure_3);
	TIM_OCStructInitStructure_3.TIM_OCMode=TIM_OCMode_PWM1;//输出比较模式
	TIM_OCStructInitStructure_3.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCStructInitStructure_3.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCStructInitStructure_3.TIM_Pulse=0x0000;			//设置CCR的值
	TIM_OC3Init(TIM2,&TIM_OCStructInitStructure_3);
	
	TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_Cmd(TIM2,ENABLE);		//最后开启时钟
}

void MotorSetpeed(uint16_t Speed)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	GPIO_SetBits(GPIOA,GPIO_Pin_5);
    TIM_SetCompare3(TIM2,Speed);
}

