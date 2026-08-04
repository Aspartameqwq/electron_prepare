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
#include "bsp_DotMatrix.h"

unsigned char disp1[12][8]={//一共12行

{0x3C,0x42,0x42,0x42,0x42,0x42,0x66,0x38},/*"0",0*/

{0x38,0x08,0x08,0x08,0x08,0x08,0x08,0x18},/*"1",1*/

{0x7C,0x42,0x02,0x04,0x08,0x30,0x42,0x7E},/*"2",2*/

{0x7C,0x46,0x04,0x18,0x06,0x02,0x42,0x3C},/*"3",3*/

{0x0C,0x14,0x14,0x24,0x44,0x3C,0x04,0x0C},/*"4",4*/

{0x3E,0x40,0x58,0x64,0x02,0x02,0x42,0x3C},/*"5",5*/

{0x3E,0x40,0x48,0x76,0x42,0x42,0x42,0x3C},/*"6",6*/

{0x7E,0x04,0x04,0x08,0x10,0x10,0x10,0x10},/*"7",7*/

{0x7C,0x42,0x62,0x3C,0x44,0x42,0x42,0x3C},/*"8",8*/

{0x7C,0x42,0x42,0x46,0x3A,0x02,0x44,0x38},/*"9",9*/

{0x3A,0x04,0x04,0x24,0x24,0x18,0x08,0x7E},/*"立",10*/

{0x52,0x4E,0x76,0xD6,0x56,0x76,0x4A,0x4E},/*"创",11*/
};

int main(void)
{
    SYSCFG_DL_init();

    DotMatrix_Init();
    Write_DotMatrix_AllOff();
    printf("DotMatrix demo start\r\n");

    while(1)
    {
        //第一个点阵显示disp1数组的第10行内容
        //第二个点阵显示disp1数组的第11行内容
        //第三个点阵显示disp1数组的第7行内容
        //第四个点阵显示disp1数组的第6行内容
        DotMatrix_display(disp1[10],disp1[11], disp1[7], disp1[6]);
        delay_ms(1000);
    }
}
