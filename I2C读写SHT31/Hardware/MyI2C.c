#include "stm32f10x.h"                  // Device header

#include "MyI2C.h"
#include "Delay.h"

#define SCL(x) GPIO_WriteBit(GPIOB,GPIO_Pin_10,(BitAction)(x))

#define SDA(x) GPIO_WriteBit(GPIOB,GPIO_Pin_11,(BitAction)(x))

#define SDA_READ GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)

static void SDA_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;

    GPIO_Init(GPIOB,&GPIO_InitStructure);
}

static void SDA_In(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;

    GPIO_Init(GPIOB,&GPIO_InitStructure);
}

void MyI2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;

    GPIO_Init(GPIOB,&GPIO_InitStructure);

    SDA(1);
    SCL(1);
}

void MyI2C_Start(void)
{
    SDA_Out();

    SDA(1);
    SCL(1);

    SDA(0);

    SCL(0);
}

void MyI2C_Stop(void)
{
    SDA_Out();

    SDA(0);

    SCL(1);

    SDA(1);
}

void MyI2C_SendByte(uint8_t Byte)
{
    uint8_t i;

    SDA_Out();

    for(i=0;i<8;i++)
    {
        SDA(Byte&0x80);

        SCL(1);

        Delay_us(5);

        SCL(0);

        Byte<<=1;
    }
}

uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i;
    uint8_t Byte=0;

    SDA_In();

    for(i=0;i<8;i++)
    {
        Byte<<=1;

        SCL(1);

        if(SDA_READ)
        {
            Byte|=0x01;
        }

        SCL(0);
    }

    return Byte;
}

uint8_t MyI2C_ReceiveAck(void)
{
    uint8_t Ack;

    SDA_In();

    SCL(1);

    Ack=SDA_READ;

    SCL(0);

    return Ack;
}

void MyI2C_SendAck(uint8_t AckBit)
{
    SDA_Out();

    SDA(AckBit);

    SCL(1);

    SCL(0);
}
