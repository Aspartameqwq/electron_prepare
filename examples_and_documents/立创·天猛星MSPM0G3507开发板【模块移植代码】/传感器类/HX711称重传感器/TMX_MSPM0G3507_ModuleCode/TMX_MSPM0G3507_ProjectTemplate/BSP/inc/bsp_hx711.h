/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef _BSP_HX711_H_
#define _BSP_HX711_H_

#include "board.h"

//设置SDA输出模式
#define DT_OUT()   {\
                        DL_GPIO_initDigitalOutput(IIC_Software_DT_IOMUX);\
                        DL_GPIO_setPins(IIC_Software_PORT, IIC_Software_DT_PIN);\
                        DL_GPIO_enableOutput(IIC_Software_PORT, IIC_Software_DT_PIN);\
                    }
//设置SDA输入模式
#define DT_IN()    { DL_GPIO_initDigitalInput(IIC_Software_DT_IOMUX); }
//获取DT引脚的电平变化
#define DT_GET()    ( ( ( DL_GPIO_readPins(IIC_Software_PORT,IIC_Software_DT_PIN) & IIC_Software_DT_PIN ) > 0 ) ? 1 : 0 )
//DT与SCK输出
#define DT(x)       ( (x) ? (DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_DT_PIN)) : (DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_DT_PIN)) )
#define SCK(x)      ( (x) ? (DL_GPIO_setPins(IIC_Software_PORT,IIC_Software_SCK_PIN)) : (DL_GPIO_clearPins(IIC_Software_PORT,IIC_Software_SCK_PIN)) )

void HX711_Get_InitValue(void);
float HX711_Get_Weight(void);

#endif
