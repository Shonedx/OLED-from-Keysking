#include "stm32f10x.h"                  // Device header
#include "Delay.h" //Delay
#include "OLED.h"
#include "NRF24L01.h"
#include "NRF24L01_DEFINE.h"
#include "HW504.h"
#include "font.h"
 int main (void)
 {

	OLED_Init();
	
	while(1)
	{	
		for(u8 i=0;i<64;i++)
		{
			OLED_NewFrame();
			OLED_DrawImage(0,0,&MANBAImg,OLED_COLOR_NORMAL);
//			OLED_PrintString(80,64-2*i,"翔鸡",&font14x14,OLED_COLOR_NORMAL);
			OLED_ShowFrame();
		}
	}
 }