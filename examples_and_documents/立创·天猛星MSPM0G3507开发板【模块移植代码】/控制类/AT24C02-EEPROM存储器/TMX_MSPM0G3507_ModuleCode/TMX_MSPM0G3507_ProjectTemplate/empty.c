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
#include "bsp_at24c02.h"


int main(void)
{
    unsigned char dat1 = 0;
    unsigned char dat2 = 0;

    SYSCFG_DL_init();

    lc_printf("\nAT24C02 Demo Start.....\r\n");

    //向0地址写入数据48
    AT24C02_WriteByte(0,48);
    delay_ms(5);

    //向8地址写入数据48
    AT24C02_WriteByte(8,66);
    delay_ms(5);

    //从0地址读取数据到dat1
    dat1 = AT24C02_ReadByte(0);
    delay_ms(5);
    //从8地址读取数据到dat2
    dat2 = AT24C02_ReadByte(8);
    delay_ms(5);

    delay_ms(50);

    //输出dat查看数据是否正确
    lc_printf("dat1 = %d\r\n",dat1);
    delay_ms(1);
    lc_printf("dat2 = %d\r\n",dat2);
    delay_ms(1);

    while(1)
    {

    }
}
