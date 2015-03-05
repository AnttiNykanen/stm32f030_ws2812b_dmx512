/*
 * STM32F030 DMX512 library
 *
 * Copyright (c) 2015 Antti Nykanen <aon at umetronics dot com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "dmx512.h"

#include "stm32f0xx_dma.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"

/*
 * Local function definitions
 */
void DMX512_Init_RCC(void);
void DMX512_Init_GPIO(void);
void DMX512_Init_USART(void);
void DMX512_Init_DMA(void);
void DMX512_Init_Timer(void);

void DMX512_Set_State(DMX512_State_TypeDef state);
void DMX512_Set_State_USART(DMX512_State_TypeDef state);
void DMX512_Set_State_Timer(DMX512_State_TypeDef state);
/* -- */

/* Internal state variable */
__IO DMX512_State_TypeDef DMX512_State = DMX512_STATE_OFF;

/* Shared NVIC initialization structure */
static NVIC_InitTypeDef NVIC_InitStructure;

void DMX512_Init(void)
{
	DMX512_Init_RCC();
	DMX512_Init_GPIO();
	DMX512_Init_USART();
	DMX512_Init_DMA();
	DMX512_Init_Timer();

	DMX512_Set_State(DMX512_STATE_COMPLETE);
}

void DMX512_Init_RCC(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG | RCC_APB2Periph_USART1, ENABLE);
}

void DMX512_Init_GPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure GPIO pin 10 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Enable GPIO pin 10 alternate function */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);
}

void DMX512_Init_USART(void)
{
	USART_InitTypeDef USART_InitStructure;
	/* Shared NVIC_InitStructure */

	USART_InitStructure.USART_BaudRate = 250000;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = 2;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART1, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}

void DMX512_Init_DMA(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	/* Shared NVIC_InitStructure */

	/* Remap USART1 Rx to DMA1 Channel5 */
	SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART1Rx, ENABLE);

	DMA_InitStructure.DMA_BufferSize = (uint32_t)DMX512_DATA_LENGTH;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)DMX512_Data;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr = ((uint32_t)&(USART1->RDR));
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void DMX512_Init_Timer(void)
{
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	/* Shared NVIC_InitStructure */

	/* Assuming 8MHz clock */
	TIM_InitStructure.TIM_ClockDivision = 0;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStructure.TIM_Prescaler = 11;
	TIM_InitStructure.TIM_Period = 0xFFFF;
	TIM_InitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM14, &TIM_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

DMX512_State_TypeDef DMX512_Get_State(void)
{
	return DMX512_State;
}

void DMX512_Set_State(DMX512_State_TypeDef state)
{
	DMX512_State = state;

	DMX512_Set_State_USART(state);
	DMX512_Set_State_Timer(state);
}

void DMX512_Set_State_USART(DMX512_State_TypeDef state)
{
	if (state == DMX512_STATE_IN_DATA) {
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);

		DMA_ClearFlag(DMA1_FLAG_TC5 | DMA1_FLAG_HT5 | DMA1_FLAG_GL5 | DMA1_FLAG_TE5);
		DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);

		DMA_SetCurrDataCounter(DMA1_Channel5, (uint16_t)DMX512_DATA_LENGTH);
		DMA_Cmd(DMA1_Channel5, ENABLE);
	} else {
		DMA_Cmd(DMA1_Channel5, DISABLE);

		if (state == DMX512_STATE_IDLE) {
			USART_ITConfig(USART1, USART_IT_ERR, ENABLE);
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		} else if (state == DMX512_STATE_IN_BREAK) {
			USART_ITConfig(USART1, USART_IT_ERR, DISABLE);
		}
	}
}

void DMX512_Set_State_Timer(DMX512_State_TypeDef state)
{
	if (state == DMX512_STATE_IN_DATA) {
		TIM14->SR = 0;
		TIM_SetCounter(TIM14, 0);
		TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
		TIM_Cmd(TIM14, ENABLE);
 	} else {
		TIM_ITConfig(TIM14, TIM_IT_Update, DISABLE);
		TIM_Cmd(TIM14, DISABLE);
	}
}

void DMX512_Receive(void)
{
	if (DMX512_State != DMX512_STATE_IDLE)
		DMX512_Set_State(DMX512_STATE_IDLE);
}

void USART1_IRQHandler(void)
{
	uint8_t data;

	data = (uint8_t)(USART_ReceiveData(USART1) & 0xFF);

	if (USART_GetITStatus(USART1, USART_IT_FE)) {
		if (data == 0x00 && DMX512_State == DMX512_STATE_IDLE)
			DMX512_Set_State(DMX512_STATE_IN_BREAK);

		USART_ClearFlag(USART1, USART_FLAG_FE);
	} else if (DMX512_State == DMX512_STATE_IN_BREAK && !USART_GetFlagStatus(USART1, USART_FLAG_FE)) {
		DMX512_Set_State(DMX512_STATE_IN_DATA);
	}

	USART_ClearFlag(USART1, USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_ORE | USART_FLAG_RXNE);
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	USART_ClearITPendingBit(USART1, USART_IT_FE);
}

void TIM14_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
	DMX512_Set_State(DMX512_STATE_ERROR);
}

void DMA1_Channel4_5_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_IT_TC5);
	DMX512_Set_State(DMX512_STATE_COMPLETE);
}
