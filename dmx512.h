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
