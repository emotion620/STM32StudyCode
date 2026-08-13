#include "stm32f10x.h"                  // Device header

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;//单ADC还是双ADC模式
	ADC_InitStructure.ADC_NbrOfChannel=1;
	ADC_InitStructure.ADC_ScanConvMode=DISABLE;
	ADC_Init(ADC1,&ADC_InitStructure);
	
	/* 打开 ADC */
	ADC_Cmd(ADC1, ENABLE);

	/* 复位校准寄存器 */
	ADC_ResetCalibration(ADC1);

	/* 等待复位完成 */
	while (ADC_GetResetCalibrationStatus(ADC1));

	/* 开始校准 */
	ADC_StartCalibration(ADC1);

	/* 等待校准完成 */
	while (ADC_GetCalibrationStatus(ADC1));
	
}

uint16_t GetADConvertVaule(uint8_t ADC_Channel)
{
	ADC_RegularChannelConfig(ADC1,ADC_Channel,1,ADC_SampleTime_55Cycles5);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET);
	return ADC_GetConversionValue(ADC1);
}

uint16_t GetADCAverageValue(uint8_t ADC_Channel)
{
    uint32_t Sum = 0;

    for(uint8_t i = 0; i < 10; i++)
    {
        Sum += GetADConvertVaule(ADC_Channel);
    }

    return Sum / 10;
}

