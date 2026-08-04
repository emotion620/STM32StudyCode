#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdio.h>
void Serial_Init(void);
void Serial_SendData(uint16_t date);
void Serial_SendArry(uint8_t * arry,uint16_t length);
void Serial_SendString(uint8_t * String);
void Serial_SendNumber(uint32_t Number);

#endif
