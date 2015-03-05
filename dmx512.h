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

#ifndef __DMX512_H
#define __DMX512_H

#include "stm32f0xx.h"

/* Initial 0 + SC + 512 bytes of data */
#define DMX512_DATA_LENGTH 512
__IO uint8_t DMX512_Data[DMX512_DATA_LENGTH];

typedef enum {
	DMX512_STATE_OFF,		// Uninitialized
	DMX512_STATE_COMPLETE,	// Data complete or nothing has been done yet
	DMX512_STATE_IDLE,		// Waiting for break
	DMX512_STATE_IN_BREAK,
	DMX512_STATE_IN_DATA,	// Data is being received
	DMX512_STATE_ERROR		// Error
} DMX512_State_TypeDef;

DMX512_State_TypeDef DMX512_Get_State(void);

void DMX512_Init(void);
void DMX512_Receive(void);

uint8_t DMX512_Get_Channel(uint16_t channel);

void USART1_IRQHandler(void);
void DMA1_Channel4_5_IRQHandler(void);
void TIM14_IRQHandler(void);

#endif /* __DMX512_H */
