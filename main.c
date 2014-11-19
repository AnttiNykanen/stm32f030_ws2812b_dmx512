#include "dmx512.h"
#include "ws2812b.h"

int main(void)
{
#if 0
	WS2812B_Init_GPIO();
	WS2812B_Init_Timer();
	WS2812B_Init_DMA();
#endif
	while(1) {
#if 0
    	while (!WS2812B_Get_TC());
    	WS2812B_Send_Buffer();
#endif
    }
}
