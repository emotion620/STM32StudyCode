#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void PWM_Init(void)
{	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;		
	TIM_TimeBaseInitStructure.TIM_Period=20000-1;
	TIM_TimeBaseInitStructure.TIM_Prescaler=72-1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);

	TIM_OCInitTypeDef TIM_OCStructInitStructure;
	TIM_OCStructInit(&TIM_OCStructInitStructure);
	TIM_OCStructInitStructure.TIM_OCMode=TIM_OCMode_PWM1;//输出比较模式
	TIM_OCStructInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCStructInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCStructInitStructure.TIM_Pulse=0x0000;			//设置CCR的值
	TIM_OC2Init(TIM2,&TIM_OCStructInitStructure);
	
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_Cmd(TIM2,ENABLE);		//最后开启时钟
}

void ServoSetAngle(uint16_t angle)
{
    TIM_SetCompare2(TIM2,500 + angle * 2000 / 180);
}

uint16_t GetKeyNum(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	uint16_t keynum=1;
	if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
	{
		Delay_ms(20);
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0);
		keynum=0;
	}
	return keynum;
}


