#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "Key.h"


int main(void)
{
	uint8_t keynum;
	LED_Init();
	Key_Init();
	while(1)
	{
		keynum=Get_KeyNum();
		if(keynum==1)
			LED1_Turn();
		if(keynum==2)
			LED2_Turn();
	}
}
