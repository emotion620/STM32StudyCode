#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void PWM_Init(void)
{	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;		
	TIM_TimeBaseInitStructure.TIM_Period=100-1;		//ARR
	TIM_TimeBaseInitStructure.TIM_Prescaler=720-1;		//PSC
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter=0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);

	TIM_OCInitTypeDef TIM_OCStructInitStructure_1;
	TIM_OCStructInit(&TIM_OCStructInitStructure_1);
	TIM_OCStructInitStructure_1.TIM_OCMode=TIM_OCMode_PWM1;//输出比较模式
	TIM_OCStructInitStructure_1.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCStructInitStructure_1.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCStructInitStructure_1.TIM_Pulse=0x0000;			//设置CCR的值
	TIM_OC1Init(TIM2,&TIM_OCStructInitStructure_1);
	
	TIM_OCInitTypeDef TIM_OCStructInitStructure_2;
	TIM_OCStructInit(&TIM_OCStructInitStructure_2);
	TIM_OCStructInitStructure_2.TIM_OCMode=TIM_OCMode_PWM1;//输出比较模式
	TIM_OCStructInitStructure_2.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCStructInitStructure_2.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCStructInitStructure_2.TIM_Pulse=0x0000;			//设置CCR的值
	TIM_OC2Init(TIM2,&TIM_OCStructInitStructure_2);
	
	TIM_OCInitTypeDef TIM_OCStructInitStructure_3;
	TIM_OCStructInit(&TIM_OCStructInitStructure_3);
	TIM_OCStructInitStructure_3.TIM_OCMode=TIM_OCMode_PWM1;//输出比较模式
	TIM_OCStructInitStructure_3.TIM_OCPolarity=TIM_OCPolarity_High;
	TIM_OCStructInitStructure_3.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCStructInitStructure_3.TIM_Pulse=0x0000;			//设置CCR的值
	TIM_OC3Init(TIM2,&TIM_OCStructInitStructure_3);
	
	TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Enable);
	TIM_OC2PreloadConfig(TIM2,TIM_OCPreload_Enable);
	TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);
	
	TIM_Cmd(TIM2,ENABLE);		//最后开启时钟
}

void RGB_SetColor(uint16_t R,uint16_t G,uint16_t B)
{
    TIM_SetCompare1(TIM2,R);   // PA0 红
    TIM_SetCompare2(TIM2,G);   // PA1 绿
    TIM_SetCompare3(TIM2,B);   // PA2 蓝
}

void RGB_Breath(void)
{
    int i;

    while(1)
    {
        // 灭 -> 亮
        for(i=0;i<=99;i++)
        {
            RGB_SetColor(i,i,i);
            Delay_ms(8);
        }
        // 亮 -> 灭
        for(i=99;i>=0;i--)
        {
            RGB_SetColor(i,i,i);
            Delay_ms(8);
        }
    }
}
