#include "stm32f10x.h"                  // Device header

#define SCL(x) GPIO_WriteBit(GPIOB,GPIO_Pin_10,(BitAction)(x));
#define SDA(x) GPIO_WriteBit(GPIOB,GPIO_Pin_11,(BitAction)(x));
#define SDA_Read GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)

void MyI2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	SCL(1);
	SDA(1);
}

void MyI2C_Start(void)
{	
	SDA(1);	//这里SDA要尽量在SCL前面，以防误以为是STOP
	SCL(1);
	SDA(0);
	SCL(0);	
}

void MyI2C_Stop(void)
{	
	SDA(0);
	SCL(0);	
	SCL(1);
	SDA(1);
}

void MyI2C_SendByte(uint8_t Data)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		SDA(Data&(0x80>>i));
		//然后释放SCL，从机将在SCL高电平期间读取数据位	
		SCL(1);
		SCL(0);
	}
	//在发送积木还没拼成前，不需要加上ACK
}

uint8_t MyI2C_ReceiveByte(void)
{
	uint8_t Data=0x00,i;
	SDA(1);
	for(i=0;i<8;i++)
	{	
		SCL(1);
		if(SDA_Read==1)
			Data|=(0x80>>i);
		SCL(0);
	}
	return Data;
}

void MyI2C_SendAck(uint8_t ACK)//主机在接收完一个字节之后
{
	SDA(ACK);
	//然后释放SCL，从机将在SCL高电平期间读取数据位	
	SCL(1);
	SCL(0);
}

uint8_t MyI2C_ReceiveAck(void)//主机在发送完一个字节之后
{
	uint8_t ACK;
	SDA(1);
	SCL(1);
	ACK=SDA_Read;
	SCL(0);
	return ACK;
}

