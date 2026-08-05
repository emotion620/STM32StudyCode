#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Delay.h"
#include "MyDMA.h"

uint8_t Adda[4]={0x00,0x01,0x02,0x03};
uint8_t Addb[4];
int main(void)
{
	OLED_Init();
	MyDMA_Init((uint32_t)Adda,(uint32_t)Addb,4);
	OLED_ShowString(1,1,"a[4]:");
	OLED_ShowString(3,1,"b[4]:");
	OLED_ShowHexNum(1,8,(uint32_t)Adda,8);
	OLED_ShowHexNum(3,8,(uint32_t)Addb,8);
	
	OLED_ShowHexNum(2,1,Adda[0],2);
	OLED_ShowHexNum(2,4,Adda[1],2);
	OLED_ShowHexNum(2,7,Adda[2],2);
	OLED_ShowHexNum(2,10,Adda[3],2);
	                                
	OLED_ShowHexNum(4,1,Addb[0],2);  
	OLED_ShowHexNum(4,4,Addb[1],2);  
	OLED_ShowHexNum(4,7,Addb[2],2);
	OLED_ShowHexNum(4,10,Addb[3],2);
	
	while(1)        
	{
		OLED_ShowHexNum(2,1,Adda[0],2);
		OLED_ShowHexNum(2,4,Adda[1],2);
		OLED_ShowHexNum(2,7,Adda[2],2);
		OLED_ShowHexNum(2,10,Adda[3],2);
		OLED_ShowHexNum(4,1,Addb[0],2);  
		OLED_ShowHexNum(4,4,Addb[1],2);  
		OLED_ShowHexNum(4,7,Addb[2],2);
		OLED_ShowHexNum(4,10,Addb[3],2);
		Delay_ms(1000);
		
		DMA_Transfer(4);
		
		OLED_ShowHexNum(4,1,Addb[0],2);  
		OLED_ShowHexNum(4,4,Addb[1],2);  
		OLED_ShowHexNum(4,7,Addb[2],2);
		OLED_ShowHexNum(4,10,Addb[3],2);
		Delay_ms(1000);
		
		Adda[0]++;
		Adda[1]++;
		Adda[2]++;
		Adda[3]++;
	}
}
