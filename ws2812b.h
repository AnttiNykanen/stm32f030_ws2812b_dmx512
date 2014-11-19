#ifndef __WS2812B_H
#define __WS2812B_H

#include <stdint.h>

void WS2812B_Init_GPIO(void);
void WS2812B_Init_Timer(void);
void WS2812B_Init_DMA(void);

void WS2812B_Send_Buffer(void);
uint8_t WS2812B_Get_TC(void);
void WS2812B_Set_Pixel(uint8_t bank, uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue);


void DMA1_Channel2_3_IRQHandler(void);
void TIM3_IRQHandler(void);

#endif /* __WS2812B_H */
