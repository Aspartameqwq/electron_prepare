/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef _BSP_ADS1115_H_
#define _BSP_ADS1115_H_

#include "board.h"


//SDA输入模式
#define SDA_IN()   {  DL_GPIO_initDigitalInput(ADS1115_SDA_IOMUX); }
//SDA输出模式
#define SDA_OUT()  {  DL_GPIO_initDigitalOutput(ADS1115_SDA_IOMUX); \
                      DL_GPIO_enableOutput(ADS1115_PORT, ADS1115_SDA_PIN); \
                   }

#define SCL(BIT)  ( BIT ? DL_GPIO_setPins(ADS1115_PORT,ADS1115_SCL_PIN) : DL_GPIO_clearPins(ADS1115_PORT,ADS1115_SCL_PIN) )
#define SDA(BIT)  ( BIT ? DL_GPIO_setPins(ADS1115_PORT,ADS1115_SDA_PIN) : DL_GPIO_clearPins(ADS1115_PORT,ADS1115_SDA_PIN) )
#define GETSDA()  ( ( DL_GPIO_readPins( ADS1115_PORT, ADS1115_SDA_PIN ) & ADS1115_SDA_PIN ) ? 1 : 0 )


void ADS1115_Init(void);
unsigned char WriteADS1115(unsigned char add,unsigned char dat_H,unsigned char dat_L);
float ReadADS1115(unsigned char add);
#endif