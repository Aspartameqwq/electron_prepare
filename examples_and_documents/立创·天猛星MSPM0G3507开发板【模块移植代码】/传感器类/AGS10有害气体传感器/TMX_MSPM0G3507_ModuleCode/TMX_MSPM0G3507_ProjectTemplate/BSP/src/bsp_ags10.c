/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_ags10.h"

//起始信号
void AGS10_IIC_Start(void)
{
    SDA_OUT();
    SDA(1);
    SCL(1);
    delay_1us(5);
    SDA(0);
    delay_1us(5);
    SCL(0);
    delay_1us(5);
}

//停止信号
void AGS10_IIC_Stop(void)
{
    SDA_OUT();
    SCL(0);
    SDA(0);
    SCL(1);
    delay_1us(5);
    SDA(1);
    delay_1us(5);
}

//发送非应答
void AGS10_IIC_Send_Nack(void)
{
    SDA_OUT();
    SCL(0);
    SDA(0);
    SDA(1);
    SCL(1);
    delay_1us(5);
    SCL(0);
    SDA(0);
}

//发送应答
void AGS10_IIC_Send_Ack(void)
{
    SDA_OUT();
    SCL(0);
    SDA(1);
    SDA(0);
    SCL(1);
    delay_1us(5);
    SCL(0);
    SDA(1);
}

/**********************************************************
 * 函 数 名 称：I2C_WaitAck
 * 函 数 功 能：等待从机应答
 * 传 入 参 数：无
 * 函 数 返 回：1=非应答         0=应答
 * 作       者：LCKFB
 * 备       注：无
**********************************************************/
unsigned char AGS10_I2C_WaitAck(void)
{
        char ack = 0;
        unsigned char ack_flag = 10;
        SCL(0);
        SDA(1);
        SDA_IN();
        delay_1us(5);
        SCL(1);
        delay_1us(5);

        while( (SDA_GET()==1)  &&  ( ack_flag ) )
        {
                ack_flag--;
                delay_1us(5);
        }

        //非应答
        if( ack_flag <= 0 )
        {
                AGS10_IIC_Stop();
                return 1;
        }
        else//应答
        {
                SCL(0);
                SDA_OUT();
        }
        return ack;
}

//发送一个字节
void AGS10_IIC_Send_Byte(uint8_t dat)
{
    int i = 0;
    SDA_OUT();
    SCL(0);

    for( i = 0; i < 8; i++ )
    {
        SDA( (dat & 0x80) >> 7 );
        delay_1us(5);
        SCL(1);
        delay_1us(5);
        SCL(0);
        delay_1us(5);
        dat<<=1;
    }
}

//接收一个字节
unsigned char AGS10_IIC_Read_Byte(void)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
    {
        SCL(0);
        delay_1us(5);
        SCL(1);
        delay_1us(5);
        receive<<=1;
        if( SDA_GET() )
        {
            receive |= 1;
        }
    }
    SCL(0);
    return receive;
}

//********************************************************************
//函数名称：Calc_CRC8
//功能 ：CRC8 计算，初值：0xFF，多项式：0x31(x8 + x5 + x4 +1)
//参数 ：u8* dat：需要校验数据的首地址；u8 Num：CRC 校验数据长度
//返回 ：crc：计算出的校验值
//********************************************************************
uint8_t Calc_CRC8(uint8_t *dat, uint8_t Num)
{
    uint8_t i, byte, crc=0xFF;
    for(byte=0; byte<Num; byte++)
    {
        crc ^= (dat[byte]);
        for( i = 0; i < 8; i++ )
        {
            if(crc & 0x80)  crc = ( crc << 1 ) ^ 0x31;
            else            crc = ( crc << 1 );
        }
    }
    return crc;
}

/**********************************************************
 * 函 数 名 称：ags10_read
 * 函 数 功 能：读取AGS10的TVOC浓度数据
 * 传 入 参 数：无
 * 函 数 返 回：1： 通信失败
 *             2：发送失败
 *             3：等待超时
 *             4: 校验失败
 * 作       者：LCKFB
 * 备       注：
**********************************************************/
uint32_t ags10_read(void)
{
    uint8_t timeout = 0;
    uint8_t data[5] = {0};
    uint32_t TVOC_data = 0;

    AGS10_IIC_Start();
    AGS10_IIC_Send_Byte(0X34);
    if( AGS10_I2C_WaitAck() == 1 ) return 1;
    AGS10_IIC_Send_Byte(0X00);
    if( AGS10_I2C_WaitAck() == 1 ) return 2;
    AGS10_IIC_Stop();

    do{
        delay_1ms(1);
        timeout++;
        AGS10_IIC_Start();
        AGS10_IIC_Send_Byte(0X35);
    }while( (AGS10_I2C_WaitAck() == 1) && (timeout >= 50) );

    //如果超时
    if( timeout >= 50 ) return 3;

    data[0] = AGS10_IIC_Read_Byte();
    AGS10_IIC_Send_Ack();
    data[1] = AGS10_IIC_Read_Byte();
    AGS10_IIC_Send_Ack();
    data[2] = AGS10_IIC_Read_Byte();
    AGS10_IIC_Send_Ack();
    data[3] = AGS10_IIC_Read_Byte();
    AGS10_IIC_Send_Ack();
    data[4] = AGS10_IIC_Read_Byte();
    AGS10_IIC_Send_Nack();

    AGS10_IIC_Stop();

    if( Calc_CRC8(data,4) != data[4] )
    {
        lc_printf("\nCheck failed\r\n");
        return 4;
    }
    TVOC_data = (data[1]<<16) | (data[2]<<8) | data[3] ;
    return TVOC_data;
}
