#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void GPIOAPin0_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}
uint8_t Onewire_Init(void)
{
	uint8_t ack;
	
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	Delay_us(500);
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	Delay_us(70);
	ack=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);
	Delay_us(500);
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	return ack;
}

uint8_t OneWire_ReceiveData(void)
{
	uint8_t ReceiveData;
	
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	Delay_us(2);
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
	Delay_us(15);
	ReceiveData=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);
	Delay_us(45);
	return ReceiveData;
}

void OneWire_SendBit(uint8_t SendData)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_0);   // 拉低总线
    if (SendData) {
        // 写 1: 稍作延时后释放总线
        Delay_us(2);                     // >1 μs 且 <15 μs
        GPIO_SetBits(GPIOA, GPIO_Pin_0); // 释放总线（拉高）
        Delay_us(60);                    // 维持时隙总时间 ≥ 60 μs
    } else {
        // 写 0: 保持低电平至少 30 μs
        Delay_us(60);
        GPIO_SetBits(GPIOA, GPIO_Pin_0); // 释放总线
        Delay_us(5);                     // 恢复时间
    }
}

void OneWire_SendByte(uint8_t SendData)
{
	uint8_t i=0;
	for(;i<8;i++)
	{
		OneWire_SendBit(SendData&(0x01<<i));
	}
}

uint8_t OneWire_ReceiveByte(void)
{
	uint8_t i;
	uint8_t ReceiveData = 0;

	for(i=0;i<8;i++)
	{
		if(OneWire_ReceiveData())
		{
			ReceiveData |= (0x01 << i);
		}
	}
	return ReceiveData;
}
