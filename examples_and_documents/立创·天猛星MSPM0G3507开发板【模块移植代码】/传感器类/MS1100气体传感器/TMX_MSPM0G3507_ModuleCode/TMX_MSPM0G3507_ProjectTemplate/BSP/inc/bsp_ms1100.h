/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef __BSP_MS1100_H__
#define __BSP_MS1100_H__

#include "board.h"

 //采样次数
#define SAMPLES    30

#define MS1100_DO   ( ( DL_GPIO_readPins( GPIO_PORT, GPIO_DO_PIN ) & GPIO_DO_PIN ) ? 1 : 0 )

uint32_t Get_MS1100_Value(void);//读取AO值
uint8_t Get_DO_Num(void);//读取DO值

#endif
