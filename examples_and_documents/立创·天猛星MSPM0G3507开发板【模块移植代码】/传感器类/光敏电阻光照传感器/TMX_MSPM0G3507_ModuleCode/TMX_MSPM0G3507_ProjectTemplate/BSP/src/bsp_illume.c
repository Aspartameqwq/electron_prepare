/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_illume.h"
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
/**********************************************************
 * 函 数 名 称：Get_Adc_Value
 * 函 数 功 能：获得某个通道的值
 * 传 入 参 数：Count：采集次数
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：LP
**********************************************************/
uint32_t Get_Adc_Value(uint8_t Count)
{
	unsigned int gAdcResult = 0;
	uint8_t i = 0;
	for(i = 0; i < Count; i++)
	{
		//获取数据
		gAdcResult += ADC_GET();
	}

	return (gAdcResult / Count);
}

/******************************************************************
 * 函 数 名 称：Get_illume_Percentage_value
 * 函 数 说 明：读取光敏电阻值，并且返回百分比
 * 函 数 形 参：无
 * 函 数 返 回：返回百分比
 * 作       者：LCKFB
 * 备       注：最亮100  最暗0
******************************************************************/
uint32_t Get_illume_Percentage_value(void)
{
    //ADC精度都是12位
    //2的12次方 = 4096
    //因为单片机是从0开始算，所以要4096-1=4095
    int adc_max = 4095;
    int adc_new = 0;
    int Percentage_value = 0;

    adc_new = Get_Adc_Value(10);
    //百分比 = （ 当前值 / 最大值 ）* 100
    Percentage_value = ( 1 - ( (float)adc_new / adc_max ) ) * 100;

    return Percentage_value;
}

/******************************************************************
 * 函 数 名 称：Get_DO_In
 * 函 数 说 明：读取DO引脚的电平状态
 * 函 数 形 参：无
 * 函 数 返 回：1=检测过亮   0=检测过暗
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
uint8_t Get_DO_In(void)
{
    if( GET_DO_IN == 1 )
    {
        return 1;
    }
    return 0;
}
