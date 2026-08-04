/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef __BSP_BH1750_H__
#define __BSP_BH1750_H__

#include "board.h"

//设置SDA输出模式
#define SDA_OUT()   {\
                        DL_GPIO_initDigitalOutput(IIC_Software_SDA_IOMUX);\
                        DL_GPIO_setPins(IIC_Software_PORT, IIC_Software_SDA_PIN);\
                        DL_GPIO_enableOutput(IIC_Software_PORT, IIC_Software_SDA_PIN);\
                    }
//设置SDA输入模式
#define SDA_IN()   {  DL_GPIO_initDigitalInput(IIC_Software_SDA_IOMUX); }

//获取SDA引脚的电平变化
#define SDA_GET() ( ( DL_GPIO_readPins( IIC_Software_PORT, IIC_Software_SDA_PIN ) & IIC_Software_SDA_PIN ) ? 1 : 0 )
//SDA与SCL输出
#define SDA(x)      ( (x) ? (DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_SDA_PIN)) : (DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_SDA_PIN)) )
#define SCL(x)      ( (x) ? (DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_SCL_PIN)) : (DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_SCL_PIN)) )



#define   SlaveAddress   0x46 //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
                              //ALT  ADDRESS引脚接地时地址为0x46，接电源时地址为0xB8

float Multiple_read_BH1750(void);
char Single_Write_BH1750(uint8_t REG_Address);
void BH1750_Init(void);

#endif