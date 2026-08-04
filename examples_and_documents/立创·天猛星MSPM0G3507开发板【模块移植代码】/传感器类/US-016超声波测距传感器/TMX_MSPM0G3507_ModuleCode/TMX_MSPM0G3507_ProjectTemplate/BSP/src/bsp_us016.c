/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_us016.h"
#include "board.h"
#include "stdio.h"

// 超声波量程 Range接地量程为1  Range接VCC量程为3  浮空量程为3
// 测试时，Range浮空，故量程为3

#define RANGE   0       /* [0]则量程为3米  [1]则量程为1米 */


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
 * 函 数 名 称：Get_US016_Value
 * 函 数 说 明：对ADC值进行平均值计算后输出
 * 函 数 形 参：无
 * 函 数 返 回：对应扫描的ADC值
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
unsigned int Get_US016_Value(void)
{
    uint32_t Data = 0;

    for(int i = 0; i < SAMPLES; i++)
    {
        Data += ADC_GET();

        //lc_printf("Data = %d\r\n", Data);

        delay_ms(5);
    }

    Data = Data / SAMPLES;

    //lc_printf("return Data = %d\r\n", Data);

    return Data;
}
/******************************************************************
 * 函 数 名 称：Get_US016_Distance
 * 函 数 说 明：读取测距距离
 * 函 数 形 参：无
 * 函 数 返 回：浮点型的测距的距离
 * 作       者：LCKFB
 * 备       注：
量程为3米时距离公式为：L = (A*3072/4096)*（Vref/Vcc）
量程为1米时距离公式为：L = (A*1024/4096)*（Vref/Vcc）
                      Vref 为 ADC 的参考电压，Vcc 为 US-016 的电源电压
******************************************************************/
float Get_US016_Distance(void)
{
    float distance = 0;

    unsigned int d = Get_US016_Value();
    
    #if !RANGE
        distance = d * 0.75;
    #else
        distance = d * 0.25;
    #endif

    //lc_printf("return distance = %f\r\n", distance);

    return distance;
}
