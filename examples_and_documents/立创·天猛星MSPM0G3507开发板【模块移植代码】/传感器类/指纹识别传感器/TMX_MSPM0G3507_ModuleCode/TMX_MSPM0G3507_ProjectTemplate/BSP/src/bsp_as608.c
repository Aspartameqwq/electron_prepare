/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */
#include "bsp_as608.h"
#include "stdio.h"
#include "string.h"

volatile unsigned char FPM10A_RECEICE_BUFFER[32];
unsigned int finger_id = 0;

const unsigned char FPM10A_Get_Device[10] ={0x01,0x00,0x07,0x13,0x00,0x00,0x00,0x00,0x00,0x1b};//口令验证
const unsigned char FPM10A_Pack_Head[6] = {0xEF,0x01,0xFF,0xFF,0xFF,0xFF};  //协议包头
const unsigned char FPM10A_Get_Img[6] = {0x01,0x00,0x03,0x01,0x00,0x05};    //获得指纹图像
const unsigned char FPM10A_Get_Templete_Count[6] ={0x01,0x00,0x03,0x1D,0x00,0x21 }; //获得模版总数
const unsigned char FPM10A_Search[11]={0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x03,0xE7,0x00,0xF8}; //搜索指纹搜索范围0 - 999,使用BUFFER1中的特征码搜索
const unsigned char FPM10A_Search_0_9[11]={0x01,0x00,0x08,0x04,0x01,0x00,0x00,0x00,0x13,0x00,0x21}; //搜索0-9号指纹
const unsigned char FPM10A_Img_To_Buffer1[7]={0x01,0x00,0x04,0x02,0x01,0x00,0x08}; //将图像放入到BUFFER1
const unsigned char FPM10A_Img_To_Buffer2[7]={0x01,0x00,0x04,0x02,0x02,0x00,0x09}; //将图像放入到BUFFER2
const unsigned char FPM10A_Reg_Model[6]={0x01,0x00,0x03,0x05,0x00,0x09}; //将BUFFER1跟BUFFER2合成特征模版
const unsigned char FPM10A_Delete_All_Model[6]={0x01,0x00,0x03,0x0d,0x00,0x11};//删除指纹模块里所有的模版
volatile unsigned char  FPM10A_Save_Finger[9]={0x01,0x00,0x06,0x06,0x01,0x00,0x0B,0x00,0x19};//将BUFFER1中的特征码存放到指定的位置


uint8_t  u1_recv_buff[USART1_RECEIVE_LENGTH]; // 接收缓冲区
uint16_t u1_recv_length;                      // 接收数据长度
uint8_t  u1_recv_flag;                        // 接收完成标志位


/******************************************************************
 * 函 数 名 称：as608_config
 * 函 数 说 明：初始化as608
 * 函 数 形 参：
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：as608的默认波特率是57600
******************************************************************/
void as608_config()
{
    //清除串口中断标志
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    //使能串口中断
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);

    //清除串口中断标志
    NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN);
    //使能串口中断
    NVIC_EnableIRQ(UART_1_INST_INT_IRQN);

}

/************************************************
函数名称 ： uart1_send_byte
功    能 ： 串口发送一个字节
参    数 ： ucch：要发送的字节
返 回 值 ：
作    者 ： LC
*************************************************/
void uart1_send_byte(uint8_t ucch)
{
    //当串口1忙的时候等待，不忙的时候再发送传进来的字符
    while( DL_UART_isBusy(UART_1_INST) == true );
    //发送单个字符
    DL_UART_Main_transmitData(UART_1_INST, ucch);
}

/************************************************
函数名称 ： uart1_receive_clear
功    能 ： 清除串口接收的全部数据
参    数 ： 无
返 回 值 ： 无
作    者 ： LC
*************************************************/
void  uart1_receive_clear(void)
{
    unsigned int i = 0;
    for( i = 0; i < USART1_RECEIVE_LENGTH; i++ )
    {
        u1_recv_buff[ i ] = 0;
    }
    u1_recv_length = 0;
	u1_recv_flag = 0;
}

/******************************************************************
 * 函 数 名 称：get_as608_touch
 * 函 数 说 明：获取是否有手指触摸识别区
 * 函 数 形 参：无
 * 函 数 返 回：0没有触摸    1有触摸
 * 作       者：LC
 * 备       注：无
******************************************************************/
char get_as608_touch(void)
{
	if( TOUCH_IN == 1 )//触摸为1
	{
		//lc_printf("Touch-1\r\n");
		return 1;
	}
	else
	{
		//lc_printf("Touch-0\r\n");
	}
	return 0;
}

/******************************************************************
 * 函 数 名 称：FPM10A_Cmd_Send_Pack_Head
 * 函 数 说 明：发送包头
 * 函 数 形 参：无
 * 函 数 返 回：wu
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Cmd_Send_Pack_Head(void)
{
	int i;
	for(i=0;i<6;i++) //包头
	{
		uart1_send_byte(FPM10A_Pack_Head[i]);
	}
}
/******************************************************************
 * 函 数 名 称：FPM10A_Cmd_Check
 * 函 数 说 明：发送指令
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Cmd_Check(void)
{
	int i=0;
	FPM10A_Cmd_Send_Pack_Head(); //发送通信协议包头

	for(i=0;i<10;i++)
	{
		uart1_send_byte(FPM10A_Get_Device[i]);
	}
}
/******************************************************************
 * 函 数 名 称：FPM10A_Receive_Data
 * 函 数 说 明：接收反馈数据缓冲
 * 函 数 形 参：ucLength 接收长度
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Receive_Data(unsigned char ucLength)
{
	unsigned char i = 0;
	unsigned int timeout = 1000;//超时时间,单位Ms
	//等待数据接收完毕
	while(u1_recv_flag==0 && timeout > 0 )
	{
		delay_ms(1);
		timeout--;
	}

	delay_ms(100);        // 一定要加延时！！！
	if( u1_recv_flag == 1 )
	{
		u1_recv_flag = 0;
		for (i=0;i<ucLength;i++)
		{
			FPM10A_RECEICE_BUFFER[i] = u1_recv_buff[i];
		}
		uart1_receive_clear();
	}
	else
	{
	  //Error, no data received
	}

}
/******************************************************************
 * 函 数 名 称：FPM10A_Cmd_Get_Img
 * 函 数 说 明：获得指纹图像命令
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Cmd_Get_Img(void)
{
    unsigned char i;
    FPM10A_Cmd_Send_Pack_Head(); //发送通信协议包头
    for(i=0;i<6;i++) //发送命令 0x1d
	{
		uart1_send_byte(FPM10A_Get_Img[i]);
	}
}
/******************************************************************
 * 函 数 名 称：FINGERPRINT_Cmd_Img_To_Buffer1
 * 函 数 说 明：将图像转换成特征码存放在Buffer1中
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FINGERPRINT_Cmd_Img_To_Buffer1(void)
{
	unsigned char i;
	FPM10A_Cmd_Send_Pack_Head(); //发送通信协议包头

	for(i=0;i<7;i++)   //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
	{
		uart1_send_byte(FPM10A_Img_To_Buffer1[i]);
	}
}
/******************************************************************
 * 函 数 名 称：FINGERPRINT_Cmd_Img_To_Buffer2
 * 函 数 说 明：将图像转换成特征码存放在Buffer2中
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FINGERPRINT_Cmd_Img_To_Buffer2(void)
{
	unsigned char i;
	for(i=0;i<6;i++)    //发送包头
	{
		uart1_send_byte(FPM10A_Pack_Head[i]);
	}
	for(i=0;i<7;i++)   //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
	{
		uart1_send_byte(FPM10A_Img_To_Buffer2[i]);
	}
}
/******************************************************************
 * 函 数 名 称：FPM10A_Cmd_Search_Finger
 * 函 数 说 明：搜索全部用户999枚
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Cmd_Search_Finger(void)
{
    unsigned char i;
    FPM10A_Cmd_Send_Pack_Head(); //发送通信协议包头

    for(i=0;i<11;i++)
    {
        uart1_send_byte(FPM10A_Search[i]);
    }
}
/******************************************************************
 * 函 数 名 称：FPM10A_Cmd_Reg_Model
 * 函 数 说 明：将BUFFER1跟BUFFER2合成特征模版
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Cmd_Reg_Model(void)
{
    unsigned char i;
    FPM10A_Cmd_Send_Pack_Head(); //发送通信协议包头

    for(i=0;i<6;i++)
    {
        uart1_send_byte(FPM10A_Reg_Model[i]);
    }
}
/******************************************************************
 * 函 数 名 称：FINGERPRINT_Cmd_Delete_All_Model
 * 函 数 说 明：删除指纹模块里的所有指纹模版
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FINGERPRINT_Cmd_Delete_All_Model(void)
{
	unsigned char i;

	for(i=0;i<6;i++) //包头
	{
		uart1_send_byte(FPM10A_Pack_Head[i]);
	}

	for(i=0;i<6;i++) //命令合并指纹模版
	{
		uart1_send_byte(FPM10A_Delete_All_Model[i]);
	}
}
/******************************************************************
 * 函 数 名 称：FPM10A_Cmd_Save_Finger
 * 函 数 说 明：保存指纹
 * 函 数 形 参：保存指纹的位置（ID号）
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Cmd_Save_Finger( unsigned int storeID )
{
    unsigned long temp = 0;
    unsigned char i;
    FPM10A_Save_Finger[5] =(storeID&0xFF00)>>8;
    FPM10A_Save_Finger[6] = (storeID&0x00FF);

    for(i=0;i<7;i++)   //计算校验和
    {
        temp = temp + FPM10A_Save_Finger[i];
    }

    FPM10A_Save_Finger[7]=(temp & 0x00FF00) >> 8; //存放校验数据
    FPM10A_Save_Finger[8]= temp & 0x0000FF;
    FPM10A_Cmd_Send_Pack_Head(); //发送通信协议包头

    for(i=0;i<9;i++)
    {
        //发送命令 将图像转换成 特征码 存放在 CHAR_buffer1
        uart1_send_byte(FPM10A_Save_Finger[i]);
    }
}




/******************************************************************
 * 函 数 名 称：key_scanf
 * 函 数 说 明：按键功能  确定-取消
 * 函 数 形 参：
 * 函 数 返 回：0=未动作  1=确定  2=取消
 * 作       者：LC
 * 备       注：当前是使用串口来模拟按键，如果你有按键请自行修改
 *              修改要求：按下确定键时返回1；按下取消键时，返回2。
******************************************************************/
uint8_t  recv0_buff[USART1_RECEIVE_LENGTH];
uint16_t recv0_length;
uint8_t  recv0_flag;

void recv0_clear_buff()
{
	for(int i = 0; i < USART1_RECEIVE_LENGTH; i++)
	{
		recv0_buff[i] = 0;
	}

	recv0_length = 0;
	recv0_flag = 0;
}


char key_scanf(void)
{

    /***************    你的代码    ***************/
    if(strstr( (const char*)recv0_buff, "Yes") != NULL )
    {
        lc_printf("key_scanf-YES\r\n");

		recv0_clear_buff();

        return 1;//返回 确定键被按下
    }
    if(strstr( (const char*)recv0_buff, "No") != NULL )
    {
        lc_printf("key_scanf-NO\r\n");

		recv0_clear_buff();

        return 2;//返回 取消键被按下
    }
     /***************    你的代码    ***************/

    return 0;
}


/******************************************************************
 * 函 数 名 称：FPM10A_Add_Fingerprint
 * 函 数 说 明：添加指纹
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Add_Fingerprint(void)
{
//        unsigned char id_show[3]={0,0,0};
        unsigned char key_num= key_scanf();
        finger_id=0;

        lc_printf("Do you want to add fingerprints? [Yes/No]\r\n");

        while( key_num != 2 )//按返回键直接回到主菜单
        {
			key_num= key_scanf();

			 //按确认键开始录入指纹信息
			 if( key_num == 1 )
			 {
				lc_printf("start add\r\n");

				while( key_num != 2 )//按下返回键退出录入返回fingerID调整状态
				{
					key_num= key_scanf();
					FPM10A_Cmd_Get_Img(); //获得指纹图像
					FPM10A_Receive_Data(12);
					//判断接收到的确认码,等于0指纹获取成功
					if(FPM10A_RECEICE_BUFFER[9]==0)
					{
						delay_ms(100);
						FINGERPRINT_Cmd_Img_To_Buffer1();
						FPM10A_Receive_Data(12);
						delay_ms(1000);
						while( key_num != 2 )
						{
							key_num= key_scanf();
							FPM10A_Cmd_Get_Img(); //获得指纹图像
							FPM10A_Receive_Data(12);
							//判断接收到的确认码,等于0指纹获取成功
							if(FPM10A_RECEICE_BUFFER[9]==0)
							{
								delay_ms(200);
								lc_printf("successfully added, ID = %d\r\n",finger_id);
								FINGERPRINT_Cmd_Img_To_Buffer2();
								FPM10A_Receive_Data(12);
								FPM10A_Cmd_Reg_Model();//转换成特征码
								FPM10A_Receive_Data(12);
								//保存指纹
								FPM10A_Cmd_Save_Finger(finger_id);
								FPM10A_Receive_Data(12);
								delay_ms(1000);
								finger_id=finger_id+1;
								break;
							}
						}

						break;
					}
				}
			}
        }
}
/******************************************************************
 * 函 数 名 称：FPM10A_Find_Fingerprint
 * 函 数 说 明：搜索指纹
 * 函 数 形 参：无
 * 函 数 返 回：指纹ID号
 * 作       者：LC
 * 备       注：255：未查到  其他：查找到了
******************************************************************/
unsigned int FPM10A_Find_Fingerprint(void)
{
	unsigned int find_fingerid = 255;

//    lc_printf("Please put your finger in\r\n");

    if( get_as608_touch() == 1 )//有手指触摸识别区
    {
        FPM10A_Cmd_Get_Img(); //获得指纹图像
        FPM10A_Receive_Data(12);
        //判断接收到的确认码,等于0指纹获取成功
        if(FPM10A_RECEICE_BUFFER[9]==0)
        {
            delay_ms(100);
            FINGERPRINT_Cmd_Img_To_Buffer1();
            FPM10A_Receive_Data(12);
            FPM10A_Cmd_Search_Finger();
            FPM10A_Receive_Data(16);
            if(FPM10A_RECEICE_BUFFER[9] == 0) //搜索成功
            {
                //拼接指纹ID数
                find_fingerid = FPM10A_RECEICE_BUFFER[10]*256 + FPM10A_RECEICE_BUFFER[11];
                lc_printf("ID = %d\r\n",find_fingerid);
                delay_ms(500);
            }
            else //没有找到
            {
                lc_printf("not found\r\n");
            }
        }
    }
    return find_fingerid;
}
/******************************************************************
 * 函 数 名 称：FPM10A_Delete_All_Fingerprint
 * 函 数 说 明：删除所有存贮的指纹库
 * 函 数 形 参：无
 * 函 数 返 回：无
 * 作       者：LC
 * 备       注：无
******************************************************************/
void FPM10A_Delete_All_Fingerprint(void)
{
	unsigned char key_num=0;
	lc_printf("Whether to delete fingerprint ?  [Yes/No]\r\n");

	do{
		key_num = key_scanf();
		if(key_num == 1 )//点击确定键
		{
			lc_printf("deleting\r\n");
			delay_ms(300);
			FINGERPRINT_Cmd_Delete_All_Model();
			FPM10A_Receive_Data(12);
			lc_printf("Have all been cleared\r\n");
			break;
		}
		if( u1_recv_flag == 1 )
		{
			u1_recv_flag = 0;
			lc_printf("rev = %s\r\n", u1_recv_buff);
		}
	}while( key_num != 2 );//没有点击取消键，则继续循环
}

/******************************************************************
 * 函 数 名 称：Device_Check
 * 函 数 说 明：模块检查
 * 函 数 形 参：无
 * 函 数 返 回：0未检测到模块或者模块异常  1检测到模块并且通信成功
 * 作       者：LC
 * 备       注：返回0时要注意接线是否正确、串口配置是否可用
******************************************************************/
char Device_Check(void)
{
		FPM10A_RECEICE_BUFFER[9]=1; //串口数组第九位可判断是否通信正常

		FPM10A_Cmd_Check();    //单片机向指纹模块发送校对命令
		FPM10A_Receive_Data(12);     //将串口接收到的数据转存

		if(FPM10A_RECEICE_BUFFER[9] == 0)  //判断数据低第9位是否接收到0
		{
			return 1;
		}
		return 0;
}


/* 在这里实现串口1的接收 */
/* 串口的中断服务函数 */
void UART_0_INST_IRQHandler(void)
{
    //如果产生了串口中断
    switch( DL_UART_getPendingInterrupt(UART_0_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断
            //接发送过来的数据保存在变量中
            recv0_buff[recv0_length++] = DL_UART_Main_receiveData(UART_0_INST);

            //lc_printf("%c", recv0_buff[recv0_length - 1]);

            recv0_buff[recv0_length] = '\0';  // 数据接收完毕，数组结束标志
            recv0_flag = 1;

            break;

        default://其他的串口中断
            break;
    }
}


//串口的中断服务函数
void UART_1_INST_IRQHandler(void)
{
    //如果产生了串口中断
    switch( DL_UART_getPendingInterrupt(UART_1_INST) )
    {
        case DL_UART_IIDX_RX://如果是接收中断
            //接发送过来的数据保存在变量中
            u1_recv_buff[u1_recv_length++] = DL_UART_Main_receiveData(UART_1_INST);

            u1_recv_buff[u1_recv_length] = '\0';  // 数据接收完毕，数组结束标志
            u1_recv_flag = 1;

            break;

        default://其他的串口中断
            break;
    }
}


