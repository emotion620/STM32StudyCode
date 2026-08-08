#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

void DHT11_Output(void);
void DHT11_Input(void);
uint8_t DHT11_Start(void);
uint8_t DHT11_ReadBit(void);
uint8_t DHT11_ReadByte(void);
uint8_t DHT11_ReadData(uint8_t *buf);

#endif
