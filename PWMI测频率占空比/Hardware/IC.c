#include "stm32f10x.h"                  // Device header

void IC_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	//TIM2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//PA6测CH1的PWM
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	TIM_InternalClockConfig(TIM3);
	TIM_TimeBaseInitTypeDef TIM_TimeInitStruct;
	TIM_TimeInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;//定时器内部时钟分频不是PSC
	TIM_TimeInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeInitStruct.TIM_Period=65535-1;				//ARR的值
	TIM_TimeInitStruct.TIM_Prescaler=72-1;				//PSC
	TIM_TimeInitStruct.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM3,&TIM_TimeInitStruct);
	
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_ICInitStructure.TIM_Channel=TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter=0xF;
	TIM_ICInitStructure.TIM_ICPolarity=TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler=TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICSelection=TIM_ICSelection_DirectTI;
	TIM_ICInit(TIM3,&TIM_ICInitStructure);
	TIM_PWMIConfig(TIM3,&TIM_ICInitStructure);
	
	TIM_SelectInputTrigger(TIM3,TIM_TS_TI1FP1);
	TIM_SelectSlaveMode(TIM3,TIM_SlaveMode_Reset);
	
	TIM_Cmd(TIM3,ENABLE);
}

uint32_t GetICFreq(void)
{
	return 1000000 / (TIM_GetCapture1(TIM3)+1);
}

uint16_t GetICDuty(void)
{
	return (TIM_GetCapture2(TIM3)+1)*100/(TIM_GetCapture1(TIM3)+1);
	//Capture1测周期，Capture2测高电对应CCR1,CCR2
}
