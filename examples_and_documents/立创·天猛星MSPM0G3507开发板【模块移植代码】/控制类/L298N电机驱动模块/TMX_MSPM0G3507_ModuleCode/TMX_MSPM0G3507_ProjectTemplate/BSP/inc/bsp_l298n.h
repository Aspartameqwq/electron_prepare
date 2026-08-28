/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef _BSP_L298N_H
#define _BSP_L298N_H

#include "board.h"

void AB_Motor_Stop(void);
void A_Motor_Control(uint8_t dir, uint32_t speed);
void B_Motor_Control(uint8_t dir, uint32_t speed);

#endif  /* BSP_L298N_H */
