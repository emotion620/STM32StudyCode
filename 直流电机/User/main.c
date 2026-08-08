#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Motor.h"
#include "OLED.h"
#include "Delay.h"
#include "PWM.h"

int main(void)
{	
	uint16_t Speed=0,KeyNum=1;
	Motor_Init();
	OLED_Init();
	OLED_ShowString(1,1,"Speed:");
	while(1)
	{
		KeyNum=GetKeyNum();
		if(KeyNum==0)
		{
			Speed+=10;
			if(Speed>60)
				Speed=0;
		}
		OLED_ShowNum(1,7,Speed,2);
		MotorSetpeed(Speed);
	}
}
