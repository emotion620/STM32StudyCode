#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

#define OLED_WIDTH			128
#define OLED_HEIGHT			64
#define OLED_CHAR_W			6
#define OLED_CHAR_H			12

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Update(void);
void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t color);
void OLED_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len);

#endif
