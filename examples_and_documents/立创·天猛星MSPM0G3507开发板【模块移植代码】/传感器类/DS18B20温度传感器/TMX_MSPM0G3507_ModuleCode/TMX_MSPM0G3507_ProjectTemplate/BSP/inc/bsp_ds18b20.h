/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef _BSP_DS18B20_H_
#define _BSP_DS18B20_H_

#include "board.h"
//设置DQ输出模式
#define DQ_OUT()  {      \
                        DL_GPIO_initDigitalOutput(DS18B20_DQ_IOMUX);  \
                        DL_GPIO_setPins(DS18B20_PORT, DS18B20_DQ_PIN);  \
                        DL_GPIO_enableOutput(DS18B20_PORT, DS18B20_DQ_PIN); \
                        DL_GPIO_setPins(DS18B20_PORT, DS18B20_DQ_PIN); \
                   }
//设置DQ输入模式
#define DQ_IN()   {  DL_GPIO_initDigitalInput(DS18B20_DQ_IOMUX); }
//获取DQ引脚的电平变化
#define DQ_GET()   ( ( DL_GPIO_readPins( DS18B20_PORT, DS18B20_DQ_PIN ) & DS18B20_DQ_PIN ) ? 1 : 0 )
//DQ输出
#define DQ(x)      ( (x) ? (DL_GPIO_setPins(DS18B20_PORT,DS18B20_DQ_PIN)) : (DL_GPIO_clearPins(DS18B20_PORT,DS18B20_DQ_PIN)) )

void DS18B20_Reset(void);
uint8_t DS18B20_Check(void);
char DS18B20_Init(void);
float DS18B20_GetTemperture(void);
#endif