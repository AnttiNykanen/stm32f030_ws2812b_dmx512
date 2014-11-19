#include <stdint.h>

#include "stm32f0xx_dma.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

#include "ws2812b.h"

static const uint8_t WS2812B_BUFFER_SIZE = 48;
static const uint8_t WS2812B_DEAD_PERIOD = 1;

__O uint8_t WS2812B_IO_High = 0xFF;
__O uint8_t WS2812B_IO_Low  = 0x00;

__IO uint8_t WS2812B_IO_FrameData[48];

__IO uint8_t WS2812B_TC = 1;
__IO uint8_t WS2812B_TIM_Overflows = 0;

void WS2812B_Init_GPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = 0xFF; /* Pins 0-7 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void WS2812B_Init_Timer(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_Period = 10;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM3, DISABLE);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 2;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);

	TIM_OCInitStructure.TIM_Pulse = 4;
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);

	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void WS2812B_Init_DMA(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	// TIM3 update event, DMA1 Channel 3
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOA->ODR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&WS2812B_IO_High;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_BufferSize = 0; // ???
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	// TIM3 CC1 event, DMA1 Channel 4
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)WS2812B_IO_FrameData; //WS2812B_IO_FrameData[0];
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);

	// TIM3 CC3 event, DMA1 Channel 2
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&WS2812B_IO_Low;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);

	// DMA1 Channel 2 interrupt
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
}

void WS2812B_Send_Buffer(void)
{
	WS2812B_TC = 0;

	DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2 | DMA1_FLAG_TE2);
	DMA_ClearFlag(DMA1_FLAG_TC3 | DMA1_FLAG_HT3 | DMA1_FLAG_GL3 | DMA1_FLAG_TE3);
	DMA_ClearFlag(DMA1_FLAG_TC4 | DMA1_FLAG_HT4 | DMA1_FLAG_GL4 | DMA1_FLAG_TE4);

	DMA_SetCurrDataCounter(DMA1_Channel2, WS2812B_BUFFER_SIZE);
	DMA_SetCurrDataCounter(DMA1_Channel3, WS2812B_BUFFER_SIZE);
	DMA_SetCurrDataCounter(DMA1_Channel4, WS2812B_BUFFER_SIZE);

	TIM3->SR = 0;

	DMA_Cmd(DMA1_Channel2, ENABLE);
	DMA_Cmd(DMA1_Channel3, ENABLE);
	DMA_Cmd(DMA1_Channel4, ENABLE);

	TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
	TIM_DMACmd(TIM3, TIM_DMA_CC3, ENABLE);
	TIM_DMACmd(TIM3, TIM_DMA_Update, ENABLE);

	TIM_SetCounter(TIM3, 10);

	TIM_Cmd(TIM3, ENABLE);
}

uint8_t WS2812B_Get_TC(void)
{
	return WS2812B_TC;
}

void WS2812B_Set_Pixel(uint8_t bank, uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		// Clear data
		WS2812B_IO_FrameData[pixel * 24 + i] &= ~(0x01 << bank);
		WS2812B_IO_FrameData[pixel * 24 + 8 + i] &= ~(0x01 << bank);
		WS2812B_IO_FrameData[pixel * 24 + 16 + i] &= ~(0x01 << bank);

		// Write new data
		WS2812B_IO_FrameData[pixel * 24 + i] |= ((((green<<i) & 0x80)>>7)<<bank);
		WS2812B_IO_FrameData[pixel * 24 + 8 + i] |= ((((red<<i) & 0x80)>>7)<<bank);
		WS2812B_IO_FrameData[pixel * 24 + 16 + i] |= ((((blue<<i) & 0x80)>>7)<<bank);
	}
}

void DMA1_Channel2_3_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC2);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	DMA_Cmd(DMA1_Channel2, DISABLE);
	DMA_Cmd(DMA1_Channel3, DISABLE);
	DMA_Cmd(DMA1_Channel4, DISABLE);

	TIM_DMACmd(TIM3, TIM_DMA_CC1, DISABLE);
	TIM_DMACmd(TIM3, TIM_DMA_CC3, DISABLE);
	TIM_DMACmd(TIM3, TIM_DMA_Update, DISABLE);
}

void TIM3_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_Cmd(TIM3, DISABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
	WS2812B_TC = 1;
}

