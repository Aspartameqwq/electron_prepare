/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef _BSP_MAX7219_H_
#define _BSP_MAX7219_H_

#include "board.h"

#define MAX7219_CLK(X) ( X ? DL_GPIO_setPins(MAX7219_PORT,MAX7219_CLK_PIN) : DL_GPIO_clearPins(MAX7219_PORT,MAX7219_CLK_PIN) )
#define MAX7219_DIN(X) ( X ? DL_GPIO_setPins(MAX7219_PORT,MAX7219_DIN_PIN) : DL_GPIO_clearPins(MAX7219_PORT,MAX7219_DIN_PIN) )
#define MAX7219_CS(X)  ( X ? DL_GPIO_setPins(MAX7219_PORT,MAX7219_CS_PIN) : DL_GPIO_clearPins(MAX7219_PORT,MAX7219_CS_PIN) )


void Write_Max7219(uint8_t address,uint8_t dat);
void Write_Max7219_2(uint8_t address,uint8_t dat);
void Write_Max7219_AllOff(void);
void MAX7219_Init(void);

#endif