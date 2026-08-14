#ifndef  __MPU6050_H__
#define  __MPU6050_H__

void MPU6050_WriteReg(uint8_t RegAddress,uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);
void MPU6050_Init(void);

#endif
