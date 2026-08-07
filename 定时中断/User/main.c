#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Delay.h"
#include "ExternCount.h"

uint16_t Count,Carry;
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	OLED_Init();
	ExternCount();
	OLED_ShowString(1,1,"Carry:");
	OLED_ShowString(2,1,"Count:");

	while(1)
	{
		Count=GetCountNum();
		OLED_ShowNum(1,7,Carry,5);
		OLED_ShowNum(2,7,Count,5);
	}
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
		if(Count==9)
		{
			Carry+=1;
		}
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}

