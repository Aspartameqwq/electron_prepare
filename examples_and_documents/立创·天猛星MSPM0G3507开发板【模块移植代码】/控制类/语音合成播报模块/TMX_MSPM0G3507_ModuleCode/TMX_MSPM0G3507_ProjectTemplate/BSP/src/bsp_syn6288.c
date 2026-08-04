/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_syn6288.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#define SYN6288RX_LEN_MAX 128

unsigned char SYN6288RX_BUFF[SYN6288RX_LEN_MAX];
unsigned char SYN6288RX_LEN = 0;

/******************************************************************
 * 函 数 名 称：SYN6288_Init
 * 函 数 说 明：SYN6288初始化
 * 函 数 形 参：
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：默认波特率为9600
******************************************************************/
void SYN6288_Init(void)
{
    //清除串口中断标志
    NVIC_ClearPendingIRQ(UART_2_INST_INT_IRQN);
    //使能串口中断
    NVIC_EnableIRQ(UART_2_INST_INT_IRQN);

    SYN6288_Send_Cmd(0x31, 0,(uint8_t *)"\n");
}



/******************************************************************
 * 函 数 名 称：SYN6288_Send_Bit
 * 函 数 说 明：向SYN6288发送单个字符
 * 函 数 形 参：ch发送的字符
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void SYN6288_Send_Bit(unsigned char ch)
{
    //当串口1忙的时候等待，不忙的时候再发送传进来的字符
    while( DL_UART_isBusy(UART_2_INST) == true );
    //发送单个字符
    DL_UART_Main_transmitData(UART_2_INST, ch);
}

/******************************************************************
 * 函 数 名 称：SYN6288_send_String
 * 函 数 说 明：SYN6288发送字符串
 * 函 数 形 参：str要发送的字符串
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void SYN6288_send_String(unsigned char *str)
{
        while( str && *str ) // 地址为空或者值为空跳出
        {
                SYN6288_Send_Bit(*str++);
        }
}
//获取串口接收的数据
unsigned char *Get_SYN6288RX_BUFF(void)
{
    return SYN6288RX_BUFF;
}

//清除串口接收的数据
void Clear_SYN6288RX_BUFF(void)
{
    unsigned char i = SYN6288RX_LEN_MAX-1;
    while(i)
    {
        SYN6288RX_BUFF[i--] = '\0';
    }
    SYN6288RX_LEN = 0;
}

/************************************************************
 * 函数名称：SYN6288_Send_Cmd
 * 函数说明：向SYN6288发送命令
 * 型    参：
 * 【CmdType=命令字】           可使用参数有：
 *                                              0x01        语音合成命令
 *                                              0x31        设置波特率（默认9600）
 *                                              0x02        停止合成命令
 *                                              0x03        暂停合成命令
 *                                              0x04        恢复合成命令
 *                                              0x21        芯片状态查询命令
 *                                              0x88        芯片进入低功耗模式
 * 【CmdPar=命令参数】          可使用参数有：
 *                                              字节高5位的十进制为0时，表示不加背景音乐
 *                                              字节高5位的十进制为1~15时，表示所选背景音乐的编号
 *                                              字节低3位的十进制为0~3，并且命令字为语音合成命令时，分别代表设置文本为GB2312格式、GBK格式、BIG5格式、UNICODE格式；
 *                                              字节低3位的十进制为0~2，并且命令字为设置波特率时，分别代表设置波特率为9600、19200、38400；
 * 【text=播报的文本】

 * 返 回 值：0=发送成功
 * 备    注：

 * 接收到控制命令帧，芯片会向上位机发送1个字节的状态回传，上位机可根据这个回传来判断芯片目前的工作状态
 * 初始化成功回传            0X4A
 * 收到正确的命令帧回传      0x41
 * 收到不能识别命令帧回传    0x45
 * 芯片播音状态回传          0x4E
 * 芯片空闲状态回传          0x4F
*************************************************************/
unsigned char SYN6288_Send_Cmd(uint8_t CmdType, uint8_t CmdPar, uint8_t *text)
{
        unsigned char frame_header = 0XFD;       //帧头
        unsigned int Text_Len = strlen((const char*)text);//待发送文本的长度
        unsigned int Data_Len = Text_Len + 3;              //数据区长度；3=帧头、帧尾和异或校验
        unsigned char Xor_Check = 0;                       //异或校验存储
        unsigned char Send_Buff[210];                     //待发送的命令帧，命令帧最大206个字节
        uint8_t i = 0;

        Send_Buff[0] = frame_header;   //帧头
        Send_Buff[1] = Data_Len>>8;      //高位在前
        Send_Buff[2] = Data_Len&0x00ff; //低位在前
        Send_Buff[3] = CmdType;         //命令字
        Send_Buff[4] = CmdPar;          //命令数据
        sprintf((char*)Send_Buff+5, "%s", text );

        //发送数据
        for( i = 0; i < Text_Len+5; i++ )
        {
                Xor_Check = Xor_Check ^ Send_Buff[i];//对每一个数据进行异或校验保存
                SYN6288_Send_Bit( Send_Buff[i] );//发送数据
        }
        SYN6288_Send_Bit( Xor_Check );//发送最后一位：异或校验数据

        return 0;
}

//串口的中断服务函数
void UART_2_INST_IRQHandler(void)
{
    //如果产生了串口中断
    switch( DL_UART_getPendingInterrupt(UART_2_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断

            SYN6288RX_BUFF[ SYN6288RX_LEN ] = DL_UART_Main_receiveData(UART_2_INST); // 接收数据

            SYN6288RX_LEN = ( SYN6288RX_LEN + 1 ) % SYN6288RX_LEN_MAX;

            SYN6288RX_BUFF[SYN6288RX_LEN] = '\0';

            break;

        default://其他的串口中断
            break;
    }
}