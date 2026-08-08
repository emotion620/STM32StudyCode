#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "PWM.h"
#include "OLED.h"

int main(void)
{
    PWM_Init();
	OLED_Init();
	uint16_t keynum=1,angle=0;
	OLED_ShowString(1,1,"Angle:");
    while(1)
    {
		keynum=GetKeyNum();
		if(keynum==0)
		{
			angle+=90;
			if(angle>180)
				angle=0;
		}
		OLED_ShowNum(1,7,angle,3);
		ServoSetAngle(angle);

    }
}
