/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_jq8900.h"
#include "stdio.h"
#include "string.h"

unsigned char JQ8900_RX_BUFF[JQ8900_RX_LEN_MAX];
unsigned char JQ8900_RX_FLAG = 0;
unsigned char JQ8900_RX_LEN = 0;

/******************************************************************
 * 函 数 名 称：JQ8900_USART_Send_Bit
 * 函 数 说 明：向JQ8900模块发送单个字符
 * 函 数 形 参：ch=字符
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
void JQ8900_USART_Send_Bit(unsigned char ch)
{
    //当串口1忙的时候等待，不忙的时候再发送传进来的字符
    while( DL_UART_isBusy(UART_2_INST) == true );
    //发送单个字符
    DL_UART_Main_transmitData(UART_2_INST, ch);
}

/******************************************************************
 * 函 数 名 称：JQ8900_USART_send_String
 * 函 数 说 明：向JQ8900模块发送字符串
 * 函 数 形 参：str=发送的字符串
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
void JQ8900_USART_send_String(unsigned char *str, unsigned int len)
{
        while( len-- )
        {
                JQ8900_USART_Send_Bit(*str++);
        }
}
//清除串口接收的数据
/******************************************************************
 * 函 数 名 称：Clear_JQ8900_RX_BUFF
 * 函 数 说 明：清除JQ8900发过来的数据
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
void Clear_JQ8900_RX_BUFF(void)
{
    unsigned char i = JQ8900_RX_LEN_MAX-1;
    while(i)
    {
        JQ8900_RX_BUFF[i--] = 0;
    }
    JQ8900_RX_LEN = 0;
    JQ8900_RX_FLAG = 0;
}

/******************************************************************
 * 函 数 名 称：JQ8900_Init
 * 函 数 说 明：JQ8900模块初始化
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：JQ8900的默认波特率是9600
******************************************************************/
void JQ8900_Init(void)
{
    //清除串口中断标志
    NVIC_ClearPendingIRQ(UART_2_INST_INT_IRQN);
    //使能串口中断
    NVIC_EnableIRQ(UART_2_INST_INT_IRQN);
}

/**********************************************************
 * 函 数 名 称：SendData
 * 函 数 功 能：一线串行通信控制
 * 传 入 参 数：addr=发送的指令
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
**********************************************************/
void SendData ( unsigned char  addr )
{
    unsigned char i;
    SET_JQ8900_APP(1); /*开始拉高  */
    delay_us ( 500 );
    SET_JQ8900_APP(0);/*开始引导码*/
    delay_ms ( 4 );/*此处延时最少要大于2ms，官方建议4MS  */
    for ( i = 0; i < 8; i++ ) /*总共8位数据,从低位开始  */
    {
                //数据总是从1开始
        SET_JQ8900_APP(1);
        if ( addr & 0x01 )
        {
            delay_us ( 1300 );/*3:1表示数据位1,每个位用两个脉冲表示  */
           SET_JQ8900_APP(0);
            delay_us ( 500 );
        }
        else
        {
            delay_us ( 500 );/*1：3表示数据位0 ,每个位用两个脉冲表示  */
            SET_JQ8900_APP(0);
            delay_us ( 1300 );
        }
        addr >>= 1;
    }
    SET_JQ8900_APP(1);
        delay_ms(10);//两个字节之间延时建议在10ms以上
}

//串口的中断服务函数
void UART_2_INST_IRQHandler(void)
{
    //如果产生了串口中断
    switch( DL_UART_getPendingInterrupt(UART_2_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断

            JQ8900_RX_BUFF[ JQ8900_RX_LEN ] = DL_UART_Main_receiveData(UART_2_INST); // 接收数据

            #if DEBUG

                    //测试，查看接收到了什么数据
                    lc_printf("%c", JQ8900_RX_BUFF[ JQ8900_RX_LEN ]);

            #endif

            JQ8900_RX_LEN = ( JQ8900_RX_LEN + 1 ) % JQ8900_RX_LEN_MAX;

            JQ8900_RX_BUFF[JQ8900_RX_LEN] = '\0';
            JQ8900_RX_FLAG = 1;

            break;

        default://其他的串口中断
            break;
    }
}