#include "stm32f10x.h"
#include "OLED.h"
#include "Delay.h"
#include "OneWire.h"

uint8_t DHT11_Data[5];

int main(void)
{
	OLED_Init();
	uint8_t Humidity,Temperature;
	uint8_t Humidity_dec,Temperature_dec;
    OLED_ShowString(1,1,"Humidity:");
    OLED_ShowString(3,1,"Temperature:");
	OLED_ShowString(2,7,".");
    OLED_ShowString(4,7,".");

    while(1)
    {
		if(DHT11_ReadData(DHT11_Data))
		{
			Humidity = DHT11_Data[0];
			Humidity_dec= DHT11_Data[1];
			Temperature= DHT11_Data[2];
			Temperature_dec= DHT11_Data[3];
			OLED_ShowNum(2,5,Humidity,2);
			OLED_ShowNum(4,5,Temperature,2);
			OLED_ShowNum(2,8,Humidity_dec,1);
			OLED_ShowNum(4,8,Temperature_dec,1);
			Delay_ms(1000);
		}
	}
}
