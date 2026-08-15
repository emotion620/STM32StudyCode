#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "Delay.h"

#define SHT31_ADDR 0x44

void SHT31_Init(void)
{
    MyI2C_Init();
}

void SHT31_Read(float *Temperature,float *Humidity)
{
    uint8_t Buffer[6];

    uint16_t TempRaw;
    uint16_t HumiRaw;

    MyI2C_Start();

    MyI2C_SendByte(SHT31_ADDR<<1);

    MyI2C_ReceiveAck();

    MyI2C_SendByte(0x2C);

    MyI2C_ReceiveAck();

    MyI2C_SendByte(0x06);

    MyI2C_ReceiveAck();

    MyI2C_Stop();

    Delay_ms(15);

    MyI2C_Start();

    MyI2C_SendByte((SHT31_ADDR<<1)|1);

    MyI2C_ReceiveAck();

    for(int i=0;i<6;i++)
    {
        Buffer[i]=MyI2C_ReceiveByte();

        if(i==5)
        {
            MyI2C_SendAck(1);
        }
        else
        {
            MyI2C_SendAck(0);
        }
    }

    MyI2C_Stop();

    TempRaw=(Buffer[0]<<8)|Buffer[1];

    HumiRaw=(Buffer[3]<<8)|Buffer[4];

    *Temperature=
        -45+
        175*
        TempRaw/
        65535.0f;

    *Humidity=
        100*
        HumiRaw/
        65535.0f;
}
