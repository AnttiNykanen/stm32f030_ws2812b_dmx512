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
