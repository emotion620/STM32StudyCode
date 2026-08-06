#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

void GPIOAPin0_Init(void);
uint8_t Onewire_Init(void);
uint8_t OneWire_ReceiveData(void);
void OneWire_SendBit(uint8_t SendData);
void OneWire_SendByte(uint8_t SendData);
uint8_t OneWire_ReceiveByte(void);

#endif
