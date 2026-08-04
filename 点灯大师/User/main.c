#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "Delay.h"

int main(void)
{
	LED_Init();
	while(1)
	{
		LED_Light();
		Delay_ms(1000);
		LED_Fade();
		Delay_ms(1000);
	}
}
