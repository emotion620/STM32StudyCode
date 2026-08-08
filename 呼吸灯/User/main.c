#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "PWM.h"
#include "OLED.h"
#include "Delay.h"

int main(void)
{
	PWM_Init();

	while(1)
	{
		RGB_Breath();
	}
}
