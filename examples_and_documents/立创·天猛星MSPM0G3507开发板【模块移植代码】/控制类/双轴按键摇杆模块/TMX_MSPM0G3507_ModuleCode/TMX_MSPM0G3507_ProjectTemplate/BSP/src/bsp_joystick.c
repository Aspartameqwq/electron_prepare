/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#include "bsp_joystick.h"
#include "board.h"
#include "stdio.h"

/**********************************************************
 * 函 数 名 称：ADC_GET
 * 函 数 功 能：读取一次ADC数据
 * 传 入 参 数：chx : 0为通道0，1为通道1
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：LP
**********************************************************/
uint32_t ADC_GET(uint8_t CHx)
{
    unsigned int gAdcResult = 0;

    //使能ADC转换
    DL_ADC12_enableConversions(ADC12_0_INST);
    //软件触发ADC开始转换
    DL_ADC12_startConversion(ADC12_0_INST);

    //如果当前状态 不是 空闲状态
    while (DL_ADC12_getStatus(ADC12_0_INST) != DL_ADC12_STATUS_CONVERSION_IDLE );

    //清除触发转换状态
    DL_ADC12_stopConversion(ADC12_0_INST);
    //失能ADC转换
    DL_ADC12_disableConversions(ADC12_0_INST);

    if(CHx == 0)
    {
        //获取数据
        gAdcResult = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_CH0);

        return gAdcResult;
    }
    else
    {
        //获取数据
        gAdcResult = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_CH1);

        return gAdcResult;
    }
}
/******************************************************************
 * 函 数 名 称：Get_Adc_Joystick_Value
 * 函 数 说 明：对保存的数据进行平均值计算后输出
 * 函 数 形 参：CHx 那个通道值
 * 函 数 返 回：对应扫描的ADC值
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
unsigned int Get_Adc_Joystick_Value(char CHx)
{
    uint32_t Data = 0;

    for(int i = 0; i < SAMPLES; i++)
    {
        Data += ADC_GET(CHx);

        delay_ms(10);
    }

    Data = Data / SAMPLES;

    return Data;
}

/******************************************************************
 * 函 数 名 称：Get_MQ2_Percentage_value
 * 函 数 说 明：读取摇杆值，并且返回百分比
 * 函 数 形 参：0=读取摇杆左右值，1=读取摇杆上下值
 * 函 数 返 回：返回百分比
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
uint32_t Get_Joystick_Percentage_value(char dir)
{
    int adc_new = 0;
    int Percentage_value = 0;

    if( dir == 0 )
    {
        adc_new = Get_Adc_Joystick_Value(0); // 通道0：X的值
    }
    else if( dir == 1 )
    {
        adc_new = Get_Adc_Joystick_Value(1); // 通道1：Y的值
    }
    else
    {
        lc_printf("\nCH Error!!\r\n");
    }

    Percentage_value = ((float)adc_new/4095.0f) * 100.f;
    return Percentage_value;
}

/******************************************************************
 * 函 数 名 称：Get_SW_state
 * 函 数 说 明：读取摇杆是否有按下
 * 函 数 形 参：无
 * 函 数 返 回：0摇杆被按下   1摇杆没有按下
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
char Get_SW_state(void)
{
    //如果被按下
    if( GET_SW == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
