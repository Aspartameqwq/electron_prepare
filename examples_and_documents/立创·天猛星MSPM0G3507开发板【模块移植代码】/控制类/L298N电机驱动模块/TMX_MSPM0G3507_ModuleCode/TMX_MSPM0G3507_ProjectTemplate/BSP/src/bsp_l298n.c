/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_l298n.h"

/******************************************************************
 * 函 数 名 称：AB_Motor_Stop
 * 函 数 说 明：AB端口电机停止
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
void AB_Motor_Stop(void)
{
    //AIN1输出
    DL_TimerG_setCaptureCompareValue(PWM_IN12_INST, 999 , GPIO_PWM_IN12_C0_IDX);
    //AIN2输出
    DL_TimerG_setCaptureCompareValue(PWM_IN12_INST, 999 , GPIO_PWM_IN12_C1_IDX);
    //AIN3输出
    DL_TimerG_setCaptureCompareValue(PWM_IN34_INST, 999 , GPIO_PWM_IN34_C0_IDX);
    //AIN4输出
    DL_TimerG_setCaptureCompareValue(PWM_IN34_INST, 999 , GPIO_PWM_IN34_C1_IDX);
}

/******************************************************************
 * 函 数 名 称：A_Motor_Control
 * 函 数 说 明：A端口电机控制
 * 函 数 形 参：dir旋转方向 1正转0反转   speed旋转速度，范围（0 ~ per-1）
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
void A_Motor_Control(uint8_t dir, uint32_t speed)
{
    if( dir == 1 )
    {
        //AIN1输出
        DL_TimerG_setCaptureCompareValue(PWM_IN12_INST, 0 , GPIO_PWM_IN12_C0_IDX);
        //AIN2输出
        DL_TimerG_setCaptureCompareValue(PWM_IN12_INST, speed , GPIO_PWM_IN12_C1_IDX);
    }
    else
    {
        //AIN1输出
        DL_TimerG_setCaptureCompareValue(PWM_IN12_INST, speed , GPIO_PWM_IN12_C0_IDX);
        //AIN2输出
        DL_TimerG_setCaptureCompareValue(PWM_IN12_INST, 0 , GPIO_PWM_IN12_C1_IDX);
    }
}


/******************************************************************
 * 函 数 名 称：B_Motor_Control
 * 函 数 说 明：B端口电机控制
 * 函 数 形 参：dir旋转方向 1正转0反转   speed旋转速度，范围（0 ~ per-1）
 * 函 数 返 回：无
 * 作       者：LCKFB
 * 备       注：无
******************************************************************/
void B_Motor_Control(uint8_t dir, uint32_t speed)
{
    if( dir == 1 )
    {
        //AIN3输出
        DL_TimerG_setCaptureCompareValue(PWM_IN34_INST, 0 , GPIO_PWM_IN34_C0_IDX);
        //AIN4输出
        DL_TimerG_setCaptureCompareValue(PWM_IN34_INST, speed , GPIO_PWM_IN34_C1_IDX);
    }
    else
    {
        //AIN3输出
        DL_TimerG_setCaptureCompareValue(PWM_IN34_INST, 0 , GPIO_PWM_IN34_C0_IDX);
        //AIN4输出
        DL_TimerG_setCaptureCompareValue(PWM_IN34_INST, speed , GPIO_PWM_IN34_C1_IDX);
    }
}

