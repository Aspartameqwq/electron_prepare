/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef __BSP_SHT30_H__
#define __BSP_SHT30_H__

#include "board.h"

extern double Temperature, Humidity;

//SDA输入模式
#define SDA_IN()   {  DL_GPIO_initDigitalInput(IIC_Software_SDA_IOMUX); }
//SDA输出模式
#define SDA_OUT()  {  DL_GPIO_initDigitalOutput(IIC_Software_SDA_IOMUX); \
                      DL_GPIO_enableOutput(IIC_Software_PORT, IIC_Software_SDA_PIN); \
                   }

#define SCL(BIT)  ( BIT ? DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_SCL_PIN) : DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_SCL_PIN) )
#define SDA(BIT)  ( BIT ? DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_SDA_PIN) : DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_SDA_PIN) )
#define SDA_GET() ( ( DL_GPIO_readPins( IIC_Software_PORT, IIC_Software_SDA_PIN ) & IIC_Software_SDA_PIN ) ? 1 : 0 )



char SHT30_Read(uint16_t dat);

#endif