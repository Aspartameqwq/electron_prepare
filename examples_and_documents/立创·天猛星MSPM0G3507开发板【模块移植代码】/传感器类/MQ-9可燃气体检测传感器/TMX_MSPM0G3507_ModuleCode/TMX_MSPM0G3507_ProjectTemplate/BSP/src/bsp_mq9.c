/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#include "bsp_mq9.h"
#include "stdio.h"


/**********************************************************
 * 函 数 名 称：ADC_GET
 * 函 数 功 能：读取一次ADC数据
 * 传 入 参 数：无
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：LP
**********************************************************/
static uint32_t ADC_GET(void)
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

    //获取数据
    gAdcResult = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_CH0);

    return gAdcResult;
}
/******************************************************************
 * 函 数 名 称：Get_Adc_MQ9_Value
 * 函 数 说 明：
 * 函 数 形 参：
 * 函 数 返 回：对应扫描的ADC值
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
unsigned int Get_Adc_MQ9_Value(void)
{
    uint32_t Data = 0;

    for(int i = 0; i < SAMPLES; i++)
    {
        Data += ADC_GET();

        delay_ms(5);
    }

    Data = Data / SAMPLES;

    return Data;
}

/******************************************************************
 * 函 数 名 称：Get_MQ9_Percentage_value
 * 函 数 说 明：读取MQ9值，并且返回百分比
 * 函 数 形 参：无
 * 函 数 返 回：返回百分比
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
unsigned int Get_MQ9_Percentage_value(void)
{
      int adc_max = 4095;
      int adc_new = 0;
      int Percentage_value = 0;

      adc_new = Get_Adc_MQ9_Value();

      Percentage_value = ((float)adc_new / (float)adc_max) * 100.f;
      return Percentage_value;
}

/******************************************************************
 * 函 数 名 称：Get_MQ9_DO
 * 函 数 说 明：获取 MQ9 DO引脚的电平状态
 * 函 数 形 参：无
 * 函 数 返 回：0=未检测到高于灵敏度的酒精值 1=检测到高于灵敏度的酒精值
 * 作       者：LCKFB
 * 备       注：调整模块上的滑动电阻即可调整灵敏度
******************************************************************/
char Get_MQ9_DO(void)
{
    if( MQ_DO == 0 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


