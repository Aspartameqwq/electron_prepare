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
#include "bsp_ws2812.h"

int main(void)
{
    int i;
    int while_count = 1;
    uint32_t buff[4] = {WS2812_RED, WS2812_GREEN, WS2812_BLUE, WS2812_WHITE};

    SYSCFG_DL_init();

    lc_printf("\nWS2812 Demo Start.....\r\n");

    for (i = 0; i < 4; i++)
    {
        WS2812_Set_Color(0, buff[i]);
        WS2812_Set_Color(1, buff[i]);
        WS2812_Set_Color(2, buff[i]);
        WS2812_Set_Color(3, buff[i]);
        WS2812_Set_Color(4, buff[i]);
        WS2812_Set_Color(5, buff[i]);
        WS2812_Set_Color(6, buff[i]);
        WS2812_Set_Color(7, buff[i]);

        WS2812_Send_Array(); // 立即发送更新
        delay_ms(1000);
    }
    delay_ms(1000);

    i = 0;

    while(1)
    {

        WS2812_Set_Color((i + 0) % 8, buff[0]);
        WS2812_Set_Color((i + 1) % 8, buff[1]);
        WS2812_Set_Color((i + 2) % 8, buff[2]);
        WS2812_Set_Color((i + 3) % 8, buff[3]);
        WS2812_Send_Array(); // 发送更新

        // 清除其他LED
        WS2812_Set_Color((i + 4) % 8, WS2812_BLACK);
        WS2812_Set_Color((i + 5) % 8, WS2812_BLACK);
        WS2812_Set_Color((i + 6) % 8, WS2812_BLACK);
        WS2812_Set_Color((i + 7) % 8, WS2812_BLACK);
        WS2812_Send_Array(); // 发送更新

        i++;
		while_count++;
        delay_ms(200);

        // 清除
        if(while_count >= 100)
        {
            while_count = 1;
            i = 0;
        }
    }
}
