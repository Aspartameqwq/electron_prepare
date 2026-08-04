/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#include "bsp_hx711.h"
#include "stdio.h"

uint32_t HX711_Weight_Init;

//校准参数
//因为不同的传感器特性曲线不是很一致，因此，每一个传感器需要矫正这里这个参数才能使测量值很准确。
//当发现测试出来的重量偏大时，增加该数值。
//如果测试出来的重量偏小时，减小改数值。
//该值可以为小数
#define GAP_VALUE   207.00

/******************************************************************
 * 函 数 名 称：HX711_Read
 * 函 数 说 明：读取HX711
 * 函 数 形 参：无
 * 函 数 返 回：读取到的值
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
static uint32_t HX711_Read(void) //增益128
{
    unsigned long count;
    unsigned char i;

    DT_OUT();
    delay_us(5);

    DT(1);
    delay_us(1);
    SCK(0);
    count=0;

    DT_IN();
    delay_us(5);

    while(DT_GET());
    for(i=0;i<24;i++)
    {
        SCK(1);
        count=count<<1;
        delay_us(1);
        SCK(0);
        if(DT_GET())
            count++;
        delay_us(1);
    }
    SCK(1);
    count=count^0x800000;//第25个脉冲下降沿来时，转换数据
    delay_us(1);
    SCK(0);

    return(count);
}

/******************************************************************
 * 函 数 名 称：HX711_Get_InitValue
 * 函 数 说 明：获取初始值
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：后续的重量都是以该初始重量为0值，因此在初始化时，秤上不要放任何东西
******************************************************************/
void HX711_Get_InitValue(void)
{
    HX711_Weight_Init = HX711_Read();
}
/******************************************************************
 * 函 数 名 称：HX711_Get_Weight
 * 函 数 说 明：称重
 * 函 数 形 参：无
 * 函 数 返 回：称重值，单位g
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
float HX711_Get_Weight(void)
{
    uint32_t HX711_Read_Buffer = 0;
    float Return_Buffer = 0;

    HX711_Read_Buffer = HX711_Read();

    if(HX711_Read_Buffer > HX711_Weight_Init)
    {
        HX711_Read_Buffer -= HX711_Weight_Init; //获取实物的采样数值。

        Return_Buffer = (float)HX711_Read_Buffer / (float)GAP_VALUE;//计算实物的实际重量
        //因为不同的传感器特性曲线不一样，因此，每一个传感器需要矫正这里的 GAP_VALUE 这个除数。
        //当发现测试出来的重量偏大时，增加该数值。
        //如果测试出来的重量偏小时，减小改数值。
    }

    return Return_Buffer;
}

