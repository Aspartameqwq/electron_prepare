/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#ifndef __BSP_DHT11_H__
#define __BSP_DHT11_H__

#include "board.h"

//设置DHT11输出高或低电平
#define DATA_GPIO_OUT(x)    ( (x) ? (DL_GPIO_setPins(DHT11_PORT,DHT11_DATA_PIN)) : (DL_GPIO_clearPins(DHT11_PORT,DHT11_DATA_PIN)) )
//获取DHT11数据引脚高低电平状态
#define DATA_GPIO_IN        DL_GPIO_readPins(DHT11_PORT, DHT11_DATA_PIN)

extern float temperature;
extern float humidity;

unsigned int DHT11_Read_Data(void);//读取模块数据
float Get_temperature(void);//返回读取模块后的温度数据
float Get_humidity(void);//返回读取模块后的湿度数据

#endif