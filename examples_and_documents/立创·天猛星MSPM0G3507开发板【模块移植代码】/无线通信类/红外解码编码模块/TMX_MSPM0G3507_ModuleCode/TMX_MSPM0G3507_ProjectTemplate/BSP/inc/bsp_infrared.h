
/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef _BSP_infrared_H_
#define _BSP_infrared_H_

#include "board.h"

#define USART1_RECEIVE_LENGTH 1024  //串口最大接收长度

extern unsigned char infrared_recv_buff[USART1_RECEIVE_LENGTH];//串口接收缓存
extern uint16_t infrared_recv_length;//串口接收长度
extern unsigned char infrared_recv_flag;//串口接收完毕标志 1=接收完毕 0=未接收完毕

void Infrared_Init(void);//初始化万能红外引脚
void  infrared_receive_clear(void);//清除
uint8_t Recv_DataShow(void); // 回显接收到的数据
char Infrared_emission_cmd(unsigned char* Infrared_buff, char len);//红外发射命令
char modified_addr_cmd(unsigned int addr_value);//修改串口地址命令
char modified_baud_cmd(unsigned int baud_value);//修改波特率命令

#endif