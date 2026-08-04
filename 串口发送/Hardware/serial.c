#include "stm32f10x.h"                  // Device header

extern uint16_t Rx_GetData;
void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=9600;
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx |USART_Mode_Tx;
	USART_InitStructure.USART_Parity=USART_Parity_No;
	USART_InitStructure.USART_StopBits=USART_StopBits_1;
	USART_InitStructure.USART_WordLength=USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStructure);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_Init(&NVIC_InitStructure);
	USART_Cmd(USART1,ENABLE);
}

void Serial_SendData(uint16_t data)
{
	USART_SendData(USART1,data);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
}

void Serial_SendArry(uint8_t * arry,uint16_t length)
{
	uint16_t i=0;
	for(;i<length;i++)
	{
		USART_SendData(USART1,arry[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
		//串口发送需要时间等待
	}

}

void Serial_SendString(char * String)
{
	uint16_t i;
	for(i=0;String[i]!='\0';i++)
	{
		USART_SendData(USART1,String[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);		
	}
}
/*
void Serial_SendArray(uint8_t *Array,uint16_t Length)
{
    uint16_t i;

    for(i=0;i<Length;i++)
    {
        Serial_SendByte(Array[i]);
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
    }
}
*/

void Serial_SendNumber(uint32_t Number)
{
    uint8_t i;
    uint8_t NumArray[10];

    i = 0;

    // 拆数字
    while(Number > 0)
    {
        NumArray[i] = Number % 10;  // 取个位
        Number /= 10;               // 去掉个位
        i++;
    }
    // 反向发送
    while(i > 0)
    {
        i--;
        Serial_SendData(NumArray[i] + '0');
    }
}

void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET)
	{
		Rx_GetData=USART_ReceiveData(USART1);
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	}

}
