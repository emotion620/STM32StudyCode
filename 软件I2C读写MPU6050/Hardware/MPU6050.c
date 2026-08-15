#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "MPUReg.h"

#define MPU6050Address 0xD0

void MPU6050_WriteReg(uint8_t RegAddress,uint8_t Data)
{
	MyI2C_Start();
	MyI2C_SendByte(MPU6050Address);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(Data);
	MyI2C_ReceiveAck();
	MyI2C_Stop();
	
}

uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	MyI2C_Start();
	MyI2C_SendByte(MPU6050Address);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(RegAddress);
	MyI2C_ReceiveAck();
	
	MyI2C_Start();
	MyI2C_SendByte(MPU6050Address| 0x01);
	MyI2C_ReceiveAck();
	Data=MyI2C_ReceiveByte();
	MyI2C_SendAck(1);
	MyI2C_Stop();	
	return Data;
}

void MPU6050_Init(void)
{
	MyI2C_Init();
	MPU6050_WriteReg(PWR_MGMT_1,0x01);
	MPU6050_WriteReg(SMPLRT_DIV,0x09);
	MPU6050_WriteReg(CONFIG,0x03);
	MPU6050_WriteReg(GYRO_CONFIG,0x00);	//+-250
	MPU6050_WriteReg(ACCEL_CONFIG,0x00);//+-2G
}

void MPU6050_GetData(int16_t* Accx, int16_t* Accy, int16_t* Accz,
					int16_t* Gyrox, int16_t* Gyroy, int16_t* Gyroz)
{
	uint8_t AccxH,AccxL;
	AccxH=MPU6050_ReadReg(ACCEL_XOUT_H);
	AccxL=MPU6050_ReadReg(ACCEL_XOUT_L);	
	*Accx=(AccxH<<8)|AccxL;
	
	uint8_t AccyH,AccyL;
	AccyH=MPU6050_ReadReg(ACCEL_YOUT_H);
	AccyL=MPU6050_ReadReg(ACCEL_YOUT_L);	
	*Accy=(AccyH<<8)|AccyL;

	uint8_t AcczH,AcczL;
	AcczH=MPU6050_ReadReg(ACCEL_ZOUT_H);
	AcczL=MPU6050_ReadReg(ACCEL_ZOUT_L);	
	*Accz=(AcczH<<8)|AcczL;
	
	uint8_t GyroxH,GyroxL;
	GyroxH=MPU6050_ReadReg(GYRO_XOUT_H);
	GyroxL=MPU6050_ReadReg(GYRO_XOUT_L);	
	*Gyrox=(GyroxH<<8)|GyroxL;
	
	uint8_t GyroyH,GyroyL;
	GyroxH=MPU6050_ReadReg(GYRO_YOUT_H);
	GyroxL=MPU6050_ReadReg(GYRO_YOUT_H);	
	*Gyroy=(GyroyH<<8)|GyroyL;
	
	uint8_t GyrozH,GyrozL;
	GyrozH=MPU6050_ReadReg(GYRO_ZOUT_H);
	GyrozL=MPU6050_ReadReg(GYRO_ZOUT_H);	
	*Gyroz=(GyrozH<<8)|GyrozL;			
}

