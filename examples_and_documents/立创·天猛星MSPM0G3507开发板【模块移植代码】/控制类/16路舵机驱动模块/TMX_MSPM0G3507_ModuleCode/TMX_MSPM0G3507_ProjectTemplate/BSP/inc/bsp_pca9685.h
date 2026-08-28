/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef __BSP_PCA9685_H__
#define __BSP_PCA9685_H__

#include "board.h"
//SDA输入模式
#define SDA_IN()   {  DL_GPIO_initDigitalInput(IIC_Software_SDA_IOMUX); }
//SDA输出模式
#define SDA_OUT()  {  DL_GPIO_initDigitalOutput(IIC_Software_SDA_IOMUX); \
                      DL_GPIO_enableOutput(IIC_Software_PORT, IIC_Software_SDA_PIN); \
                   }

#define SCL(BIT)  ( BIT ? DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_SCL_PIN) : DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_SCL_PIN) )
#define SDA(BIT)  ( BIT ? DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_SDA_PIN) : DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_SDA_PIN) )
#define SDA_GET() ( ( DL_GPIO_readPins( IIC_Software_PORT, IIC_Software_SDA_PIN ) & IIC_Software_SDA_PIN ) ? 1 : 0 )

#define PCA_Addr              0x80        //IIC地址
#define PCA_Model             0x00
#define LED0_ON_L             0x06
#define LED0_ON_H             0x07
#define LED0_OFF_L            0x08
#define LED0_OFF_H            0x09
#define PCA_Pre               0xFE        //配置频率地址

void PCA9685_Init(float hz,uint8_t angle);
void setAngle(uint8_t num,uint8_t angle);
void PCA9685_setFreq(float freq);
void PCA9685_setPWM(uint8_t num,uint32_t on,uint32_t off);

#endif