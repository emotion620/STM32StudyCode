#ifndef __PWM_H__
#define __PWM_H__

void PWM_Init(void);
void TIM_SetCompare1Value(uint16_t Compare1Value);
void TIM_SetPerscale(uint16_t PerscaleValue);

#endif
