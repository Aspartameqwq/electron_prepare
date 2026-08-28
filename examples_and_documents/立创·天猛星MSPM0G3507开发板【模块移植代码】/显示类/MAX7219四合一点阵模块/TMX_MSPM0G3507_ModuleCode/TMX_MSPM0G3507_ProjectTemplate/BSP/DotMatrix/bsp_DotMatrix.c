/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_DotMatrix.h"
#include "stdio.h"


/******************************************************************
 * 函 数 名 称：Write_DotMatrix_byte
 * 函 数 说 明：向DotMatrix写入字节
 * 函 数 形 参：dat写入的数据
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void Write_DotMatrix_byte(uint8_t dat)
{
    uint8_t i;
    DotMatrix_CS(0);
    for(i=8;i>=1;i--)
    {
        DotMatrix_CLK(0);
        if( dat&0x80 )
        {
            DotMatrix_DIN(1);
        }
        else
        {
            DotMatrix_DIN(0);
        }
        dat=dat<<1;
        DotMatrix_CLK(1);
    }
}

/******************************************************************
 * 函 数 名 称：Write_DotMatrix
 * 函 数 说 明：向DotMatrix写入数据
 * 函 数 形 参：address写入地址  dat写入数据
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void Write_DotMatrix(uint8_t address,uint8_t dat)
{
    Write_DotMatrix_byte(address);           //写入地址，即点阵行号1-8
    Write_DotMatrix_byte(dat);               //写入数据，即该行显示内容
}

/******************************************************************
 * 函 数 名 称：DotMatrix_Lock
 * 函 数 说 明：将更新的数据写入芯片
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void DotMatrix_Lock(void)
{
    DotMatrix_CS(1);
    DotMatrix_CS(0);
}

/******************************************************************
 * 函 数 名 称：DotMatrix_display
 * 函 数 说 明：4个点阵显示
 * 函 数 形 参：show1第一个点阵显示内容   show2第二个 show3第三个 show4第四个
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void DotMatrix_display(uint8_t* show1, uint8_t* show2, uint8_t* show3, uint8_t* show4)
{
    uint8_t i = 0;
    for(i = 1; i < 9; i++ )//1~8行 进行更新
    {
        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(show1[i-1]);  //第一个点阵

        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(show2[i-1]);  //第二个点阵

        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(show3[i-1]);  //第三个点阵

        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(show4[i-1]);  //第四个点阵
        DotMatrix_Lock();//锁存显示数据
    }
}


/******************************************************************
 * 函 数 名 称：Write_DotMatrix_AllOff
 * 函 数 说 明：控制第一片DotMatrix的全部数码管全灭
 * 函 数 形 参：address写入地址  dat写入数据
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void Write_DotMatrix_AllOff(void)
{
    int i =  0;
    for( i = 1; i < 9; i++ )//1~8行 进行更新
    {
        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(0x00);  //全灭

        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(0x00);  //全灭

        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(0x00);  //全灭

        Write_DotMatrix_byte(i);   //写入地址，即行编号1-8
        Write_DotMatrix_byte(0x00);  //全灭
        DotMatrix_Lock();//更新内容
    }
}
/******************************************************************
 * 函 数 名 称：DotMatrix_Init
 * 函 数 说 明：DotMatrix初始化
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void DotMatrix_Init(void)
{
    unsigned int i = 0;

    for( i = 0; i < 4; i++ )//设置4个点阵
    {
        Write_DotMatrix(0x09, 0x00);       //译码方式：不进行译码
    }
    DotMatrix_Lock();//更新设置
    for( i = 0; i < 4; i++ )//设置4个点阵
    {
        Write_DotMatrix(0x0a, 0x01);       //亮度
    }
    DotMatrix_Lock();//更新设置
    for( i = 0; i < 4; i++ )//设置4个点阵
    {
        Write_DotMatrix(0x0b, 0x07);       //扫描界限；8个数码管显示
    }
    DotMatrix_Lock();//更新设置

    for( i = 0; i < 4; i++ )//设置4个点阵
    {
        Write_DotMatrix(0x0c, 0x01);       //掉电模式：0，普通模式：1
    }
    DotMatrix_Lock();//更新设置
    for( i = 0; i < 4; i++ )//设置4个点阵
    {
        Write_DotMatrix(0x0f, 0x00);       //显示测试：1；测试结束，正常显示：0
    }
    DotMatrix_Lock();//更新设置
}