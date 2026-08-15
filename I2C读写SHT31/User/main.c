#include "stm32f10x.h"
#include "OLED.h"
#include "Delay.h"
#include "SHT31.h"

float Temperature;
float Humidity;

int main(void)
{
    OLED_Init();

    SHT31_Init();

    OLED_ShowString(1,1,"Temp:");

    OLED_ShowString(3,1,"Humi:");

    while(1)
    {
        SHT31_Read(&Temperature,&Humidity);

        OLED_ShowNum(1,7,(int)Temperature,2);

        OLED_ShowChar(1,9,'.');

        OLED_ShowNum(1,10,
                     (int)(Temperature*10)%10,
                     1);

        OLED_ShowNum(3,7,(int)Humidity,2);

        OLED_ShowChar(3,9,'.');

        OLED_ShowNum(3,10,
                     (int)(Humidity*10)%10,
                     1);

        Delay_ms(500);
    }
}