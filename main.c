/*
 * STM32F030 DMX512 WS2812B driver
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
#include "ws2812b.h"

int main(void)
{
	uint8_t i;
	DMX512_Init();

	WS2812B_Init_GPIO();
	WS2812B_Init_Timer();
	WS2812B_Init_DMA();

	while(1) {
		while (!WS2812B_Get_TC());

		if (DMX512_Get_State() == DMX512_STATE_COMPLETE) {
			for (i = 0; i < 10; i++)
				WS2812B_Set_Pixel(0, i, DMX512_Data[i*3+1], DMX512_Data[i*3+2], DMX512_Data[i*3+3]);
	    	WS2812B_Send_Buffer();
		}

		if (DMX512_Get_State() == DMX512_STATE_COMPLETE || DMX512_Get_State() == DMX512_STATE_ERROR)
			DMX512_Receive();
    }
}
