#ifndef  __ADC_H__
#define  __ADC_H__

void AD_Init(void);
uint16_t GetADConvertVaule(uint8_t ADC_Channel);
uint16_t GetADCAverageValue(uint8_t ADC_Channel);

#endif
