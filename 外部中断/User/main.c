#include "stm32f10x.h"                  // Device header
#include "Encoder.h"
#include "OLED.h"
#include "Delay.h"

int main(void)
{
	int32_t num=0;
	OLED_Init();
	Encoder_Init();
	OLED_ShowString(1,1,"Encoder:00000");
	while(1)
	{
		num+=Get_EncoderNum();
		OLED_ShowSignedNum(1,9,num,5);
		Delay_ms(100);
	}
}
