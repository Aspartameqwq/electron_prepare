
/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_infrared.h"
#include "board.h"
#include "stdio.h"

unsigned char infrared_recv_buff[USART1_RECEIVE_LENGTH];//串口接收缓存
uint16_t infrared_recv_length;//串口接收长度
unsigned char infrared_recv_flag;//串口接收完毕标志 1=接收完毕 0=未接收完毕

unsigned char device_addr       = 0XA1;//默认器件地址
unsigned char Infrared_emission = 0XF1;//红外发射状态
unsigned char modified_addr     = 0XF2;//修改设备地址
unsigned char modified_baud     = 0XF3;//修改波特率

/******************************************************************
 * 函 数 名 称：Infrared_Init
 * 函 数 说 明：初始化万能红外引脚
 * 函 数 形 参：设置波特率
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：万能红外默认波特率为9600
******************************************************************/
void Infrared_Init(void)
{
        //清除串口中断标志
        NVIC_ClearPendingIRQ(UART_2_INST_INT_IRQN);
        //使能串口中断
        NVIC_EnableIRQ(UART_2_INST_INT_IRQN);
}

/************************************************
函数名称 ： infrared_send_byte
功    能 ： 串口发送一个字节
参    数 ： ucch：要发送的字节
返 回 值 ：
作    者 ： LCKFB
*************************************************/
void infrared_send_byte(uint8_t ucch)
{
    //当串口1忙的时候等待，不忙的时候再发送传进来的字符
    while( DL_UART_isBusy(UART_2_INST) == true );
    //发送单个字符
    DL_UART_Main_transmitData(UART_2_INST, ucch);
}

void infrared_send_hex(uint8_t *ch, int len)
{
    while(len--)
    {
        infrared_send_byte(*ch++);
    }
}
/************************************************
函数名称 ： infrared_receive_clear
功    能 ： 清除串口接收的全部数据
参    数 ： 无
返 回 值 ： 无
作    者 ： LCKFB
*************************************************/
void infrared_receive_clear(void)
{
    unsigned int i = 0;

    for( i = 0; i < USART1_RECEIVE_LENGTH; i++ )
    {
        infrared_recv_buff[ i ] = 0;
    }

    infrared_recv_length = 0;
    infrared_recv_flag = 0;
}

/******************************************************************
 * 函 数 名 称：Infrared_emission_cmd
 * 函 数 说 明：控制模块发射红外命令
 * 函 数 形 参：Infrared_buff要发射的红外信号  len红外信号长度
 * 函 数 返 回：0：  超时未接收到发射成功数据
 *              1：  发射成功
 *              2：  接收的数据不是发射成功数据
 *              100：发射的数据不是3位

 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
char Infrared_emission_cmd(unsigned char* Infrared_buff, char len)
{
    unsigned char send_data[5] = {0};//必须赋初值
    unsigned int  time_out = 1000; //超时时间，单位MS

    //如果要发送的数据长度不对
    if( (len < 3) || (len > 3) )
        return 100;

    send_data[0] = device_addr;         //设备地址
    send_data[1] = Infrared_emission;   //操作位
    send_data[2] = Infrared_buff[0];          //数据位1
    send_data[3] = Infrared_buff[1];          //数据位2
    send_data[4] = Infrared_buff[2];          //数据位3

    infrared_receive_clear();//先清除接收的数据
    infrared_send_hex(send_data, 5);//发送数据
    //等待回应数据
    while( infrared_recv_flag != 1 && time_out > 0 )
    {
        time_out--;
        delay_ms(1);
    }
    if( time_out > 0 )//没有超时
    {
        infrared_recv_flag = 0;
        //如果接收到通信地址修改成功的回应数据
        if( infrared_recv_buff[0] == 0XF1 ) return 1;
        else return 2;
    }
    return 0;
}

/******************************************************************
 * 函 数 名 称：modified_addr_cmd
 * 函 数 说 明：修改串口地址命令
 * 函 数 形 参：addr_value 要修改的串口地址
 * 函 数 返 回：0：  超时未接收到修改成功数据
 *              1：  修改成功
 *              2：  接收的数据不是修改成功数据
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
char modified_addr_cmd(unsigned int addr_value)
{
    unsigned char send_data[5] = {0};//必须赋初值
    unsigned int  time_out = 1000; //超时时间，单位MS

    send_data[0] = device_addr;     //设备地址
    send_data[1] = modified_addr;   //操作位
    send_data[2] = addr_value;      //数据位

    infrared_receive_clear();//先清除接收的数据
    infrared_send_hex(send_data, 5);//发送数据
    //等待回应数据
    while( infrared_recv_flag != 1 && time_out > 0 )
    {
        time_out--;
        delay_ms(1);
    }
    if( time_out > 0 )//没有超时
    {
        infrared_recv_flag = 0;
        //如果接收到通信地址修改成功的回应数据
        if( infrared_recv_buff[0] == 0XF2 ) return 1;
        else return 2;
    }
    return 0;
}

/******************************************************************
 * 函 数 名 称：modified_baud_cmd
 * 函 数 说 明：修改波特率命令
 * 函 数 形 参：baud_value 要修改的波特率，可以输入的值有：
 *              4800、9600、19200、57600
 * 函 数 返 回：0：  超时未接收到修改成功数据
 *              1：  修改成功
 *              2：  接收的数据不是修改成功数据
 * 作       者：LCKFB
 * 备       注：
******************************************************************/
char modified_baud_cmd(unsigned int baud_value)
{
    unsigned char send_data[5] = {0};//必须赋初值
    unsigned int  time_out = 1000; //超时时间，单位MS

    send_data[0] = device_addr;     //设备地址
    send_data[1] = modified_baud;   //操作位

    switch(baud_value)//要修改的波特率值
    {
        case 4800:   send_data[2] = 0X01; break;
        case 9600:   send_data[2] = 0X02; break;
        case 19200:  send_data[2] = 0X03; break;
        case 57600:  send_data[2] = 0X04; break;
    }
    infrared_receive_clear();//先清除接收的数据
    infrared_send_hex(send_data, 5);//发送数据
    //等待回应数据
    while( infrared_recv_flag != 1 && time_out > 0 )
    {
        time_out--;
        delay_ms(1);
    }
    if( time_out > 0 )//没有超时
    {
        infrared_recv_flag = 0;
        //如果接收到波特率设置成功的回应数据
        if( infrared_recv_buff[0] == 0XF3 ) return 1;
        else return 2;
    }
    return 0;
}

/******************************************************************
 * 函 数 名 称：Recv_DataShow
 * 函 数 说 明：显示接收到的数据
 * 函 数 形 参：无
 * 函 数 返 回： 0：  未有数据
 *              1：  有数据
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
uint8_t Recv_DataShow(void)
{
    if(infrared_recv_flag == 1) // 表示是否有数据接收
    {
        uint8_t *current = infrared_recv_buff; // 创建一个指针来遍历缓冲区
        while(infrared_recv_length-- != 0) // 检查当前位置的值是否为0
        {
            lc_printf("%02x ", *current); // 打印当前字节
            current++; // 移动到下一个字节
        }
        lc_printf("\n"); // 打印换行

        infrared_receive_clear(); // 清除接收到的数据
        return 1; // 表示有数据被打印
    }

    return 0; // 表示没有数据被打印
}

//串口的中断服务函数
void UART_2_INST_IRQHandler(void)
{
    //如果产生了串口中断
    switch( DL_UART_getPendingInterrupt(UART_2_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断
            //接发送过来的数据保存在变量中
            infrared_recv_buff[infrared_recv_length++] = DL_UART_Main_receiveData(UART_2_INST);

            infrared_recv_buff[infrared_recv_length] = '\0';
            infrared_recv_flag = 1;

            break;

        default://其他的串口中断
            break;
    }
}