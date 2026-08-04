/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef _BSP_AS608_H_
#define _BSP_AS608_H_

#include "board.h"

#define TOUCH_IN    ( ( DL_GPIO_readPins( GPIO_PORT, GPIO_TOUCH_PIN ) & GPIO_TOUCH_PIN ) ? 1 : 0 )


/* 串口缓冲区的数据长度 */
#define USART1_RECEIVE_LENGTH  64

extern uint8_t  u2_recv_buff[USART1_RECEIVE_LENGTH]; // 接收缓冲区
extern uint16_t u2_recv_length;                     // 接收数据长度
extern uint8_t  u2_recv_flag;                       // 接收完成标志位

void as608_config();
char get_as608_touch(void);
void uart1_receive_clear(void);

char Device_Check(void);
void FPM10A_Add_Fingerprint(void);//添加指纹
unsigned int FPM10A_Find_Fingerprint(void);//查找指纹
void FPM10A_Delete_All_Fingerprint(void);
#endif