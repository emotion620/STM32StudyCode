#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void DHT11_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
}

void DHT11_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
}

uint8_t DHT11_Start(void)
{
    DHT11_Output();
    GPIO_ResetBits(GPIOA,GPIO_Pin_0);
    Delay_ms(20);
    GPIO_SetBits(GPIOA,GPIO_Pin_0);
    DHT11_Input();
    Delay_us(40);
    //等待DHT11拉低
    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==1)
    {
        return 0;
    }
    //等待80us低电平结束
    while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==0);
    //等待80us高电平结束
    while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==1);
    return 1;
}
uint8_t DHT11_ReadBit(void)
{
    uint8_t data;
    //等待50us低电平结束
    while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==0);
    Delay_us(30);

    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))
    {
        data=1;
    }
    else
    {
        data=0;
    }
    //等待该bit结束
    while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0));
    return data;
}

uint8_t DHT11_ReadByte(void)
{
    uint8_t i;
    uint8_t data = 0;
    for(i=0;i<8;i++)
    {
        data <<= 1;          //左移，给下一位腾位置
        if(DHT11_ReadBit())
        {
            data |= 1;       //当前bit为1
        }
    }
    return data;
}

uint8_t DHT11_ReadData(uint8_t *buf)
{
    uint8_t i;
    //发送开始信号
    if(DHT11_Start()==0)
    {
        return 0;
    }
    //读取5个字节
    for(i=0;i<5;i++)
    {
        buf[i]=DHT11_ReadByte();
    }
    //校验
    if(buf[4] == buf[0]+buf[1]+buf[2]+buf[3])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
