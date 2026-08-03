#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "serial.h"

int main(void)
{
	OLED_Init();
	Serial_Init();
	Serial_SendData('a');
	while(1)
	{
		
	}
}
