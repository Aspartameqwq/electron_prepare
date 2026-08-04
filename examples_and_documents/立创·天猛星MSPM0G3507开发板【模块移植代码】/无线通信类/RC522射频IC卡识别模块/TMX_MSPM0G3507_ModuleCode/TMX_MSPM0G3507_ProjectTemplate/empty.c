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
#include "string.h"
#include "bsp_rc522.h"

/* 卡的ID存储，32位,4字节 */
u8 ucArray_ID [ 4 ];
uint8_t ucStatusReturn;    //返回状态

int main(void)
{
    SYSCFG_DL_init();

    int i = 0;

    uint8_t read_write_data[16]={0};//读写数据缓存
    uint8_t card_KEY[6] ={0xff,0xff,0xff,0xff,0xff,0xff};//默认密码

    lc_printf ("\nRC522 IC Init....\r\n");
    RC522_Rese( );//复位RC522
    lc_printf ("RC522 Demo Start!\r\n");

    while(1)
    {

        /* 寻卡（方式：范围内全部），第一次寻卡失败后再进行一次，寻卡成功时卡片序列传入数组ucArray_ID中 */
        if ( ( ucStatusReturn = PcdRequest ( PICC_REQALL, ucArray_ID ) ) != MI_OK )
        {
            ucStatusReturn = PcdRequest ( PICC_REQALL, ucArray_ID );
        }
        if ( ucStatusReturn == MI_OK  )
        {
            /* 防冲突操作，被选中的卡片序列传入数组ucArray_ID中 */
            if ( PcdAnticoll ( ucArray_ID ) == MI_OK )
            {
                //输出卡ID
                lc_printf("ID: %X %X %X %X\r\n", ucArray_ID [ 0 ], ucArray_ID [ 1 ], ucArray_ID [ 2 ], ucArray_ID [ 3 ]);

                 //选卡
                if( PcdSelect(ucArray_ID) != MI_OK )
                {    lc_printf("PcdSelect failure\r\n");         }

                //校验卡片密码
                //数据块6的密码A进行校验（所有密码默认为16个字节的0xff）
                if( PcdAuthState(PICC_AUTHENT1B, 6, card_KEY,  ucArray_ID) != MI_OK )
                {    lc_printf("PcdAuthState failure\r\n");      }

                //往数据块4写入数据read_write_data
                read_write_data[0] = 0xaa;//将read_write_data的第一位数据改为0xaa
                if( PcdWrite(4,read_write_data) != MI_OK )
                {    lc_printf("PcdWrite failure\r\n");          }

                //将read_write_data的16位数据，填充为0（清除数据的意思）
                memset(read_write_data,0,16);
                delay_us(8);

            }
        }
        //读取数据块4的数据
        if( PcdRead(4,read_write_data) != MI_OK )
        {    lc_printf("PcdRead failure\r\n");           }

        //输出读出的数据
        for( i = 0; i < 16; i++ )
        {
                lc_printf("%x ",read_write_data[i]);
        }
        lc_printf("\r\n\n");

        delay_ms(500);
    }
}
