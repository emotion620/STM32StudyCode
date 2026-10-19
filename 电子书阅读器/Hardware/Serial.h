#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdint.h>

void Serial_Init(void);
void Serial_SendByte(uint8_t byte);
void Serial_SendString(const char *str);
void Serial_SendBuf(const uint8_t *buf, uint16_t len);
uint16_t Serial_Available(void);
int16_t Serial_ReadByte(void);					/* 没有数据返回 -1 */
int16_t Serial_ReadByteTimeout(uint32_t ms);	/* 超时返回 -1 */

#endif
