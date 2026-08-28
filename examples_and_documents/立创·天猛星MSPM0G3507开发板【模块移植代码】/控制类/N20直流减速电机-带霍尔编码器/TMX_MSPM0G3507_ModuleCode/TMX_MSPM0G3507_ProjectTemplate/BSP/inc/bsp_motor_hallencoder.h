/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef	__BSP_MOTOR_HALLENCODER_H__
#define __BSP_MOTOR_HALLENCODER_H__

#include "board.h"

// 获得绝对值
#define ABS(a)      (a>0 ? a:(-a))

typedef struct{
    int Should_Get_Encoder_Count;   // 将要获得的编码器计数
    int Obtained_Get_Encoder_Count; // 得到的编码器的计数
}Encoder;


void Motor_Init(void);
int Motor_Get_Encoder(int dir);
void Motor_Set_PWM(int pwma,int pwmb);
void Motor_Stop(void);

#endif
