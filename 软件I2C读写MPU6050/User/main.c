#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MPU6050.h"

int16_t Ax,Ay,Az,Gx,Gy,Gz;
int main(void)
{
	
	OLED_Init();
	MPU6050_Init();
	while(1)
	{
		MPU6050_GetData(&Ax,&Ay,&Az,&Gx,&Gy,&Gz);
		OLED_ShowSignedNum(1,1,Ax,5);
		OLED_ShowSignedNum(2,1,Ay,5);
		OLED_ShowSignedNum(3,1,Az,5);

	}
}
