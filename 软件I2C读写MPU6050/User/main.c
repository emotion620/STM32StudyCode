#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MPU6050.h"

int main(void)
{
	OLED_Init();
	MPU6050_Init();
	MPU6050_WriteReg(0x6B,0x00);
	MPU6050_WriteReg(0x19,0x21);
	
	uint8_t ID=MPU6050_ReadReg(0x19);
	OLED_ShowString(1,1,"ID:");
	OLED_ShowHexNum(1,4,ID,2);
	while(1)
	{
		
	}
}
