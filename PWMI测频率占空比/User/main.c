#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "PWM.h"
#include "IC.h"

int main(void)
{
	OLED_Init();
	PWM_Init();
	IC_Init();
	OLED_ShowString(1,1,"Freq:00000(Hz)");
	OLED_ShowString(2,1,"Duty:00(%)");
	TIM_SetCompare1Value(42);//CCR   Duty = CCR / (ARR + 1)
	TIM_SetPerscale(72-1); //  PSC  Freq = CK_PSC / (PSC + 1) / (ARR + 1)
	while(1)
	{
		OLED_ShowNum(1,6,GetICFreq(),5);
		OLED_ShowNum(2,6,GetICDuty(),2);
	}
}
