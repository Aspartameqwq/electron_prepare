/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef __BSP_JQ8900_H__
#define __BSP_JQ8900_H__

#include "board.h"

//是否开启串口调试，查看WIFI回显数据
#define     DEBUG   1

#define JQ8900_RX_LEN_MAX        250 //串口接收最大长度

#define SET_JQ8900_APP(X)        ( (X) ? (DL_GPIO_setPins(GPIO_PORT,GPIO_VPP_PIN)) : (DL_GPIO_clearPins(GPIO_PORT,GPIO_VPP_PIN)) )

void JQ8900_Init(void);
void JQ8900_USART_send_String(unsigned char *str, unsigned int len);
void SendData ( unsigned char  addr );
void JQ8900_USART_Send_Bit(unsigned char ch);
#endif
