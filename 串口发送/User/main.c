#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "serial.h"

uint16_t Rx_GetData;

int main(void)
{
	OLED_Init();
	Serial_Init();
	
	while(1)
	{
		OLED_ShowHexNum(1,1,Rx_GetData,2);
	}
}
//	uint8_t arr[]={0x7d,0x16,0x55};
//	Serial_SendData('a');
//	Serial_SendArry(arr,3);
//	Serial_SendString("HelloWorld\r\n");
//	Serial_SendNumber(12345);
//	uint8_t arr[50];
//	sprintf(arr,"Num=%d\r\n",456);
//	Serial_SendString(arr);




