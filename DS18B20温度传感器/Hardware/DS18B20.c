#include "stm32f10x.h"                  // Device header
#include "OneWire.h"
#include "OLED.h"

#define SKIP_ROM         	 0xCC
#define CONVERT_T			 0x44
#define READ_SCRATCHPAD		 0xBE

void DS18B20_Conver_T(void)
{
	Onewire_Init();
	OneWire_SendByte(SKIP_ROM);
	OneWire_SendByte(CONVERT_T);

}	

float DS18B20_GetT(void)
{	
	uint8_t LSB,MSB;
	int16_t Temp;
	float T;
	Onewire_Init();
	OneWire_SendByte(SKIP_ROM);
	OneWire_SendByte(READ_SCRATCHPAD);
	LSB=OneWire_ReceiveByte();
	MSB=OneWire_ReceiveByte();
	OLED_ShowString(3,9,":LSB");
	OLED_ShowString(4,9,":MSB");
	OLED_ShowBinNum(3,1,LSB,8);
	OLED_ShowBinNum (4,1,MSB,8);
	Temp=(MSB<<8)|LSB;
	T=Temp/16.0;
	return T;
}


