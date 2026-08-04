/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef _BSP_TOUCHKEY_H_
#define _BSP_TOUCHKEY_H_

#include "board.h"

#define KEY_IN1   ( ( DL_GPIO_readPins( TTP224_PORT, TTP224_OUT1_PIN ) & TTP224_OUT1_PIN ) ? 1 : 0 )
#define KEY_IN2   ( ( DL_GPIO_readPins( TTP224_PORT, TTP224_OUT2_PIN ) & TTP224_OUT2_PIN ) ? 1 : 0 )
#define KEY_IN3   ( ( DL_GPIO_readPins( TTP224_PORT, TTP224_OUT3_PIN ) & TTP224_OUT3_PIN ) ? 1 : 0 )
#define KEY_IN4   ( ( DL_GPIO_readPins( TTP224_PORT, TTP224_OUT4_PIN ) & TTP224_OUT4_PIN ) ? 1 : 0 )


char Key_IN1_Scanf(void);//触摸按键1的输入状态
char Key_IN2_Scanf(void);//触摸按键2的输入状态
char Key_IN3_Scanf(void);//触摸按键3的输入状态
char Key_IN4_Scanf(void);//触摸按键4的输入状态

#endif