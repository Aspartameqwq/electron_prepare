/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "board.h"
#include "lcd_init.h"
#include "lcd.h"
#include "pic.h"


int main(void)
{
    SYSCFG_DL_init();

	u8 i,j;
	float t = 0;

	LCD_Init();//LCD初始化
	LCD_Fill(0, 0, LCD_W, LCD_H, WHITE);

	while(1)
	{
		LCD_ShowChinese(0,0,"中景园电子",RED,WHITE,32,0);
		LCD_ShowString(0,40,"LCD_W:",RED,WHITE,16,0);
		LCD_ShowIntNum(48,40,LCD_W,3,RED,WHITE,16);
		LCD_ShowString(80,40,"LCD_H:",RED,WHITE,16,0);
		LCD_ShowIntNum(128,40,LCD_H,3,RED,WHITE,16);
		LCD_ShowString(80,40,"LCD_H:",RED,WHITE,16,0);
		LCD_ShowString(0,70,"Increaseing Nun:",RED,WHITE,16,0);
		LCD_ShowFloatNum1(128,70,t,4,RED,WHITE,16);

		t += 0.11;

		for(j = 0; j < 3; j++)
		{
			for(i=0;i<6;i++)
			{
				LCD_ShowPicture(40*i,120+j*40,40,40,gImage_1);
			}
		}

		delay_ms(1000);
		LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
		Display_Asc_String(0,15,7, "ASCII_5x7",RED,WHITE);      //ASC 5X7点阵
		Display_Asc_String(0,25,8, "ASCII_7x8",RED,WHITE);      //ASC 7X8点阵
		Display_Asc_String(0,35,12, "ASCII_6x12",RED,WHITE);	  //ASC 6X12点阵
		Display_Asc_String(0,50,16, "ASCII_8x16",RED,WHITE);	  //ASC 8X16点阵
		Display_Asc_String(0,70,24, "ASCII_12x24",RED,WHITE);   //ASC 12X24点阵
		Display_Asc_String(0,100,32, "ASCII_16x32",RED,WHITE);	//ASC 16X32点阵  
		Display_GB2312_String(0,145,12, "中景园１２ｘ１２",RED,WHITE); //12x12汉字
		Display_GB2312_String(0,160,16, "中景园１６ｘ１６",RED,WHITE); //15x16汉字
		Display_GB2312_String(0,179,24, "中景园２４ｘ２４",RED,WHITE); //24x24汉字
		Display_GB2312_String(0,204,32, "中景园３２ｘ３",RED,WHITE);   //32x32汉字
		delay_ms(1000);
		LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
		
		Display_TimesNewRoman_String(0,15,12, "ASCII_8x12",RED,WHITE);   //ASC 8x12点阵(TimesNewRoman类型)
		Display_TimesNewRoman_String(0,30,16, "ASCII_12x16",RED,WHITE);  //ASC 12x16点阵(TimesNewRoman类型)
		Display_TimesNewRoman_String(0,50,24, "ASCII_16x24",RED,WHITE);	//ASC 16x24点阵(TimesNewRoman类型)
		Display_TimesNewRoman_String(0,80,32, "ASCII_24x",RED,WHITE);	  //ASC 24x32点阵(TimesNewRoman类型)
		Display_Arial_String(0,120,12, "ASCII_8x12",RED,WHITE);    //ASC 8x12点阵(Arial类型)
		Display_Arial_String(0,140,16, "ASCII_12x16",RED,WHITE);   //ASC 12x16点阵(Arial类型)
		Display_Arial_String(0,160,24, "ASCII_16x24",RED,WHITE);	 //ASC 16x24点阵(Arial类型)
		Display_Arial_String(0,190,32, "ASCII_24x",RED,WHITE);	   //ASC 24x32点阵(Arial类型)
		delay_ms(1000);
		LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	}
}
