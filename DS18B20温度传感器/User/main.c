#include "stm32f10x.h"
#include "DS18B20.h"
#include "OLED.h"
#include "Delay.h"
#include "OneWire.h"

float T;
int main(void)
{
    OLED_Init();
	GPIOAPin0_Init();
	Onewire_Init();
    OLED_ShowString(1,1,"Temperature:");

    while(1)
    {
		DS18B20_Conver_T();
		Delay_ms(750);
		T=DS18B20_GetT();
		if(T<0)				//如果温度小于0
		{
			OLED_ShowChar(2,1,'-');	//显示负号
			T=-T;			//将温度变为正数
		}
		else				//如果温度大于等于0
		{
			OLED_ShowChar(2,1,'+');	//显示正号
		}
		OLED_ShowNum(2,2,T,3);		//显示温度整数部分
		OLED_ShowChar(2,5,'.');		//显示小数点
		OLED_ShowNum(2,6,(unsigned long)(T*10000)%10000,4);//显示温度小数部分
		Delay_ms(100);
	}
}
