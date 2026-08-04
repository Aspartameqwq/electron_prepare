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
#include "stdio.h"
#include "lcd_init.h"
#include "lcd.h"
#include "pic.h"


int main(void)
{
    SYSCFG_DL_init();

    // 串口输出测试
    lc_printf("Hello %s\r\n","World");

    // 日志输出测试
    LOG_D("Log Debug Test....%s", "LCKFB");

	float t = 0;

	LCD_Init(); // LCD初始化
    
	LCD_Fill(0, 0, LCD_W,LCD_H,WHITE);

	while(1)
	{
		LCD_ShowChinese(40,0,(const char *)"中景园电子",RED,WHITE,16,0);
		LCD_ShowString(10,20,(const u8 *)"LCD_W:",RED,WHITE,16,0);
		LCD_ShowIntNum(58,20,LCD_W,3,RED,WHITE,16);
		LCD_ShowString(10,40,(const u8 *)"LCD_H:",RED,WHITE,16,0);
		LCD_ShowIntNum(58,40,LCD_H,3,RED,WHITE,16);
		LCD_ShowFloatNum1(10,60,t,4,RED,WHITE,16);
		t+=0.11;
		LCD_ShowPicture(100,20,40,40,gImage_1);
	}
}
