#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "ADC.h"

uint16_t ADValue1,ADValue2;
float Voltage1,Voltage2;
int main(void)
{
	OLED_Init();
	AD_Init();
	OLED_ShowString(1,1,"ADValue:");
	OLED_ShowString(2,1,"Vlotage:");
	OLED_ShowString(2,10,".");	
	OLED_ShowString(3,1,"ADValue:");
	OLED_ShowString(4,1,"Vlotage:");
	OLED_ShowString(4,10,".");	
	
	while(1)        
	{
		ADValue1=GetADCAverageValue(ADC_Channel_0);
		OLED_ShowNum(1,9,ADValue1,4);
		Voltage1 = ADValue1 * 3.3 / 4095.0;
		OLED_ShowNum(2,9,Voltage1,1);
		OLED_ShowNum(2,11,(uint16_t)(Voltage1*100)%100,2);
		ADValue2=GetADCAverageValue(ADC_Channel_2);
		OLED_ShowNum(3,9,ADValue2,4);
		Voltage2 = ADValue2 * 3.3 / 4095.0;
		OLED_ShowNum(4,9,Voltage2,1);
		OLED_ShowNum(4,11,(uint16_t)(Voltage2*100)%100,2);

	}
}
