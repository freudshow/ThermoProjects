/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: lcd_menu.cpp
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年9月10日 
**  描        述: 液晶显示目录
**	修 改  记 录:   	
*****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>
extern "C"{
#include "lcmdrv.h"
#include "PictureCode.h"
#include "HZTable.h"
#include "ASCIICode.h"
}
#include "app_flashmem.h"
#include "app_down.h"
//#include "IniJzqPara.h"
#include "StructProtocol.h"
//#include "BaseProtocol.h"
#include "interfacefuns.h"
#include "lcd_Menu.h"
#include "ReadMeterData.h"
#include "fat_sd.h"

/********************************************** struct *********************************************/
struct TimeNode{
	uint8 Hour;
	uint8 Minute;
};

struct  StateInfo{
			TimeNode 	LastNode;
			uint16		MeterNum;				//抄表总数
			uint16		ReadedCmplNum;			//抄表成功数
			uint16		UsedTime;				//本次抄表用时
};
StateInfo gREAD_MeterReadState;
/********************************************** define *********************************************/
//#define IN_LCD_MENU
#define	TotalRotatePage 5 //轮显的所有页数
#define DataItemNum     6 //采集数据项的总数
#define TotalMeterType
/*****************************************extern  global **********************************************/
extern uint16 gVersion;
/****************************************    global **********************************************/
uint8 GChannelType = 0; //当前通信通道类型
uint8 GRoMode = 0;//轮显模式 一次值或者二次值
uint8 FixPsw[6] = {0x30,0x30,0x30,0x30,0x30,0x30};
//METER_DATA gMeterData;//计量表项全局变量，计量数据放入其中
#ifdef DEMO_APP
uint8 Channel_1_SUCESS =0;
uint8 Channel_2_SUCESS =0;
uint8 Channel_3_SUCESS =0;
CJ188_Format Demo_CJ188_ch1;
CJ188_Format Demo_CJ188_ch2;
CJ188_Format Demo_CJ188_ch3;

uint8 Meter_R_End =0;
uint8 Demo_param_set =0;
uint8 Show_Data_Status=0;
uint8 Data_complete=0;
extern uint8 DemoDateIsGet ;
#endif
checkData str_check_data;

DisplayData strDisplayData; 
uint8 autotestmode = FALSE;//muxiaoqing test
uint8 manualtestmode = FALSE;
uint8 autotestRetryNum = 0;
uint8 autotestdisp3 = FALSE;
uint8 autotestIsfromExitKey =0;
uint8 autotestFirstIn =1;
uint8 testPortreadNum =0;
extern uint8 IRQ5testBuf_Counter;
extern uint8 IRQ5testBuf[32];
extern uint8 IRQ4testBuf_Counter;
extern uint8 IRQ4testBuf[32];
extern uint8 IRQ3testBuf_Counter;
extern uint8 IRQ3testBuf[32];
extern uint8 IRQ2testBuf_Counter;
extern uint8 IRQ2testBuf[32];
extern uint8 IRQ1testBuf_Counter;
extern uint8 IRQ1testBuf[32];
extern uint8 forbidsending1 ;//muxiaoqing test

extern LCDContrastPara NVLCDConraston;

//数组初始化
//本数组内关于 OK 按键后索引，有待以后添加
KeyState KeyTab[KEYBORD_MENE_SIZE] = 
{
	{0,1,1,1,1,1,0,(*MainMenu)},							//主菜单
	//一级菜单
	//{1,2,31,1,1,4,0,(*Action_LcdMenu11)},					//"参数设置与查看"					
	{1,2,3,1,1,4,0,(*Action_LcdMenu11)},					//"参数设置与查看"	muxiaoqing test				

	{2,3,1,2,2,8,0,(*Action_LcdMenu12)},					//"集中器抄表状态查看"
	{3,1,2,3,3,13,0,(*Action_LcdMenu13)},					//"集中器管理与维护"

	
	//二级菜单
	// "参数设置与查看" 下的菜单
	{4,5,45,4,4,16,1,(*Action_LcdMenu11_21)},				//"通信通道设置"
	{5,6,4,5,5,17,1,(*Action_LcdMenu11_22)}, 				//"终端时间设置"
	{6,7,5,6,6,18,1,(*Action_LcdMenu11_23)},  				//"终端编号设置"
	{7,28,6,7,7,19,1,(*Action_LcdMenu11_24)},  				//"抄表时间点设置"
   

	// "集中器抄表状态查看" 下的菜单, 暂时只有一个菜单
	{8,9,8,8,8,20,2,(*Action_LcdMenu12_21)},					//显示抄表状态菜单
	{9,10,8,9,9,21,2,(*Action_LcdMenu12_22)},							//立即抄表
	{10,11,9,10,10,22,2,(*Action_LcdMenu12_23)},							//抄表通道设置
	{11,11,10,11,11,47,2,(*Action_LcdMenu12_24)},							//失败信息查询
	{12,11,1,1,1,1,0,(*MainMenu)},							//预留


	{13,14,15,13,13,25,3,(*Action_LcdMenu13_21)},			//"重启终端"
	{14,15,13,14,14,26,3,(*Action_LcdMenu13_22)},			//"版本信息"
	{15,13,14,15,15,27,3,(*Action_LcdMenu13_23)},			//"对比度调节"

	//三级菜单
	{16,16,16,16,16,16,4,(*ZigbeeSet)},						//"设置zigbee通信，主站地址"
	{17,17,17,17,17,17,5,(*Set_SystemTime)},				//"设置时间具体函数"
	{18,18,18,18,18,18,6,(*Set_SystemAddr)},				//"设置终端编号函数"
	{19,19,19,19,19,19,7,(*Set_ReadMeterTime)},				//"设置抄表时间点函数"
	
	{20,20,20,20,20,20,8,(*ReadMeterStateShow)},			//"抄表状态信息详情函数"
	{21,21,21,21,21,21,9,(*ReadMeterNow)},							//立即抄表
	{22,22,22,22,22,22,10,(*SlectReadMeterChannel)},		/*选择抄表通道*/
	{23,1,1,1,1,1,0,(*MainMenu)},							//预留
	{24,1,1,1,1,1,0,(*MainMenu)},							//预留

	{25,26,27,25,25,25,13,(*ResetSystem)},					//"重启终端确认或者否认函数"
	{26,27,25,26,26,26,14,(*VerInfoShow)},					//"终端版本信息函数"
	{27,25,26,27,27,27,15,(*LCDContrastSet)},				//"液晶对比度函数"

	{28,45,7,28,28,51,1,(*Action_LcdMenu11_25)},  	        //"GPRS参数设置"
	{29,29,29,29,29,29,52,(*Set_IPAndPort)},  		     	//"IP及端口号设置"

	//一级菜单
	{30,31,3,30,30,32,0,(*Action_LcdMenu14)},					//"时间通断面积法"
//	{31,1,30,31,31,33,0,(*Action_LcdMenu15)},					//"选择计量方法"
		{31,35,30,31,31,33,0,(*Action_LcdMenu15)},					//"选择计量方法"

	//二级菜单
	{32,32,32,32,32,34,30,(*Chose_time_data)},				   //"选择时间数据"
	{33,33,33,33,33,33,31,(*Chose_metric_method)},			   //"选择计量方法"
	{34,34,34,34,34,34,32,(*Display_Data_Menu)},			   //"数据显示界面"
	//一级菜单
	{35,1,31,35,35,36,0,(*Action_LcdMenu16)},					//muxiaoqing test "出厂测试"
	
	//二级菜单
	#if 0
	{36,37,43,36,36,44,35,(*Action_LcdMenu16_21)},			  //显示"SD 卡数据写入和读出对比测试"菜单
	{37,38,36,37,37,45,35,(*Action_LcdMenu16_22)},			//"时钟芯片的写入和读出对比测试" 菜单
	{38,39,37,38,38,46,35,(*Action_LcdMenu16_23)},			//"GPRS 指令测试"菜单
	{39,40,38,39,39,47,35,(*Action_LcdMenu16_24)},			//"调试串口命令测试"菜单
	{40,41,39,40,40,48,35,(*Action_LcdMenu16_25)},			//"抄表串口1命令测试"菜单
	{41,42,40,41,41,49,35,(*Action_LcdMenu16_26)},			//"GPRS通讯命令测试"菜单
	{42,43,41,42,42,50,35,(*Action_LcdMenu16_27)},			 //"RS485 通讯1命令测试"菜单
	{43,36,42,43,43,51,35,(*Action_LcdMenu16_28)},			//"RS485 通讯2命令测试"菜单
	#else
	
#ifdef DEMO_APP
    
    {36,37,37,36,36,38,35,(*Action_LcdMenu16_21)},			  //显示"参数设置"菜单
    {37,36,36,37,37,41,35,(*Action_LcdMenu16_22)},			//显示"自动采集" 菜单
#else
	{36,37,37,36,36,38,35,(*Action_LcdMenu16_21)},			  //显示"手动测试"菜单
	{37,36,36,37,37,46,35,(*Action_LcdMenu16_22)},			//显示"自动测试" 菜单
#endif	
	#endif
	//三级菜单
	#if 0
	{44,44,44,44,44,44,36,(*factory_test_method_sd)},			//"SD 卡数据写入和读出对比测试"
	{45,45,45,45,45,45,37,(*factory_test_method_time)},			//"时钟芯片的写入和读出对比测试"
	{46,46,46,46,46,46,38,(*factory_test_method_GPRS)},			//"GPRS 指令测试"
	{47,47,47,47,47,47,39,(*factory_test_method_Debug_RS232_0)},			//"调试串口命令测试"
	{48,48,48,48,48,48,40,(*factory_test_method_ChaoBiao_RS232_1)},			//"抄表串口1命令测试"
	{49,49,49,49,49,49,41,(*factory_test_method_GPRS_TTL_2)},			//"GPRS通讯命令测试"
	{50,50,50,50,50,50,42,(*factory_test_method_Commu_Zigbee_3)},			//"RS485 通讯1命令测试"
	{51,51,51,51,51,51,43,(*factory_test_method_Commu_RS485_4)},			//"RS485 通讯2命令测试"
	#else
#ifdef DEMO_APP
//三级菜单

	{38,39,41,38,38,42,36,(*Action_LcdMenu16_21_31)},			  //"德鲁"
	{39,40,38,39,39,43,36,(*Action_LcdMenu16_21_32)},			//"兰吉尔"
	{40,41,39,40,40,44,36,(*Action_LcdMenu16_21_33)},			//"天罡"
	//{41,42,40,41,41,50,36,(*Action_LcdMenu16_21_34)},			//"调试串口命令测试"菜单
	
    {41,41,41,41,41,41,37,(*ReadMeter_Demo)}, 		//"自动采集"

//四级菜单

    
	{42,42,42,42,42,42,38,(*Set_MeterParam_1)},			//表一的参数设置
	{43,43,43,43,43,43,39,(*Set_MeterParam_2)},			//表二的参数设置
	{44,44,44,44,44,44,40,(*Set_MeterParam_3)},			//表三的参数设置
	//{50,50,50,50,50,50,41,(*factory_test_method_Debug_RS232_0)},			//"调试串口命令测试"

#else	
	//三级菜单

	{38,39,45,38,38,47,36,(*Action_LcdMenu16_21_31)},			  //显示"SD 卡数据写入和读出对比测试"菜单
	{39,40,38,39,39,48,36,(*Action_LcdMenu16_21_32)},			//"时钟芯片的写入和读出对比测试" 菜单
	{40,41,39,40,40,49,36,(*Action_LcdMenu16_21_33)},			//"GPRS 指令测试"菜单
	{41,42,40,41,41,50,36,(*Action_LcdMenu16_21_34)},			//"调试串口命令测试"菜单
	{42,43,41,42,42,51,36,(*Action_LcdMenu16_21_35)},			//"抄表串口1命令测试"菜单
	{43,44,42,43,43,52,36,(*Action_LcdMenu16_21_36)},			//"GPRS通讯命令测试"菜单
	{44,45,43,44,44,53,36,(*Action_LcdMenu16_21_37)},			 //"RS485 通讯1命令测试"菜单
	{45,38,44,45,45,54,36,(*Action_LcdMenu16_21_38)},			//"RS485 通讯2命令测试"菜单
	{46,46,46,46,46,46,37,(*factory_test_method_auto)},			//"自动测试"

//四级菜单

    
	{47,44,44,44,44,47,38,(*factory_test_method_sd)},			//"SD 卡数据写入和读出对比测试"
	{48,48,48,48,48,48,39,(*factory_test_method_time)},			//"时钟芯片的写入和读出对比测试"
	{49,49,49,49,49,48,40,(*factory_test_method_GPRS)},			//"GPRS 指令测试"
	{50,50,50,50,50,50,41,(*factory_test_method_Debug_RS232_0)},			//"调试串口命令测试"
	{51,51,51,51,51,51,42,(*factory_test_method_ChaoBiao_RS232_1)},			//"抄表串口1命令测试"
	{52,52,52,52,52,52,43,(*factory_test_method_GPRS_TTL_2)},			//"GPRS通讯命令测试"
	{53,53,53,53,53,53,44,(*factory_test_method_Commu_Zigbee_3)},			//"RS485 通讯1命令测试"
	{54,54,54,54,54,54,45,(*factory_test_method_Commu_RS485_4)},			//"RS485 通讯2命令测试"
	#endif

	
#endif	


//begin:added by zjjin.
{45,4,28,45,45,46,1,(*Action_LcdMenu11_26)}, 		//"补抄表参数设置"
{46,46,46,46,46,46,45,(*Set_ReplenishPara)},		//"补抄次数及时间间隔设置"
{47,48,48,47,47,49,11,(*ShowMeterFail)}, 			//选中表失败信息查询。
{48,47,47,48,48,50,11,(*ShowValveFail)},			//选中阀失败信息查询。
{49,49,49,49,49,49,47,(*InquiryMeterFailInfo)},			//表失败信息查询。
{50,50,50,50,50,50,48,(*InquiryValveFailInfo)}, 		//阀失败信息查询。
{51,52,52,51,51,53,28,(*SelectAPNSet)}, 			//选择APN设置、IP及端口设置，选中APN设置。
{52,51,51,52,52,29,28,(*SelectIPSet)},		    	//选择APN设置、IP及端口设置，选中IP及端口设置.
{53,53,53,53,53,53,51,(*APNSet)},	











//end:added by zjjin.




};

uint8 KeyFuncIndex = 0;
uint8 KeyMessageFlag = 0;


void test(void)
{
	strDisplayData.user_id = 112;
	strDisplayData.address = 0x45f0;
	strDisplayData.area = 250;
	strDisplayData.open_time = 0x40;//0x12345678;
	strDisplayData.room_temperature = 0x2345;
	strDisplayData.forward_temperature = 0x2345;
	strDisplayData.return_temperature = 0x2345;
	strDisplayData.proportion_energy = 0x234567;
	strDisplayData.total_energy = 0x12345678;
}


/****************************************************************************************************
**	函 数  名 称: FreshIPAPortSet
**	函 数  功 能: 刷新IP和端口设置
**	输 入  参 数: uint8* InputBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshIPAndPortSet(uint8 * InputBuff,uint8 * InputBuff1 )
{
	uint8 i,line,column;


	display_white();
	ShowMenuTop();
	ShowMenuBottom(21);
	ShowLineChar_HZ(1,9,&MenuHZ11_29[2],8);	//IP及端口号设置
	ShowLineChar_HZ(2,0,&MainIP[0],4);	    //主IP:
	ShowLineChar_HZ(4,0,&MainPort[0],6);   //主IP端口:
    ShowPicture(80,22,16,32,&MenuPic_OK[0]);//5行 OK
	line = 3;
	column = 0;
	for(i=0;i<4;i++)
	{
		ShowChar(line,column+(12*i),InputBuff[3*i]);
		ShowChar(line,column+(12*i+3),InputBuff[3*i+1]);
		ShowChar(line,column+(12*i+6),InputBuff[3*i+2]);
	}

	ShowLineChar_HZ(3,9,&Dot[0],1);	//第1个点.
	ShowLineChar_HZ(3,21,&Dot[0],1);//第2个点.
	ShowLineChar_HZ(3,33,&Dot[0],1);//第3个点.
    
	for(i=0;i<5;i++)
	{
		ShowChar(4,24+3*i,InputBuff1[i]);
	}
}
/****************************************************************************************************
**	函 数  名 称: Set_IPAPort
**	函 数  功 能: IP及端口号设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Set_IPAndPort(void)  
{
  	uint8 status;
	uint8 line;
	uint8 line1;
	uint8 line2;
	uint8 column;
	uint8 column1;
	uint8 column2;
	int8 i;
	uint8 ASCIIData[12];
	uint8 ASCIIData1[5];

	uint8 TermAddr[4] = {0};
	uint8 TermAddrReverse[4] = {0x00};
	uint16 TermAddr1[4] = {0};

	uint16 TermPort = 0;
	uint32 TermPort2 = 0;


	SelectInfo TabMenu[] = {{3,0,45},{4,24,15},{5,21,12}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;

	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;

	CPU_SR			cpu_sr;

	OS_ENTER_CRITICAL();
	memcpy(TermAddrReverse, gPARA_UpPara.HostIPAddr, 4);
	TermPort = gPARA_UpPara.HostPort ;
	OS_EXIT_CRITICAL();
	
	//由于存储低字节在前，所以现在将字节顺序调换
	for(i=0; i<4; i++)
	{
	 TermAddr[i] = TermAddrReverse[3-i];
	}
	for(i=0; i<4; i++)
	{
	  TermAddr1[i] = HexToBcd2bit( TermAddr[i] ); //0XAB
	} 	
	//ip地址的
	for(i=0;i<4;i++)
	{
	   	ASCIIData[3*i]   = HexToAscii( TermAddr1[i]>>8 );
		ASCIIData[3*i+1] = HexToAscii((TermAddr1[i]>>4)&0x0f );
		ASCIIData[3*i+2] = HexToAscii( TermAddr1[i]&0x0f );
	}
   
   	//端口号	
	 TermPort2 = HexToBcd4bit( TermPort );  //0XABCD
	 ASCIIData1[0] = HexToAscii(  TermPort2 >>16 );
	 ASCIIData1[1] = HexToAscii( (TermPort2&0x0f000)>>12 );
	 ASCIIData1[2] = HexToAscii( (TermPort2&0x00f00)>>8 );
	 ASCIIData1[3] = HexToAscii( (TermPort2&0x000f0)>>4 );
	 ASCIIData1[4] = HexToAscii( (TermPort2&0x0000f) );
   								  		           
	i = 0;
	line = TabMenu[0].startline;
	column = TabMenu[0].startcolumn;
	line1 = TabMenu[1].startline;
	column1 = TabMenu[1].startcolumn;
	line2 = TabMenu[0].startline;
	column2 = TabMenu[0].startcolumn;

	MaxTabNum = ( sizeof(TabMenu) / sizeof(SelectInfo) ) - 1;//最大下标

	while(OK_Flag == FALSE)
	{
		FreshIPAndPortSet(&ASCIIData[0],&ASCIIData1[0]);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len( TabMenu[TabNum].startline , TabMenu[TabNum].startcolumn , TabMenu[TabNum].columnnum ) ;
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if( TabNum == MaxTabNum )
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 4;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
				default:	
					break;
			}		
		}
		else if(TabNum == 0)
	         {
		       ReverseShowChar(line,column);
			   status = GetKeyStatus();//有待修改为得到键盘的状态函数
			   switch(status)
				{
				   case KEY_UP://向上键
						 if( (i%3 == 0) )
						 {
							  if(ASCIIData[i] == 0x32)
							    {
								   ASCIIData[i] = 0x30;
						    	}
							    else
							    {
							       ASCIIData[i] = ASCIIData[i] + 1;
							    }
						    
						 }
						 else
						    if(ASCIIData[i] == 0x39)
						    {
							   ASCIIData[i] = 0x30;
					    	}
						    else
						    {
						       ASCIIData[i] = ASCIIData[i] + 1;
						    }
						 break;
					case KEY_DOWN://向下键
						 if( (i%3 == 0) )
							 {
								  if(ASCIIData[i] == 0x30)
								    {
									   ASCIIData[i] = 0x32;
							    	}
								    else
								    {
								       ASCIIData[i] = ASCIIData[i] - 1;
								    }
							    
							 }
						 else
							if(ASCIIData[i] == 0x30)
							{
								ASCIIData[i] = 0x39;
							}
							else
							{
								ASCIIData[i] = ASCIIData[i] - 1;
							}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line,column,ASCIIData[i]);
						if(i > 0)
						{
							if( (i%3) == 0 )
							{
								column = column - 6;
							}
							else
							{
								column = column - 3;
							}
							i = i - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line,column,ASCIIData[i]);
						if(i < 11)
						{
							if( (i%3) == 2 )
							{
								column = column + 6;
							}
							else
							{
								column = column + 3;
							}
							i = i + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						//返回原始列，下次进入时可以从头开始
						i = 0;
						line = TabMenu[0].startline;
						column = TabMenu[0].startcolumn;
						line1 = TabMenu[1].startline;
						column1 = TabMenu[1].startcolumn;
						line2 = TabMenu[2].startline;
						column2 = TabMenu[2].startcolumn;
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 4;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					default:	
						break;
			    }
		    }//end else if(TabNum == 0)
			else if(TabNum == 1)
		         {
			       ReverseShowChar(line1,column1);
				   status = GetKeyStatus();//有待修改为得到键盘的状态函数
				   switch(status)
					{
					   case KEY_UP://向上键
						    if( (i == 0) )
							 {
								  if(ASCIIData1[i] == 0x36)
								    {
									   ASCIIData1[i] = 0x30;
							    	}
								    else
								    {
								       ASCIIData1[i] = ASCIIData1[i] + 1;
								    }
							    
							 }
						   else
								if(ASCIIData1[i] == 0x39)
							    {
								   ASCIIData1[i] = 0x30;
						    	}
							    else
							    {
							       ASCIIData1[i] = ASCIIData1[i] + 1;
							    }
						    break;
						case KEY_DOWN://向下键
							if( (i == 0) )
								 {
									  if(ASCIIData1[i] == 0x30)
									    {
										   ASCIIData1[i] = 0x36;
								    	}
									    else
									    {
									       ASCIIData1[i] = ASCIIData1[i] - 1;
									    }
								    
								 }
							 else
								if(ASCIIData1[i] == 0x30)
								{
									ASCIIData1[i] = 0x39;
								}
								else
								{
									ASCIIData1[i] = ASCIIData1[i] - 1;
								}	
							break;			
						case KEY_LEFT://向左键
							ShowChar(line1,column1,ASCIIData1[i]);
							if(i > 0)
							{
								i = i - 1;
								column1 = column1 - 3;
							}
							break;
						case KEY_RIGHT://向右键
							ShowChar(line1,column1,ASCIIData1[i]);
							if(i < 4)
							{
								i = i + 1;
								column1 = column1 + 3;
							}
							break;
						case KEY_ENTER://回车键
							Tab_Flag = (!Tab_Flag);
							//返回原始列，下次进入时可以从头开始
							i = 0;
							line = TabMenu[0].startline;
							column = TabMenu[0].startcolumn;
							line1 = TabMenu[1].startline;
							column1 = TabMenu[1].startcolumn;
							TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
							line2 = TabMenu[2].startline;
							line2 = line2;/*防止编译警告*/
						    column2 = TabMenu[2].startcolumn;
							column2 = column2;	/*防止编译警告*/
							break;
						case KEY_EXIT://返回键
							KeyFuncIndex = 4;
							(*KeyTab[KeyFuncIndex].CurrentOperate)();
							return;			
						default:	
							break;
				    }
			    }
	}//end while
	
    //寄存器设置
   	memset(TermAddr1, 0x00, 8);
	memset(TermAddr, 0x00, 4);
	TermPort2 =	0;
	TermPort = 0;

	for(i=0;i<4;i++)
	{		
	  TermAddr1[i] = ((ASCIIData[3*i]-0x30)<<8) + (((ASCIIData[3*i+1]-0x30)<<4)&0xf0) + ((ASCIIData[3*i+2]-0x30)&0x0F);
	}
	for(i=0; i<4; i++)
	{
	  TermAddr[i] = Uint16BCDToHex1( TermAddr1[i] ); //0XAB
	}
	for(i=0; i<4; i++)
	{
	 TermAddrReverse[i] = TermAddr[3-i];
	}

	//不要换行，否则结果不一样
	TermPort2 =((ASCIIData1[0]-0x30)<<16)+(((ASCIIData1[1]-0x30)<<12)&0x0f000)+(((ASCIIData1[2]-0x30)<<8)&0x00f00)+(((ASCIIData1[3]-0x30)<<4)&0x000f0)+((ASCIIData1[4]-0x30)&0x0000f); 
	TermPort = BcdToHex_16bit1( TermPort2 );

    if(TermPort2 > 0x65535)  
	 {
	   ShowLineChar_HZ(6,5,&PortErr[0],7);	//端口设置ERR
	   OSTimeDly(2000);
	   KeyFuncIndex = 29;
	   (*KeyTab[KeyFuncIndex].CurrentOperate)();
	 }

	  for(i=0;i<4;i++)
	  {		
	    if( TermAddr1[i]>0x0255 )
	     { 
		   ShowLineChar_HZ(7,5,&IPErr[0],7);	//ip设置ERR
		   OSTimeDly(1500);
		   KeyFuncIndex = 29;
	       (*KeyTab[KeyFuncIndex].CurrentOperate)();
         }	  
    }
	 
	SetIP( TermAddrReverse );//IP保存成功	 
	SetHostPort( TermPort );//Port保存成功	

  	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;

}


/****************************************************************************************************
**	函 数  名 称: SelectAPNSet
**	函 数  功 能: 选择APN。
**	输 入  参 数: 
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void SelectAPNSet(void)
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(2,3,&ZCApnSet[0],5);
	ShowLineChar_HZ(3,3,&ZCIPAndPortSet[0],7);
	ReverseShowLine(2);

}


/****************************************************************************************************
**	函 数  名 称: SelectIPSet
**	函 数  功 能: 选择IP及端口设置。
**	输 入  参 数: 
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void SelectIPSet(void)
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(2,3,&ZCApnSet[0],5);
	ShowLineChar_HZ(3,3,&ZCIPAndPortSet[0],7);
	ReverseShowLine(3);

}






/****************************************************************************************************
**	函 数  名 称: FreshAPNPara
**	函 数  功 能: 刷新APN参数显示。
**	输 入  参 数: uint8* InputBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshAPNPara(uint8 *InputBuff)
{
	uint8 i,line,column;


	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,1,&ZCAPN1[0],7);
	ShowLineChar_HZ(2,1,&ZCAPN2[0],11);
	ShowLineChar_HZ(3,1,&ZCAPN3[0],12);
	ShowLineChar_HZ(4,1,&ZCSelectBySN[0],7);
	ShowLineChar_HZ(5,1,&ZCCueRestart[0],7);

	ShowPicture(128,22,16,32,&MenuPic_OK[0]);//8行 OK



    
	line = 7;
	column = 25;

	ShowChar(line,column,*InputBuff);

}


/****************************************************************************************************
**	函 数  名 称: APNSet
**	函 数  功 能: 选择APN接入点名称。
**	输 入  参 数: 
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 选择确认后将重启集中器,按照新APN设置网络。
*****************************************************************************************************/
void APNSet(void)
{
	uint8 err = 0;
 	uint8 line1;
	uint8 column1;
	uint8 lu8ASCIIData;
	uint8 status;

	SelectInfo TabMenu[] = {{7,25,3},{8,21,12}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 1;

	GprsAPNSelect ApnSet;

	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;

	CPU_SR			cpu_sr;


	OSMutexPend (FlashMutex,0,&err);
	SDReadData("/GprsAPNSelect", &ApnSet, sizeof(GprsAPNSelect), 0);
	OSMutexPost (FlashMutex);

	line1 = TabMenu[0].startline;
	column1 = TabMenu[0].startcolumn;

	if(ApnSet.WriteFlag == 0xAA55)
		lu8ASCIIData   = HexToAscii(ApnSet.GprsAPNSet);
	else
		lu8ASCIIData   = HexToAscii(0);

   
	while(OK_Flag == FALSE)
	{
		FreshAPNPara(&lu8ASCIIData);
				
		if(Tab_Flag == TRUE){
			ReverseShowLine_len( TabMenu[TabNum].startline , TabMenu[TabNum].startcolumn , TabMenu[TabNum].columnnum ) ;
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)	{
						TabNum = MaxTabNum;
					}
					else{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)	{
						TabNum = 0;
					}
					else{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if( TabNum == MaxTabNum ){
						OK_Flag = TRUE;
					}
					else{
						Tab_Flag = FALSE;
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 4;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
				default:	
					break;
			}		
		}
		else if(TabNum == 0)
	         {
		       ReverseShowChar(line1,column1);
			   status = GetKeyStatus();//有待修改为得到键盘的状态函数
			   switch(status)
				{
				   case KEY_UP://向上键
						lu8ASCIIData = lu8ASCIIData + 1;   //补抄次数范围0-9.
				   		if(lu8ASCIIData > 0x39)
							lu8ASCIIData = 0x30;
						 break;
					case KEY_DOWN://向下键
						lu8ASCIIData = lu8ASCIIData - 1;
				   		if(lu8ASCIIData < 0x30)
							lu8ASCIIData = 0x39;
						 break;		
					case KEY_LEFT://向左键
		
						break;
					case KEY_RIGHT://向右键
		
						break;
					case KEY_ENTER://回车键
						Tab_Flag = TRUE;
						//返回原始列，下次进入时可以从头开始
						line1 = TabMenu[0].startline;
						column1 = TabMenu[0].startcolumn;

						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 4;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					default:	
						break;
			    }
		    }//end else if(TabNum == 0)
		}


	    //寄存器设置
	    ApnSet.GprsAPNSet = lu8ASCIIData - 0x30;
		

		//范围限定防错检查。
		if(ApnSet.GprsAPNSet > SUPPORT_APN_NUM)
			ApnSet.GprsAPNSet = 0;

		ApnSet.WriteFlag = 0xAA55;


//存储到SD卡中。
	OSMutexPend (FlashMutex,0,&err);
	err = SDSaveData("/GprsAPNSelect", &ApnSet, sizeof(ApnSet), 0);
	OSMutexPost (FlashMutex);

	OSTimeDlyHMSM(0,0,2,0);


  	//=============重启=============================
	NVIC_SETFAULTMASK();
	NVIC_GenerateSystemReset();




}



/****************************************************************************************************
**	函 数  名 称: FreshReplenishPara
**	函 数  功 能: 刷新补抄表参数显示。
**	输 入  参 数: uint8* InputBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshReplenishPara(uint8 *InputBuff,uint8 *InputBuff1,uint8 *InputBuff2,uint8 *InputBuff3)
{
	uint8 i,line,column;
//	uint8 lu8qian = 0;
//	uint8 lu8bai = 0;
//	uint8 lu8shi = 0;
//	uint8 lu8ge = 0;

	extern uint16 ZCBuChaoBiao[6];  //补抄表参数设置。
extern uint16 ZCBiaoBuChaoCiShu[6];  //表补抄次数。
extern uint16 ZCBiaoJianGe[8];  //表时间间隔(ms).
extern uint16 ZCFaBuChaoCiShu[6];  //阀补抄次数。
extern uint16 ZCFaJianGe[8];  //阀时间间隔(ms).

	display_white();
	ShowMenuTop();
	ShowMenuBottom(21);
	ShowLineChar_HZ(1,11,&ZCBuChaoBiao[0],6);	//1行，补抄参数设置。
	ShowLineChar_HZ(2,0,ZCBiaoBuChaoCiShu,6);	    //2行，补抄表次数:
	ShowLineChar_HZ(3,0,ZCBiaoJianGe,8);   //3行，表时间间隔(ms):
	ShowLineChar_HZ(4,0,ZCFaBuChaoCiShu,6);	    //4行，补抄阀次数:
	ShowLineChar_HZ(5,0,ZCFaJianGe,8);   //5行，阀时间间隔(ms):
	
    ShowPicture(96,22,16,32,&MenuPic_OK[0]);//6行 OK
    
	line = 2;
	column = 30;

	ShowChar(line,column,*InputBuff);

	line = 3;
	column = 31;
	for(i=0;i<4;i++){
		ShowChar(line,column,InputBuff1[i]);
		column += 3;
	}

	line = 4;
	column = 30;

	ShowChar(line,column,*InputBuff2);

	line = 5;
	column = 31;
	for(i=0;i<4;i++){
		ShowChar(line,column,InputBuff3[i]);
		column += 3;
	}
	

}


/****************************************************************************************************
**	函 数  名 称: Set_ReplenishPara
**	函 数  功 能: 补抄表次数及时间间隔设置。
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Set_ReplenishPara(void)  
{
	uint8 err = 0;
  	uint8 status;
	uint8 line1;
	uint8 line2;
	uint8 line3;
	uint8 line4;
	uint8 column1;
	uint8 column2;
	uint8 column3;
	uint8 column4;

	int8 i = 0;
	uint8 lu8ASCIIData;
	uint8 lu8ASCIIData1[4];
	uint8 lu8ASCIIData2;
	uint8 lu8ASCIIData3[4];

	SelectInfo TabMenu[] = {{2,30,3},{3,31,12},{4,30,3},{5,31,12},{6,21,12}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 4;

	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;

	CPU_SR			cpu_sr;


	ReplenishReadMeterSaveType ReplenishPara;

	OS_ENTER_CRITICAL();
	ReplenishPara = gPARA_ReplenishPara;
	OS_EXIT_CRITICAL();

	line1 = TabMenu[0].startline;
	column1 = TabMenu[0].startcolumn;
	line2 = TabMenu[1].startline;
	column2 = TabMenu[1].startcolumn;
	line3 = TabMenu[2].startline;
	column3 = TabMenu[2].startcolumn;
	line4 = TabMenu[3].startline;
	column4 = TabMenu[3].startcolumn;



	lu8ASCIIData   = HexToAscii(ReplenishPara.MeterReplenishTimes);
	lu8ASCIIData1[0] = HexToAscii(ReplenishPara.MeterInterval / 1000);
	lu8ASCIIData1[1] = HexToAscii((ReplenishPara.MeterInterval%1000) / 100);
	lu8ASCIIData1[2] = HexToAscii((ReplenishPara.MeterInterval%100) / 10);
	lu8ASCIIData1[3] = HexToAscii(ReplenishPara.MeterInterval%10);
	lu8ASCIIData2   = HexToAscii(ReplenishPara.ValveReplenishTimes);
	lu8ASCIIData3[0] = HexToAscii(ReplenishPara.ValveInterval / 1000);
	lu8ASCIIData3[1] = HexToAscii((ReplenishPara.ValveInterval%1000) / 100);
	lu8ASCIIData3[2] = HexToAscii((ReplenishPara.ValveInterval%100) / 10);
	lu8ASCIIData3[3] = HexToAscii(ReplenishPara.ValveInterval%10);
	
   
	while(OK_Flag == FALSE)
	{
		FreshReplenishPara(&lu8ASCIIData,lu8ASCIIData1,&lu8ASCIIData2,lu8ASCIIData3);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len( TabMenu[TabNum].startline , TabMenu[TabNum].startcolumn , TabMenu[TabNum].columnnum ) ;
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)	{
						TabNum = MaxTabNum;
					}
					else{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)	{
						TabNum = 0;
					}
					else{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if( TabNum == MaxTabNum ){
						OK_Flag = TRUE;
					}
					else{
						//Tab_Flag = (!Tab_Flag);
						i = 0;
						Tab_Flag = FALSE;
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 4;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
				default:	
					break;
			}		
		}
		else if(TabNum == 0)
	         {
		       ReverseShowChar(line1,column1);
			   status = GetKeyStatus();//有待修改为得到键盘的状态函数
			   switch(status)
				{
				   case KEY_UP://向上键
						lu8ASCIIData = lu8ASCIIData + 1;   //补抄次数范围0-9.
				   		if(lu8ASCIIData > 0x39)
							lu8ASCIIData = 0x30;
						 break;
					case KEY_DOWN://向下键
						lu8ASCIIData = lu8ASCIIData - 1;
				   		if(lu8ASCIIData < 0x30)
							lu8ASCIIData = 0x39;
						 break;		
					case KEY_LEFT://向左键
		
						break;
					case KEY_RIGHT://向右键
		
						break;
					case KEY_ENTER://回车键
						Tab_Flag = TRUE;
						//返回原始列，下次进入时可以从头开始
						i = 0;
						line1 = TabMenu[0].startline;
						column1 = TabMenu[0].startcolumn;
						line2 = TabMenu[1].startline;
						column2 = TabMenu[1].startcolumn;
						line3 = TabMenu[2].startline;
						column3 = TabMenu[2].startcolumn;
						line4 = TabMenu[3].startline;
						column4 = TabMenu[3].startcolumn;


						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 4;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					default:	
						break;
			    }
		    }//end else if(TabNum == 0)
			else if(TabNum == 1)
		         {
			       ReverseShowChar(line2,column2);
				   status = GetKeyStatus();//有待修改为得到键盘的状态函数
				   switch(status)
					{
					   case KEY_UP://向上键
							if(lu8ASCIIData1[i] == 0x39){
								   lu8ASCIIData1[i] = 0x30;
						    	}
							    else  {
							       lu8ASCIIData1[i] = lu8ASCIIData1[i] + 1;
							    }
						    break;
						case KEY_DOWN://向下键
							if(lu8ASCIIData1[i] == 0x30){
									lu8ASCIIData1[i] = 0x39;
								}
								else{
									lu8ASCIIData1[i] = lu8ASCIIData1[i] - 1;
								}	
							break;			
						case KEY_LEFT://向左键
							if(i > 0){
								i = i - 1;
								column2 = column2 - 3;
							}
							ShowChar(line2,column2,lu8ASCIIData1[i]);
							break;
						case KEY_RIGHT://向右键
							if(i < 3){
								i = i + 1;
								column2 = column2 + 3;
							}
							ShowChar(line2,column2,lu8ASCIIData1[i]);
							break;
						case KEY_ENTER://回车键
							Tab_Flag = TRUE;
							//返回原始列，下次进入时可以从头开始
							i = 0;
							line1 = TabMenu[0].startline;
							column1 = TabMenu[0].startcolumn;
							line2 = TabMenu[1].startline;
							column2 = TabMenu[1].startcolumn;
							line3 = TabMenu[2].startline;
							column3 = TabMenu[2].startcolumn;
							line4 = TabMenu[3].startline;
							column4 = TabMenu[3].startcolumn;

							TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置

							break;
						case KEY_EXIT://返回键
							KeyFuncIndex = 4;
							(*KeyTab[KeyFuncIndex].CurrentOperate)();
							return;			
						default:	
							break;
				    }
			    }
			else if(TabNum == 2)   {
		       ReverseShowChar(line3,column3);
			   status = GetKeyStatus();//有待修改为得到键盘的状态函数
			   switch(status)
				{
				   case KEY_UP://向上键
						lu8ASCIIData2 = lu8ASCIIData2 + 1;   //补抄次数范围0-9.
				   		if(lu8ASCIIData2 > 0x39)
							lu8ASCIIData2 = 0x30;
						 break;
					case KEY_DOWN://向下键
						lu8ASCIIData2 = lu8ASCIIData2 - 1;
				   		if(lu8ASCIIData2 < 0x30)
							lu8ASCIIData2 = 0x39;
						 break;		
					case KEY_LEFT://向左键
		
						break;
					case KEY_RIGHT://向右键
		
						break;
					case KEY_ENTER://回车键
						Tab_Flag = TRUE;
						//返回原始列，下次进入时可以从头开始
						i = 0;
						line1 = TabMenu[0].startline;
						column1 = TabMenu[0].startcolumn;
						line2 = TabMenu[1].startline;
						column2 = TabMenu[1].startcolumn;
						line3 = TabMenu[2].startline;
						column3 = TabMenu[2].startcolumn;
						line4 = TabMenu[3].startline;
						column4 = TabMenu[3].startcolumn;


						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 4;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					default:	
						break;
			    }
		    }
			else if(TabNum == 3)
		         {
			       ReverseShowChar(line4,column4);
				   status = GetKeyStatus();//有待修改为得到键盘的状态函数
				   switch(status)
					{
					   case KEY_UP://向上键
							if(lu8ASCIIData3[i] == 0x39){
								   lu8ASCIIData3[i] = 0x30;
						    	}
							    else  {
							       lu8ASCIIData3[i] = lu8ASCIIData3[i] + 1;
							    }
						    break;
						case KEY_DOWN://向下键
							if(lu8ASCIIData3[i] == 0x30){
									lu8ASCIIData3[i] = 0x39;
								}
								else{
									lu8ASCIIData3[i] = lu8ASCIIData3[i] - 1;
								}	
							break;			
						case KEY_LEFT://向左键
							if(i > 0){
								i = i - 1;
								column4 = column4 - 3;
							}
							ShowChar(line4,column4,lu8ASCIIData3[i]);
							break;
						case KEY_RIGHT://向右键
							if(i < 3){
								i = i + 1;
								column4 = column4 + 3;
							}
							ShowChar(line4,column4,lu8ASCIIData3[i]);
							break;
						case KEY_ENTER://回车键
							Tab_Flag = TRUE;
							//返回原始列，下次进入时可以从头开始
							i = 0;
							line1 = TabMenu[0].startline;
							column1 = TabMenu[0].startcolumn;
							line2 = TabMenu[1].startline;
							column2 = TabMenu[1].startcolumn;
							line3 = TabMenu[2].startline;
							column3 = TabMenu[2].startcolumn;
							line4 = TabMenu[3].startline;
							column4 = TabMenu[3].startcolumn;

							TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置

							break;
						case KEY_EXIT://返回键
							KeyFuncIndex = 4;
							(*KeyTab[KeyFuncIndex].CurrentOperate)();
							return;			
						default:	
							break;
				    }
			    }
	}//end while

	
    //寄存器设置
	ReplenishPara.MeterReplenishTimes = lu8ASCIIData - 0x30;
	ReplenishPara.MeterInterval = (lu8ASCIIData1[0] - 0x30) * 1000;
	ReplenishPara.MeterInterval += (lu8ASCIIData1[1] - 0x30) * 100;
	ReplenishPara.MeterInterval += (lu8ASCIIData1[2] - 0x30) * 10;
	ReplenishPara.MeterInterval += lu8ASCIIData1[3] - 0x30;

	ReplenishPara.ValveReplenishTimes = lu8ASCIIData2 - 0x30;
	ReplenishPara.ValveInterval = (lu8ASCIIData3[0] - 0x30) * 1000;
	ReplenishPara.ValveInterval += (lu8ASCIIData3[1] - 0x30) * 100;
	ReplenishPara.ValveInterval += (lu8ASCIIData3[2] - 0x30) * 10;
	ReplenishPara.ValveInterval += lu8ASCIIData3[3] - 0x30;

	//范围限定防错检查。
	if(ReplenishPara.MeterReplenishTimes > 9)
		ReplenishPara.MeterReplenishTimes = 2;
	if((ReplenishPara.MeterInterval>9999) || (ReplenishPara.MeterInterval<100))
		ReplenishPara.MeterInterval = 200;

	if(ReplenishPara.ValveReplenishTimes > 9)
		ReplenishPara.ValveReplenishTimes = 2;
	if((ReplenishPara.ValveInterval>9999) || (ReplenishPara.ValveInterval<100))
		ReplenishPara.ValveInterval = 500;

	ReplenishPara.WriteFlag = 0xaa55;
	gPARA_ReplenishPara = ReplenishPara;  //赋值给全局变量。


//存储到SD卡中。
	OSMutexPend (FlashMutex,0,&err);
	err = SDSaveData("/REPLENISH_PARA", &ReplenishPara, sizeof(ReplenishPara), 0);
	OSMutexPost (FlashMutex);


  	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;

}


/****************************************************************************************************
**	函 数  名 称: LCD_Initial
**	函 数  功 能: 液晶初始化，补充函数lcd_initial()中对液晶对比度的设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void LCD_Initial()
{
	lcd_initial();
    if(gStore_CommPara.LCDContrastValue < 170 || gStore_CommPara.LCDContrastValue > 220)
    {
    	gStore_CommPara.LCDContrastValue = LCD_CON_VALUE;
    }
    
    SetContrast(gStore_CommPara.LCDContrastValue);
    //Set_SelfDefinePara();
    return;
}

/****************************************************************************************************
**	函 数  名 称: RegisterMenu
**	函 数  功 能: 集中器注册界面
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void RegisterMenu(void)
{
	char path[17]="/2014/01/01/1530";
	display_white();
	ShowMenuTop();
	ShowMenuBottom(0);
	//ShowPicture(32,0,16,160,&RegisterPic[0]);
	ShowLineChar_HZ(3, 11, RegistString, 7);
	ShowSysDate(4,12,DATETYPE_CH);
	ShowSysTime(5,15,1);

	str_check_data.user_id = 01;
	str_check_data.read_num = 01;
	memcpy(str_check_data.time_path, path, 16);
	//test();
}
/****************************************************************************************************
**	函 数  名 称:  NoRotate
**	函 数  功 能: 无循环
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void NoRotate(void)
{
	display_white();
	ShowMenuTop();

	ShowPicture(64,0,16,160,&RegisterPic[0]);
	ShowSysDate(6,12,DATETYPE_CH);
	ShowSysTime(7,15,1);
}

/****************************************************************************************************
**	函 数  名 称: ShowMainMenu
**	函 数  功 能: 一级菜单的动作函数
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ShowMainMenu(void)
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(2,3,&MenuHZ11[0],7);//参数设置与查看
	ShowLineChar_HZ(3,3,&MenuHZ12[0],9);//抄表状态查看与设置
	ShowLineChar_HZ(4,3,&MenuHZ13[0],8);//集中器管理与维护
	//ShowLineChar_HZ(5,3,&MenuHZ14[0],7);//时间通断面积法
	//ShowLineChar_HZ(6,3,&MenuHZ15[0],6);//选择计量方法
//#ifdef DEMO_APP
//	
//	ShowLineChar_HZ(7,3,&MenuHZ16[0],4);//"热表抄表"采集功能演示muxiaoqing 20140718
//#else
//    ShowLineChar_HZ(7,3,&MenuHZ16[0],4);//维护 实际应该写成出厂测试muxiaoqing 20140328
//
//#endif
}
/****************************************************************************************************
**	函 数  名 称: MainMenu
**	函 数  功 能: 液晶主菜单
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void MainMenu(void)
{
	ShowMainMenu();
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu11
**	函 数  功 能: 1.参数设置与查看
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu11(void)
{
	ShowMainMenu();
	ReverseShowLine(2);
	
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu12
**	函 数  功 能: 2.抄表状态查看
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu12(void)
{
	ShowMainMenu();
	ReverseShowLine(3);
	
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu13
**	函 数  功 能: 3.集中器管理与维护
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu13(void)
{
	ShowMainMenu();
	ReverseShowLine(4);
}
/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu14
**	函 数  功 能: 4.时间通断面积法
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu14(void)
{
	ShowMainMenu();
	ReverseShowLine(5);
}
/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu15
**	函 数  功 能: 5.选择计量方法
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu15(void)
{
	ShowMainMenu();
	ReverseShowLine(6);
}
/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu16
**	函 数  功 能: 5.出厂测试
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/

void Action_LcdMenu16(void)
{
	ShowMainMenu();
	ReverseShowLine(7);
}

//二级菜单
/****************************************************************************************************
**	函 数  名 称: FreshTimeInputFun
**	函 数  功 能: 刷新时间
**	输 入  参 数: uint8* date
**				  uint8* time
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshTimeInputFun(uint8* date,uint8* time)
{
	uint8 line,column;
	line = 2;
	column = 0;	
	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,0,&InputTime[0],8);
	ShowPicture(64,22,16,32,&MenuPic_OK[0]);//OK 4行
		
	ShowIniDate(line,column,&date[0]);
	ShowIniTime(line + 1,column,&time[0]);
}

/****************************************************************************************************
**	函 数  名 称: TimeInputFun
**	函 数  功 能: 时间输入
**	输 入  参 数: uint8* DateBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 TimeInputFun(uint8* DateBuff)
{
	uint8 rt;
	uint8 status,err;
	uint8 line1,line2;
	uint8 column1,column2;
	uint8 i,j;
	uint8 datebuf[6];
	uint8 ASCIIData[12];
	uint8 HexData[6];
	
	SelectInfo TabMenu[] = {{2,0,33},{3,0,33},{4,22,11}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
		
	//读当前时间
	//UserReadDateTime(&datebuf[0]);//秒分时日月年
	ReadDateTime(&datebuf[0]);
	datebuf[4] = datebuf[4]&0x1F;//去掉星期
	for(i=0;i<6;i++)
	{
		ASCIIData[2*i] = HexToAscii(datebuf[5-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(datebuf[5-i]&0x0f);
	}
	//显示当前时间
	FreshTimeInputFun(&ASCIIData[0],&ASCIIData[6]);
	i = 0;
	j = 6;
	line1 = TabMenu[0].startline;//2
	line2 = TabMenu[1].startline;//3
	column1 = TabMenu[0].startcolumn;//0
	column2 = TabMenu[1].startcolumn;//0
	
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
		FreshTimeInputFun(&ASCIIData[0],&ASCIIData[6]);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
						rt = KEY_ENTER;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					rt = KEY_EXIT;
					return rt;			
					
				default:	
					break;
			}		
		}
		else
		{
			if(TabNum == 0)
			{
				ReverseShowChar(line1,column1);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(ASCIIData[i] == 0x39)
						{
							ASCIIData[i] = 0x30;
						}
						else
						{
							ASCIIData[i] = ASCIIData[i] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(ASCIIData[i] == 0x30)
						{
							ASCIIData[i] = 0x39;
						}
						else
						{
							ASCIIData[i] = ASCIIData[i] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line1,column1,ASCIIData[i]);
						if(i > 0)
						{
							if(i%2)
							{
								column1 = column1 - 3;
							}
							else
							{
								column1 = column1 - 8;
							}
							i = i - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line1,column1,ASCIIData[i]);
						if(i < 5)
						{
							if(i%2)
							{
								column1 = column1 + 8;
							}
							else
							{
								column1 = column1 + 3;
							}
							i = i + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						j = 6;
						line1 = TabMenu[0].startline;//2
						line2 = TabMenu[1].startline;//3
						column1 = TabMenu[0].startcolumn;//0
						column2 = TabMenu[1].startcolumn;//0					
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						rt = KEY_EXIT;
						return rt;			
					
					default:	
						break;
				}
			}//年月日设置
				
			else if(TabNum == 1)
			{
				ReverseShowChar(line2,column2);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(ASCIIData[j] == 0x39)
						{
							ASCIIData[j] = 0x30;
						}
						else
						{
							ASCIIData[j] = ASCIIData[j] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(ASCIIData[j] == 0x30)
						{
							ASCIIData[j] = 0x39;
						}
						else
						{
							ASCIIData[j] = ASCIIData[j] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line2,column2,ASCIIData[j]);
						if(j > 6)
						{
							if(j%2)
							{
								column2 = column2 - 3;
							}
							else
							{
								column2 = column2 - 8;
							}
							j = j - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line2,column2,ASCIIData[j]);
						if(j < 11)
						{
							if(j%2)
							{
								column2 = column2 + 8;
							}
							else
							{
								column2 = column2 + 3;
							}
							j = j + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						j = 6;
						line1 = TabMenu[0].startline;//2
						line2 = TabMenu[1].startline;//3
						column1 = TabMenu[0].startcolumn;//0
						column2 = TabMenu[1].startcolumn;//0					
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						rt = KEY_EXIT;
						return rt;			
					
					default:	
						break;
				}				
			}//end if(TabNum == 1) 时分秒 设置
		}//end if(Tab_Flag == TRUE)
	}//end while	
	
	//==========================================
	//此处ascii时间转换为HEXBCD
	for(i=0;i<6;i++)//将ASC码转换为BCD，并按照秒---年的顺序存放
	{
		HexData[5-i] = ((ASCIIData[2*i]-0x30)<<4) + (ASCIIData[2*i+1]-0x30);
	}
	//HexData[4] = HexData[4]|0x20;
	err = TimeCheck(&HexData[0]);
	if(err != 0)//不合要求
	{
		if(err == 1)///日期不合要求
		{
			ShowMenuBottom(7);
		}
		else if(err == 2)//时间不合法
		{
			ShowMenuBottom(8);
		}
		OSTimeDly(OS_TICKS_PER_SEC*2);
		rt = KEY_EXIT;
	}
	else
	{
		memcpy(DateBuff,HexData,6);
	}
	return rt;
}

/****************************************************************************************************
**	函 数  名 称: FreshZigbeeSet
**	函 数  功 能: 刷新zigbee设置
**	输 入  参 数: uint8* InputBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshZigbeeSet(uint8* InputBuff)
{
	uint8 i,line,column;

	LOG_assert_param( InputBuff == NULL );
		
	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(21);
	ShowLineChar_HZ(1,8,(const uint16 *)&Menu_ConfigType[3][0],10);//1行 zigbee主站地址:
	ShowPicture(48,22,16,32,&MenuPic_OK[0]);//3行 OK
	line = 2;
	column = 20;
	for(i=0;i<4;i++)
	{
		ShowChar(line,column+3*i,InputBuff[i]);
	}	
	return;
}
/****************************************************************************************************
**	函 数  名 称: FreshSlectReadMeterChannel
**	函 数  功 能: 刷新热表通道设置
**	输 入  参 数: uint8* InputBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshSlectReadMeterChannel(uint8* InputBuff)
{
	uint8 line,column;

	LOG_assert_param( InputBuff == NULL );
		
	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(21);
	ShowLineChar_HZ(1,8,(const uint16 *)&MenuHZ12_23[0],6);//1行 zigbee主站地址:
	ShowPicture(48,22,16,32,&MenuPic_OK[0]);//3行 OK
	line = 2;
	column = 20;
	
	ShowChar(line,column,InputBuff[0]);
	
	return;
}
/****************************************************************************************************
**	函 数  名 称: ZigbeeSet
**	函 数  功 能: zigbee主站地址设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ZigbeeSet(void)
{	
	uint8 status;
	uint8 line;
	uint8 column;
	uint8 column1;
	uint8 line1;
	int8 i;
	uint8 ASCIIData[4];
	uint8 ZigbeeAddr[6] = {0};
	uint8 ZigbeeAddrReverse[6] = {0x00};
//	uint8 datalen;
	
	SelectInfo TabMenu[] = {{2,20,12},{3,22,11}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	memcpy(ZigbeeAddrReverse, gPARA_TermPara.HostAddr, 6);
	OS_EXIT_CRITICAL();
	
	//由于存储低字节在前，所以现在将字节顺序调换
	for(i=0; i<6; i++)
	{
		ZigbeeAddr[i] = ZigbeeAddrReverse[5-i];
	}
	
	//取调换顺序后的后2字节有效
	for(i=0;i<2;i++)
	{
		ASCIIData[2*i] = HexToAscii(ZigbeeAddr[4+i]>>4);
		ASCIIData[2*i+1] = HexToAscii(ZigbeeAddr[4+i]&0x0f);
	}
	
	i = 0;
	line = TabMenu[0].startline;
	column = TabMenu[0].startcolumn;
	line1 = TabMenu[1].startline;
	column1 = TabMenu[1].startcolumn;
	
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
		FreshZigbeeSet(&ASCIIData[0]);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 4;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else if(TabNum == 0)
		{
			ReverseShowChar(line,column);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIData[i] == 0x39)
					{
						ASCIIData[i] = 0x30;
					}
					else
					{
						ASCIIData[i] = ASCIIData[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if(ASCIIData[i] == 0x30)
					{
						ASCIIData[i] = 0x39;
					}
					else
					{
						ASCIIData[i] = ASCIIData[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line,column,ASCIIData[i]);
					if(i > 0)
					{
						i = i - 1;
						column = column - 3;
					}
					break;
				case KEY_RIGHT://向右键
					ShowChar(line,column,ASCIIData[i]);
					if(i < 3)
					{
						i = i + 1;
						column = column + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					line1 = line1;/*防止编译警告*/
					column1 = TabMenu[1].startcolumn;
					column1 = column1;/*防止编译警告*/
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 4;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}//end else if(TabNum == 0)

	}//end while
	memset(ZigbeeAddr, 0x00, 6);
	for(i=0;i<2;i++)
	{
		ZigbeeAddr[1-i] =  ((ASCIIData[2*i]-0x30)<<4) + ((ASCIIData[2*i+1]-0x30)&0x0F);
	}
	SetHostAddr(ZigbeeAddr);
	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;
}

// 参数设置与查看 下级菜单
/****************************************************************************************************
**	函 数  名 称: ShowMenu_ParaSet
**	函 数  功 能: 目录参数设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ShowMenu_ParaSet(void)
{
	ShowLineChar_HZ(1,10,&MenuHZ11[0],7);
	ShowLineChar_HZ(2,3,&MenuHZ11_22[2],6);
	ShowLineChar_HZ(3,3,&MenuHZ11_24[2],6);
	ShowLineChar_HZ(4,3,&MenuHZ11_26[2],6);
	ShowLineChar_HZ(5,3,&MenuHZ11_27[2],7);
	ShowLineChar_HZ(6,3,&MenuHZ11_28[2],8);
	ShowLineChar_HZ(7,3,&ZCBuChaoBiao[0],6);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu11_21
**	函 数  功 能: 1.通信通道设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu11_21(void) 		
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowMenu_ParaSet();
	ReverseShowLine(2);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu11_22
**	函 数  功 能: 2.终端时间设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu11_22(void) 			// "2.终端时间设置"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowMenu_ParaSet();
	ReverseShowLine(3);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu11_23
**	函 数  功 能: 3.终端编号设置
**	输 入  参 数:  none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu11_23(void) 			// "3.终端编号设置"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowMenu_ParaSet();
	ReverseShowLine(4);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu11_24
**	函 数  功 能: 4.抄表时间点设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu11_24(void) 			// "4.抄表时间点设置"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowMenu_ParaSet();
	ReverseShowLine(5);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu11_24
**	函 数  功 能: 5.GPRS参数设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu11_25(void) 			// "5.GPRS参数设置"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowMenu_ParaSet();
	ReverseShowLine(6);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu11_24
**	函 数  功 能: 补抄表参数设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu11_26(void) 			// "6.补抄表参数设置"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowMenu_ParaSet();
	ReverseShowLine(7);
}


/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu12_21
**	函 数  功 能: 1.查看当前时间节点抄表状态
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
//该函数需要修改
void Action_LcdMenu12_21(void) 			// "1.查看当前时间节点抄表状态"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,5,&MenuHZ12[0],9);
	ShowLineChar_HZ(2,3,&MenuHZ12_21[0],6);
	ShowLineChar_HZ(3,3,&MenuHZ12_22[0],4);
	ShowLineChar_HZ(4,3,&MenuHZ12_23[0],6);/*yangfei added*/
	ShowLineChar_HZ(5,3,&MenuHZ12_24[0],6);/*失败信息查询，added by zjjin */
	ReverseShowLine(2);
}
/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu12_22
**	函 数  功 能: 立即抄表
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
//该函数需要修改
void Action_LcdMenu12_22(void) 			// "1.查看当前时间节点抄表状态"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,5,&MenuHZ12[0],9);
	ShowLineChar_HZ(2,3,&MenuHZ12_21[0],6);
	ShowLineChar_HZ(3,3,&MenuHZ12_22[0],4);
	ShowLineChar_HZ(4,3,&MenuHZ12_23[0],6);/*yangfei added*/
	ShowLineChar_HZ(5,3,&MenuHZ12_24[0],6);/*失败信息查询，added by zjjin */
	ReverseShowLine(3);
}
/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu12_23
**	函 数  功 能: 抄表通道设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
//该函数需要修改
void Action_LcdMenu12_23(void) 			/*抄表通道设置*/
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,5,&MenuHZ12[0],9);
	ShowLineChar_HZ(2,3,&MenuHZ12_21[0],6);
	ShowLineChar_HZ(3,3,&MenuHZ12_22[0],4);
	ShowLineChar_HZ(4,3,&MenuHZ12_23[0],6);/*yangfei added*/
	ShowLineChar_HZ(5,3,&MenuHZ12_24[0],6);/*失败信息查询，added by zjjin */
	ReverseShowLine(4);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu12_24
**	函 数  功 能: 失败信息查询
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
//该函数需要修改
void Action_LcdMenu12_24(void) 			/*失败信息查询*/
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,5,&MenuHZ12[0],9);
	ShowLineChar_HZ(2,3,&MenuHZ12_21[0],6);
	ShowLineChar_HZ(3,3,&MenuHZ12_22[0],4);
	ShowLineChar_HZ(4,3,&MenuHZ12_23[0],6);/*yangfei added*/
	ShowLineChar_HZ(5,3,&MenuHZ12_24[0],6);/*失败信息查询，added by zjjin */
	ReverseShowLine(5);
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu13_21
**	函 数  功 能: 1.重启终端
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
// 集中器管理与维护 下级菜单
void Action_LcdMenu13_21(void)  		// "1.重启终端"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ13[0],8);   //终端管理与维护
    ShowLineChar_HZ(2,2,&MenuHZ13_21[0],4);//重启终端
	ShowLineChar_HZ(3,2,&MenuHZ13_25[2],6);//终端版本信息
	ShowLineChar_HZ(4,2,&MenuHZ13_26[2],7);//液晶对比度
	
	ReverseShowLine(2);	
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu13_22
**	函 数  功 能: 2.终端版本信息
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu13_22(void)  		// "2.终端版本信息"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ13[0],8);   //终端管理与维护
    ShowLineChar_HZ(2,2,&MenuHZ13_21[0],4);//重启终端
	ShowLineChar_HZ(3,2,&MenuHZ13_25[2],6);//终端版本信息
	ShowLineChar_HZ(4,2,&MenuHZ13_26[2],7);//液晶对比度
	ReverseShowLine(3);	
}

/****************************************************************************************************
**	函 数  名 称: Action_LcdMenu13_23
**	函 数  功 能: 3.液晶对比度设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Action_LcdMenu13_23(void)  		// "3.液晶对比度设置"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ13[0],8);   //终端管理与维护
    ShowLineChar_HZ(2,2,&MenuHZ13_21[0],4);//重启终端
	ShowLineChar_HZ(3,2,&MenuHZ13_25[2],6);//终端版本信息
	ShowLineChar_HZ(4,2,&MenuHZ13_26[2],7);//液晶对比度
	ReverseShowLine(4);	
}



//muxiaoqing
void Action_LcdMenu16_21(void)		//"SD 卡数据写入和读出对比测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	#if 0
 //   ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
    ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
	#else
	
    ShowLineChar_HZ(1,2,&MenuHZ16_21[0],4);//"参数设置"
	ShowLineChar_HZ(2,2,&MenuHZ16_22[0],4);//"自动测试"
	#endif
	ReverseShowLine(1);	
}

void Action_LcdMenu16_22(void)		//"时钟芯片的写入和读出对比测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
   #if 0
	//	 ShowLineChar_HZ(1,2,&MenuHZ16[0],8);	//终端管理与维护
	   ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
	   ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
	   ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"   
	   ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
	   ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
	   ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
	   ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
	   ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
   #else
	   
	   ShowLineChar_HZ(1,2,&MenuHZ16_21[0],4);//"手动测试"
	   ShowLineChar_HZ(2,2,&MenuHZ16_22[0],4);//"自动测试"
   #endif
	ReverseShowLine(2); 
}
#if 0
void Action_LcdMenu16_23(void)		//"GPRS 指令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
   // ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
   ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
   ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
   ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"   
   ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
   ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
   ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
   ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
   ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
	ReverseShowLine(3); 
}

void Action_LcdMenu16_24(void)		//"调试串口命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
   // ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
   ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
   ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
   ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"   
   ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
   ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
   ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
   ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
   ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
	ReverseShowLine(4); 
}

void Action_LcdMenu16_25(void)		//"抄表串口1命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
   // ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
   ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
   ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
   ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"   
   ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
   ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
   ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
   ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
   ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
	ReverseShowLine(5); 
}

void Action_LcdMenu16_26(void)		//"GPRS通讯命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
  //  ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
  ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
  ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
  ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"   
  ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
  ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
  ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
  ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
  ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
	ReverseShowLine(6); 
}

void Action_LcdMenu16_27(void)		//"RS485 通讯1命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
   // ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
   ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
   ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
   ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"   
   ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
   ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
   ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
   ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
   ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
	ReverseShowLine(7); 
}

void Action_LcdMenu16_28(void)		//"RS485 通讯2命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
  //  ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
  ShowLineChar_HZ(1,2,&MenuHZ16_21[0],12);//"SD 卡数据写入和读出对比测试"
  ShowLineChar_HZ(2,2,&MenuHZ16_22[0],9);//"时钟芯片的写入和读出对比测试"
  ShowLineChar_HZ(3,2,&MenuHZ16_23[0],12);//"GPRS 指令测试"   
  ShowLineChar_HZ(4,2,&MenuHZ16_24[0],15);//"调试串口命令测试"
  ShowLineChar_HZ(5,2,&MenuHZ16_25[0],14);//"抄表串口1命令测试"
  ShowLineChar_HZ(6,2,&MenuHZ16_26[0],14);//"GPRS通讯命令测试"
  ShowLineChar_HZ(7,2,&MenuHZ16_27[0],16);//"RS485 通讯1命令测试"
  ShowLineChar_HZ(8,2,&MenuHZ16_28[0],15);//"RS485 通讯2命令测试"
	ReverseShowLine(8); 
}
#else
#ifdef DEMO_APP
void Action_LcdMenu16_21_31(void)		//"德鲁表采集"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],2);//"德鲁"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],3);//"兰吉尔"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],2);//"天罡"	
//    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ReverseShowLine(1);	
}

void Action_LcdMenu16_21_32(void)		//"兰吉尔表采集"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],2);////"德鲁"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],3);//"兰吉尔"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],2);//"天罡"	
 //   ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ReverseShowLine(2); 
}

void Action_LcdMenu16_21_33(void)		//天罡表采集"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],2);////"德鲁"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],3);//"兰吉尔"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],2);//"天罡"	
 //   ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ReverseShowLine(3); 
}

void Action_LcdMenu16_21_34(void)		//"预留"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	
	ReverseShowLine(4); 
}

#else
void Action_LcdMenu16_21_31(void)		//"SD 卡数据写入和读出对比测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
 //   ShowLineChar_HZ(1,2,&MenuHZ16[0],8);   //终端管理与维护
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(1);	
}

void Action_LcdMenu16_21_32(void)		//"时钟芯片的写入和读出对比测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(2); 
}

void Action_LcdMenu16_21_33(void)		//"GPRS 指令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(3); 
}

void Action_LcdMenu16_21_34(void)		//"调试串口命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(4); 
}
void Action_LcdMenu16_21_35(void)		//"抄表串口1命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(5); 
}

void Action_LcdMenu16_21_36(void)		//"GPRS通讯命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(6); 
}

void Action_LcdMenu16_21_37(void)		//"RS485 通讯1命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(7); 
}

void Action_LcdMenu16_21_38(void)		//"RS485 通讯2命令测试"
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
    ShowLineChar_HZ(1,2,&MenuHZ16_21_31[0],5);//"SD 卡数据写入和读出对比测试"
	ShowLineChar_HZ(2,2,&MenuHZ16_21_32[0],4);//"时钟芯片的写入和读出对比测试"
	ShowLineChar_HZ(3,2,&MenuHZ16_21_33[0],8);//"GPRS 指令测试"	
    ShowLineChar_HZ(4,2,&MenuHZ16_21_34[0],9);//"调试串口命令测试"
	ShowLineChar_HZ(5,2,&MenuHZ16_21_35[0],8);//"抄表串口1命令测试"
	ShowLineChar_HZ(6,2,&MenuHZ16_21_36[0],10);//"GPRS通讯命令测试"
	ShowLineChar_HZ(7,2,&MenuHZ16_21_37[0],13);//"RS485 通讯1命令测试"
	ShowLineChar_HZ(8,2,&MenuHZ16_21_38[0],12);//"RS485 通讯2命令测试"
	ReverseShowLine(8); 
}
#endif

#endif

//三级菜单
/****************************************************************************************************
**	函 数  名 称: ShowMenu_CommuSet
**	函 数  功 能: 通信通道设置 总菜单
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
//通信通道设置
void ShowMenu_CommuSet(void)	//通信通道设置 总菜单
{
	//ShowLineChar_HZ(1,10,&MenuHZ12_21[2],7);
	display_white();
	ShowMenuTop();
	//ShowMenuBottom(1);
	//ShowLineChar_HZ(1,3,&MenuHZ12_21_31[0],10);
	//ShowLineChar_HZ(2,3,&MenuHZ12_21_32[0],10);
	//ShowLineChar_HZ(3,3,&MenuHZ12_21_33[0],8);
	//ShowLineChar_HZ(5,32,&MenuHZ12_21_34[0],4);
	//ShowLineChar_HZ(5,3,&MenuHZ12_21_35[0],10);
	
	ShowLineChar_HZ(4,12,&Menu_ConfigType[0][0],10);
}

/****************************************************************************************************
**	函 数  名 称: FreshTimeSet
**	函 数  功 能: 刷新时间设置页面
**	输 入  参 数: uint8* date
**                uint8* time
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
//终端时间设置
void FreshTimeSet(uint8* date,uint8* time)//刷新时间设置页面
{
	uint8 line,column;

	LOG_assert_param( date == NULL );
	LOG_assert_param( time == NULL );

	line = 2;
	column = 0;	
	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowPicture(16,0,16,160,&MenuPic1_2_2[0]);
	ShowPicture(64,22,16,32,&MenuPic_OK[0]);//OK 4行
		
	ShowIniDate(line,column,&date[0]);
	ShowIniTime(line + 1,column,&time[0]);
}


/****************************************************************************************************
**	函 数  名 称: Set_SystemTime
**	函 数  功 能: 终端时间设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Set_SystemTime(void)  //终端时间设置
{
	uint8 status,err;
	uint8 line1,line2;
	uint8 column1,column2;
	uint8 i,j;
	uint8 datebuf[6];
	uint8 ASCIIData[12];
	uint8 HexData[6];
	
	SelectInfo TabMenu[] = {{2,0,33},{3,0,33},{4,22,11}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
		
	//读当前时间
	//UserReadDateTime(&datebuf[0]);//秒分时日月年
	ReadDateTime(&datebuf[0]);
	/*begin:yangfei added 2013-06-04 for 时间有效性判断 */
	for(i=0;i<6;i++)
		{
	debug("%d ",datebuf[i]);

		}
	if(datebuf[0]>60)
		{
		datebuf[0]=1;
		datebuf[1]=1;
		datebuf[2]=1;
		datebuf[3]=1;
		datebuf[4]=1;
		datebuf[5]=9;
		}
	/*end:yangfei added 2013-06-04 for 时间有效性判断 */
	datebuf[4] = datebuf[4]&0x1F;//去掉星期
	for(i=0;i<6;i++)
	{
		ASCIIData[2*i] = HexToAscii(datebuf[5-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(datebuf[5-i]&0x0f);
	}
	//显示当前时间
	FreshTimeSet(&ASCIIData[0],&ASCIIData[6]);
	i = 0;
	j = 6;
	line1 = TabMenu[0].startline;//2
	line2 = TabMenu[1].startline;//3
	column1 = TabMenu[0].startcolumn;//0
	column2 = TabMenu[1].startcolumn;//0
	
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
		FreshTimeSet(&ASCIIData[0],&ASCIIData[6]);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 5;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else
		{
			if(TabNum == 0)
			{
				ReverseShowChar(line1,column1);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(ASCIIData[i] == 0x39)
						{
							ASCIIData[i] = 0x30;
						}
						else
						{
							ASCIIData[i] = ASCIIData[i] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(ASCIIData[i] == 0x30)
						{
							ASCIIData[i] = 0x39;
						}
						else
						{
							ASCIIData[i] = ASCIIData[i] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line1,column1,ASCIIData[i]);
						if(i > 0)
						{
							if(i%2)
							{
								column1 = column1 - 3;
							}
							else
							{
								column1 = column1 - 8;
							}
							i = i - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line1,column1,ASCIIData[i]);
						if(i < 5)
						{
							if(i%2)
							{
								column1 = column1 + 8;
							}
							else
							{
								column1 = column1 + 3;
							}
							i = i + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						j = 6;
						line1 = TabMenu[0].startline;//2
						line2 = TabMenu[1].startline;//3
						column1 = TabMenu[0].startcolumn;//0
						column2 = TabMenu[1].startcolumn;//0					
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 5;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					
					default:	
						break;
				}
			}//年月日设置
				
			else if(TabNum == 1)
			{
				ReverseShowChar(line2,column2);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(ASCIIData[j] == 0x39)
						{
							ASCIIData[j] = 0x30;
						}
						else
						{
							ASCIIData[j] = ASCIIData[j] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(ASCIIData[j] == 0x30)
						{
							ASCIIData[j] = 0x39;
						}
						else
						{
							ASCIIData[j] = ASCIIData[j] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line2,column2,ASCIIData[j]);
						if(j > 6)
						{
							if(j%2)
							{
								column2 = column2 - 3;
							}
							else
							{
								column2 = column2 - 8;
							}
							j = j - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line2,column2,ASCIIData[j]);
						if(j < 11)
						{
							if(j%2)
							{
								column2 = column2 + 8;
							}
							else
							{
								column2 = column2 + 3;
							}
							j = j + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						j = 6;
						line1 = TabMenu[0].startline;//2
						line2 = TabMenu[1].startline;//3
						column1 = TabMenu[0].startcolumn;//0
						column2 = TabMenu[1].startcolumn;//0					
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 5;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					
					default:	
						break;
				}				
			}//end if(TabNum == 1) 时分秒 设置
		}//end if(Tab_Flag == TRUE)
	}//end while	
	
	//==========================================
	//此处需要加入设置时间函数，然后根据返回值的结果判断是否设置成功
	for(i=0;i<6;i++)//将ASC码转换为BCD，并按照秒---年的顺序存放
	{
		HexData[5-i] = ((ASCIIData[2*i]-0x30)<<4) + (ASCIIData[2*i+1]-0x30);
	}
	HexData[4] = HexData[4]|0x20;
	err = TimeCheck(&HexData[0]);
	if(err != 0)//不合要求
	{
		if(err == 1)///日期不合要求
		{
			ShowMenuBottom(7);
		}
		else if(err == 2)//时间不合法
		{
			ShowMenuBottom(8);
		}
		OSTimeDly(OS_TICKS_PER_SEC*2);
		KeyFuncIndex = 5;
		(*KeyTab[KeyFuncIndex].CurrentOperate)();
		return;
	}
	
	err = UserSetDateTime(&HexData[0]);
	ShowMenuBottom(2);//需要根据时间设置函数的返回值，在底部状态栏显示设置是否成功
	OSTimeDly(OS_TICKS_PER_SEC*2);
	//===========================================
	
	KeyFuncIndex = 5;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();
	return;
}

/****************************************************************************************************
**	函 数  名 称: FreshSetReadMeterTime
**	函 数  功 能: 刷新抄表时间设置页面
**	输 入  参 数: uint8 timenum
**                uint8 *pIndex
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshSetReadMeterTime(uint8 *pIndex, uint8 *pTime)//刷新抄表时间设置页面
{
	uint8 line,column;
	//uint8 TimeNodeIndex = 0;
	uint8 NodeTempH		= 0;
	uint8 NodeTempL		= 0;
	//uint8 time[6]={0};

	LOG_assert_param( pIndex == NULL );
	LOG_assert_param( pTime == NULL );

	line = 2;
	column = 16;	
	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,10,&MenuHZ11_27[2],7);
	ShowLineChar_HZ(2,9,&MenuHZ_TimeNum[0],9);
	ShowPicture(64,22,16,32,&MenuPic_OK[0]);//OK 4行
		
	NodeTempH = *pIndex++;
	NodeTempL = *pIndex;
	ShowChar(2, 14, NodeTempH);
	ShowChar(2, 17, NodeTempL);
	
	/*
	TimeNodeIndex = ((NodeTempH-0x30)<<4) + ((NodeTempL-0x30)&0x0F);
	if((TimeNodeIndex>0x09) || (TimeNodeIndex==0x00))		TimeNodeIndex = 0x01;
	memcpy(time,&gPARA_TimeNodes[TimeNodeIndex-1],2);
	*/
	ShowTimeMS(line + 1, column, pTime);
}
/****************************************************************************************************
**	函 数  名 称: Set_ReadMeterTime
**	函 数  功 能: 抄表时间设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Set_ReadMeterTime(void)  //抄表时间设置
{
	uint8 status;
	uint8 line1,line2;
	uint8 column1,column2;
	uint8 i,j,k;
	//uint8 datebuf[6];
	uint8 ASCIIData[4];
	uint8 ASCIIData1[2];
	//uint8 HexData[6];
	uint8 TimeNum = 0x00;
	uint8 TimeNodeIndex = 0;
	uint8 timebuf[2] = {0x0};
	
	SelectInfo TabMenu[] = {{2,14,6},{3,16,22},{4,22,11}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
		
	//读抄表时间点
	memcpy(timebuf,&gPARA_TimeNodes[TimeNum],2);
	for(i=0;i<2;i++)
	{
		ASCIIData[2*i] = HexToAscii(timebuf[1-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(timebuf[1-i]&0x0f);
	}

	//默认显示第一抄表时间点
	ASCIIData1[0] = HexToAscii((TimeNum+1)>>4);
	ASCIIData1[1] = HexToAscii((TimeNum+1)&0x0f);
	FreshSetReadMeterTime(ASCIIData1, timebuf);
		
	i = 0;
	j = 0;
	line1 = TabMenu[0].startline;//2
	line2 = TabMenu[1].startline;//3
	column1 = TabMenu[0].startcolumn;//14
	column2 = TabMenu[1].startcolumn;//16
	
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
		FreshSetReadMeterTime(ASCIIData1, timebuf);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 7;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else
		{
			if(TabNum == 0)
			{
				ReverseShowChar(line1,column1);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(ASCIIData1[i] == 0x39)
						{
							ASCIIData1[i] = 0x30;
						}
						else
						{
							ASCIIData1[i] = ASCIIData1[i] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(ASCIIData1[i] == 0x30)
						{
							ASCIIData1[i] = 0x39;
						}
						else
						{
							ASCIIData1[i] = ASCIIData1[i] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line1,column1,ASCIIData1[i]);
						if(i > 0)
						{
							column1 = column1 - 3;
							i = i - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line1,column1,ASCIIData1[i]);
						if(i < 1)
						{
							column1 = column1 + 3;
							i = i + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						j = 0;
						line1 = TabMenu[0].startline;//2
						line2 = TabMenu[1].startline;//3
						column1 = TabMenu[0].startcolumn;//0
						column2 = TabMenu[1].startcolumn;//0					
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 19;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					
					default:	
						break;
				}
				
				TimeNodeIndex = ((ASCIIData1[0]-0x30)<<4) + ((ASCIIData1[1]-0x30)&0x0F);
				TimeNodeIndex = BcdToHex(TimeNodeIndex);
				if((TimeNodeIndex>TIME_NODE_NUM_MAX) || (TimeNodeIndex==0x00))		TimeNodeIndex = 0x01;
				memcpy(timebuf,&gPARA_TimeNodes[TimeNodeIndex-1],2);
				
				for(k=0;k<2;k++)
				{
					ASCIIData[2*k] = HexToAscii(timebuf[1-k]>>4);
					ASCIIData[2*k+1] = HexToAscii(timebuf[1-k]&0x0f);
				}
			}
				
			else if(TabNum == 1)
			{
				ReverseShowChar(line2,column2);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(ASCIIData[j] == 0x39)
						{
							ASCIIData[j] = 0x30;
						}
						else
						{
							ASCIIData[j] = ASCIIData[j] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(ASCIIData[j] == 0x30)
						{
							ASCIIData[j] = 0x39;
						}
						else
						{
							ASCIIData[j] = ASCIIData[j] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line2,column2,ASCIIData[j]);
						if(j > 0)
						{
							if(j%2)
							{
								column2 = column2 - 3;
							}
							else
							{
								column2 = column2 - 8;
							}
							j = j - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line2,column2,ASCIIData[j]);
						if(j < 3)
						{
							if(j%2)
							{
								column2 = column2 + 8;
							}
							else
							{
								column2 = column2 + 3;
							}
							j = j + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						j = 0;
						line1 = TabMenu[0].startline;//2
						line2 = TabMenu[1].startline;//3
						column1 = TabMenu[0].startcolumn;//0
						column2 = TabMenu[1].startcolumn;//0					
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 19;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					
					default:	
						break;
				}
				for(i=0;i<2;i++)//将ASC码转换为BCD，
				{
					timebuf[1-i] = ((ASCIIData[2*i]-0x30)<<4) + (ASCIIData[2*i+1]-0x30);
				}				
			}//end if(TabNum == 1) 时分秒 设置
		}//end if(Tab_Flag == TRUE)
	}//end while	
	
	//==========================================
	//此处需要加入设置抄表时间点函数
	TimeNum = ((ASCIIData1[0]-0x30)<<4) + (ASCIIData1[1]-0x30);
	TimeNum = BcdToHex(TimeNum);
	for(i=0;i<2;i++)//将ASC码转换为BCD，
	{
		timebuf[1-i] = ((ASCIIData[2*i]-0x30)<<4) + (ASCIIData[2*i+1]-0x30);
	}
	
	if((BcdToHex(timebuf[1])>23)||(BcdToHex(timebuf[0])>59))
	{
	    ShowMenuBottom(8);
	    OSTimeDly(OS_TICKS_PER_SEC*2);
		KeyFuncIndex = 7;
		(*KeyTab[KeyFuncIndex].CurrentOperate)();
		return;
	}
	SetTimeNode(timebuf, TimeNum);
	//err = 抄表时间设置函数
	ShowMenuBottom(2);//需要根据时间设置函数的返回值，在底部状态栏显示设置是否成功
	OSTimeDly(OS_TICKS_PER_SEC*2);
	//===========================================
	
	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();
	return;
}


/****************************************************************************************************
**	函 数  名 称: FreshTermAddrSet
**	函 数  功 能: 刷新项目地址设置
**	输 入  参 数: uint8* InputBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: muxiaoqing modify protocal
*****************************************************************************************************/
void FreshTermAddrSet(uint8* InputBuffDevicecode,uint8* InputBuffBuildingcode,uint8* InputBuffAreacode)
{
	uint8 i,j,line,column;
#if 0
    LOG_assert_param( InputBuff == NULL );

#else
LOG_assert_param( InputBuffAreacode == NULL );
#endif
	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(21);
	ShowLineChar_HZ(1,8,&MenuHZ11_26[2],6);		//终端编号设置 
	
	ShowLineChar_HZ(2,8,&AreaCode[0],3);	//楼号:	//区号
	
	ShowLineChar_HZ(3,8,&BuildingCode[0],3);	//单元:	//楼号
	
	ShowLineChar_HZ(4,8,&DeviceCode[0],3);		//设备号
	//ShowPicture(48,22,16,32,&MenuPic_OK[0]);//3行 OK
	
	ShowPicture(80,22,16,32,&MenuPic_OK[0]);//5行 OK
	#if 0
	line = 2;
	column = 20;
	for(i=0;i<4;i++)
	{
		ShowChar(line,column+3*i,InputBuff[i]);
	}	
	#endif
	
	line = 2;
	column = 25;
	for(i=0;i<3;i++)
	{ 
	    for(j=0;j<4;j++)
	    	{
	    	    if(i==0)
        		ShowChar(line+i,column+3*j,InputBuffAreacode[j]);
				else if(i==1)				
        		ShowChar(line+i,column+3*j,InputBuffBuildingcode[j]);				
				else if(i==2)				
        		ShowChar(line+i,column+3*j,InputBuffDevicecode[j]);
	    	}
	}	
	return;
}

/****************************************************************************************************
**	函 数  名 称: Set_SystemAddr
**	函 数  功 能: 终端编号地址设置
**	输 入  参 数: none
**	输 出  参 数: 无					                          
**  返   回   值: 无
**	备		  注: 慕肖清添加 区号 楼号 设备号 完善终端编码设置协议
*****************************************************************************************************/
void Set_SystemAddr(void)//终端编号地址设置
{
	uint8 status;
#if 0	//muxiaoqing modefy	
	uint8 line;
	uint8 column;
	uint8 column1;
	uint8 line1;
	int8 i;
	uint8 ASCIIData[4];
#else
	uint8 line;
	uint8 column;
	uint8 column1;
	uint8 line1;
	
	uint8 column2;
	uint8 line2;
	
	uint8 column3;
	uint8 line3;
	int8 i;

	uint8 ASCIIDataAreacode[4];	
	uint8 ASCIIDataBuildingcode[4];	
	uint8 ASCIIDataDevicecode[4];
#endif	
	uint8 TermAddr[6] = {0};
	uint8 TermAddrReverse[6] = {0x00};
//	uint8 datalen;
#if 0	
	SelectInfo TabMenu[] = {{2,20,12},{3,22,11}};//行，列，长度
#else
    SelectInfo TabMenu[] = {{2,25,13},{3,25,13},{4,25,13},{5,22,11}};//行，列，长度
	
#endif
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	memcpy(TermAddrReverse, gPARA_TermPara.TermAddr, 6);
	OS_EXIT_CRITICAL();
	
	//由于存储低字节在前，所以现在将字节顺序调换
	for(i=0; i<6; i++)
	{
		TermAddr[i] = TermAddrReverse[5-i];
	}
#if 0	
	//取调换顺序后的后2字节有效
	for(i=0;i<2;i++)
	{
		ASCIIData[2*i] = HexToAscii(TermAddr[4+i]>>4);
		ASCIIData[2*i+1] = HexToAscii(TermAddr[4+i]&0x0f);
	}
#else
	
//取调换顺序后的后2字节有效
for(i=0;i<2;i++)
{
	ASCIIDataDevicecode[2*i] = HexToAscii(TermAddr[4+i]>>4);
	ASCIIDataDevicecode[2*i+1] = HexToAscii(TermAddr[4+i]&0x0f);
}
for(i=0;i<2;i++)
{
	ASCIIDataBuildingcode[2*i] = HexToAscii(TermAddr[2+i]>>4);
	ASCIIDataBuildingcode[2*i+1] = HexToAscii(TermAddr[2+i]&0x0f);
}
for(i=0;i<2;i++)
{
	ASCIIDataAreacode[2*i] = HexToAscii(TermAddr[0+i]>>4);
	ASCIIDataAreacode[2*i+1] = HexToAscii(TermAddr[0+i]&0x0f);
}

#endif
	i = 0;
	line = TabMenu[0].startline;
	column = TabMenu[0].startcolumn;
	line1 = TabMenu[1].startline;
	column1 = TabMenu[1].startcolumn;
	
	line2 = TabMenu[2].startline;
	column2 = TabMenu[2].startcolumn;
	
	line3 = TabMenu[3].startline;
	column3 = TabMenu[3].startcolumn;
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
	   #if 0
		FreshTermAddrSet(&ASCIIData[0]);
	   #else	   
	   FreshTermAddrSet(&ASCIIDataDevicecode[0],&ASCIIDataBuildingcode[0],&ASCIIDataAreacode[0]);
	   #endif
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 6;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
	#if 0
		else if(TabNum == 0)
		{
			ReverseShowChar(line,column);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIData[i] == 0x39)
					{
						ASCIIData[i] = 0x30;
					}
					else
					{
						ASCIIData[i] = ASCIIData[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if(ASCIIData[i] == 0x30)
					{
						ASCIIData[i] = 0x39;
					}
					else
					{
						ASCIIData[i] = ASCIIData[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line,column,ASCIIData[i]);
					if(i > 0)
					{
						i = i - 1;
						column = column - 3;
					}
					break;
				case KEY_RIGHT://向右键
					ShowChar(line,column,ASCIIData[i]);
					if(i < 3)
					{
						i = i + 1;
						column = column + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					line1 = line1 ;
					column1 = column1;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 6;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}//end else if(TabNum == 0)
		#else
		else if(TabNum == 0)		
		{
			ReverseShowChar(line,column);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIDataAreacode[i] >= 0x39)
					{
						ASCIIDataAreacode[i] = 0x30;
					}
					else
					{
						ASCIIDataAreacode[i] = ASCIIDataAreacode[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIDataDevicecode[i] <= 0x30)||(ASCIIDataDevicecode[i] >0x39))
					{
						ASCIIDataAreacode[i] = 0x39;
					}
					else
					{
						ASCIIDataAreacode[i] = ASCIIDataAreacode[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line,column,ASCIIDataAreacode[i]);
					if(i > 0)
					{
						i = i - 1;
						column = column - 3;
					}
					break;
				case KEY_RIGHT://向右键
					ShowChar(line,column,ASCIIDataAreacode[i]);
					if(i < 3)
					{
						i = i + 1;
						column = column + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 6;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		
		else if(TabNum == 1)		
		{
			ReverseShowChar(line1,column1);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIDataBuildingcode[i] >= 0x39)
					{
						ASCIIDataBuildingcode[i] = 0x30;
					}
					else
					{
						ASCIIDataBuildingcode[i] = ASCIIDataBuildingcode[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIDataDevicecode[i] <= 0x30)||(ASCIIDataDevicecode[i] >0x39))
					{
						ASCIIDataBuildingcode[i] = 0x39;
					}
					else
					{
						ASCIIDataBuildingcode[i] = ASCIIDataBuildingcode[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line1,column1,ASCIIDataBuildingcode[i]);
					if(i > 0)
					{
						i = i - 1;
						column1 = column1 - 3;
					}
					break;
				case KEY_RIGHT://向右键
					ShowChar(line1,column1,ASCIIDataBuildingcode[i]);
					if(i < 3)
					{
						i = i + 1;
						column1 = column1 + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 6;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		
		else if(TabNum == 2)		
		{
			ReverseShowChar(line2,column2);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIDataDevicecode[i] >= 0x39)
					{
						ASCIIDataDevicecode[i] = 0x30;
					}
					else
					{
						ASCIIDataDevicecode[i] = ASCIIDataDevicecode[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIDataDevicecode[i] <= 0x30)||(ASCIIDataDevicecode[i] >0x39))
					{
						ASCIIDataDevicecode[i] = 0x39;
					}
					else
					{
						ASCIIDataDevicecode[i] = ASCIIDataDevicecode[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line2,column2,ASCIIDataDevicecode[i]);
					if(i > 0)
					{
						i = i - 1;
						column2 = column2 - 3;
					}
					break;
				case KEY_RIGHT://向右键
					ShowChar(line2,column2,ASCIIDataDevicecode[i]);
					if(i < 3)
					{
						i = i + 1;
						column2 = column2 + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 6;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		#endif

	}//end while

	memset(TermAddr, 0x00, 6); //
#if 0
	for(i=0;i<2;i++)
	{
		TermAddr[1-i] =  ((ASCIIData[2*i]-0x30)<<4) + ((ASCIIData[2*i+1]-0x30)&0x0F);
	}
#else
for(i=0;i<2;i++)
{
	TermAddr[1-i] =  ((ASCIIDataDevicecode[2*i]-0x30)<<4) + ((ASCIIDataDevicecode[2*i+1]-0x30)&0x0F);
}
for(i=0;i<2;i++)
{
	TermAddr[3-i] =  ((ASCIIDataBuildingcode[2*i]-0x30)<<4) + ((ASCIIDataBuildingcode[2*i+1]-0x30)&0x0F);
}
for(i=0;i<2;i++)
{
	TermAddr[5-i] =  ((ASCIIDataAreacode[2*i]-0x30)<<4) + ((ASCIIDataAreacode[2*i+1]-0x30)&0x0F);
}

#endif
	SetTermAddr(TermAddr); //
	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;
}

/****************************************************************************************************
**	函 数  名 称: MeterDataShow
**	函 数  功 能: 显示抄表信息
**	输 入  参 数: 
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void MeterDataShow(uint16 nowNum,uint16 allNum,uint8 *DataBuf)
{
	uint8 i = 0;
	uint8 lu8line = 0;
	uint8 lu8column = 0;
	uint16 lu16qian = 0;//千位数
	uint16 lu16bai = 0; //百位数
	uint16 lu16shi = 0;
	uint16 lu16ge = 0;
	uint16 lu16tmp = 0;
	uint8 lu8tmp = 0;
	uint8 lu8tmpArray[7] = {0};
	uint8 lu8DisplayFlag = 0;//用于标记是否显示0.(一串数字前面的0控制显示用)

	if(nowNum > 999)   //防止超限，正常情况下总数不会多于600. 此函数按照3位整数处理。
		nowNum = 999;
	if(allNum > 999)
		allNum = 999;


//begin: 显示当前进度。
	lu16bai = nowNum / 100;
	lu16shi = (nowNum - lu16bai*100) / 10;
	lu16ge = nowNum % 10;

	lu8line = 1;   //从第1行第0列开始。
	lu8column = 0;

	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; //每个ASCII码占3列。
	}
	

	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}
	

	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;

	ShowChar(lu8line,lu8column,0x2f);  //显示斜线。
	lu8column += 3;

	lu16bai = allNum / 100;
	lu16shi = (allNum - lu16bai*100) / 10;
	lu16ge = allNum % 10;

	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; 
	}

	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}

	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;

//end: 显示当前进度。

//begin: 显示住户信息。
	lu8line = 1;   //从第1行第0列开始。
	lu8column = 34;
	ShowChar(lu8line,lu8column,(*(DataBuf+11)+0x30));  //显示单元号。
	lu8column += 3;
	ShowChar(lu8line,lu8column,0x2d);  //显示短横线。
	lu8column += 3;

    lu16tmp = *((uint16 *)(DataBuf+12));  //取出房号信息。
    if(lu16tmp > 9999)   //最多支持4位房号。
		lu16tmp = 9999;
	
    lu16qian = lu16tmp / 1000;
	lu16bai = (lu16tmp % 1000) / 100;
	lu16shi = (lu16tmp % 100) / 10;
	lu16ge = lu16tmp % 10;

	if(lu16qian){
		ShowChar(lu8line,lu8column,(lu16qian+0x30));
		lu8column += 3;
	}
	if(lu16bai || lu16qian){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3;
	}
	if(lu16shi || lu16bai || lu16qian){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3;
	}
	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;
//end: 显示住户信息。

//begin: 显示当前表的ID号"计量点号"
	lu8line = 2;   //从第2行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCJiLiangDian,4);
	lu8column += 18;

	memcpy(lu8tmpArray,DataBuf,2); //找到计量点号

	lu16tmp = *((uint16 *)lu8tmpArray);

	lu16bai = lu16tmp / 100;
	lu16shi = (lu16tmp % 100) / 10;
	lu16ge = lu16tmp % 10;
	
	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; //每个ASCII码占3列。
	}
	
	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}
	
	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;
	
//end: 显示当前表的ID号"计量点号"


//begin:显示热表号。
	lu8line = 3;   //从第3行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCReBiaoHao,4);
	lu8column += 18;
	
	memcpy(lu8tmpArray,(DataBuf+3),7); //找到热表编号。
	for(i=0;i<4;i++){
		lu16tmp = lu8tmpArray[3-i] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		
		lu16tmp = lu8tmpArray[3-i] & 0x0f;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
	}

//end:显示热表号。

//begin:显示累计热量。
	lu8DisplayFlag = 0;
	lu8line = 4;   //从第4行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCZongReLiang,4);
	lu8column += 18;

	lu8tmp = *(DataBuf + 17);  //判断热表数据长度判断是否抄表成功，如不成功显示全0.
	if(lu8tmp > 40)  //当前如果抄表成功，热表数据长度为43，此处判断40.
		memcpy(lu8tmpArray,(DataBuf+23),5); //找到累计热量。
	else
		memset(lu8tmpArray,0,7); 
		
	for(i=1;i<4;i++){   //累计热量只显示整数部分。
		lu16tmp = lu8tmpArray[4-i] >> 4;
		if(lu8DisplayFlag == 0){
			if(lu16tmp != 0){
				lu8DisplayFlag = 1; 
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

		}
		else{
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			lu8column += 3;
		}

		lu16tmp = lu8tmpArray[4-i] & 0x0f;
		if(lu8DisplayFlag == 0){
			if((lu16tmp != 0) || (i==3)){  //如果是最后一个数字，0也要显示。
				lu8DisplayFlag = 1; 
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

		}
		else{
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			lu8column += 3;
		}

	}
	if(lu8tmpArray[4] == 0x08)
		ShowLineChar_HZ(lu8line,lu8column,ZCMWh,3);
	else
		ShowLineChar_HZ(lu8line,lu8column,ZCkWh,3);

//end:显示累计热量。

//begin:显示累计流量。
		lu8DisplayFlag = 0;
		lu8line = 5;   //从第5行第0列开始。
		lu8column = 0;
		ShowLineChar_HZ(lu8line,lu8column,ZCZongLiuLiang,4);
		lu8column += 18;
	
		lu8tmp = *(DataBuf + 17);  //判断热表数据长度判断是否抄表成功，如不成功显示全0.
		if(lu8tmp > 40)  //当前如果抄表成功，热表数据长度为43，此处判断40.
			memcpy(lu8tmpArray,(DataBuf+38),5); //找到累计流量。
		else
			memset(lu8tmpArray,0,7); 
			
		for(i=1;i<4;i++){	//累计流量只显示整数部分。
			lu16tmp = lu8tmpArray[4-i] >> 4;
			if(lu8DisplayFlag == 0){
				if(lu16tmp != 0){
					lu8DisplayFlag = 1; 
					ShowChar(lu8line,lu8column,(lu16tmp+0x30));
					lu8column += 3;
				}
			
			}
			else{
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

			lu16tmp = lu8tmpArray[4-i] & 0x0f;
			if(lu8DisplayFlag == 0){
				if((lu16tmp != 0) || (i==3)){  //如果是最后一个数字，0也要显示。
					lu8DisplayFlag = 1; 
					ShowChar(lu8line,lu8column,(lu16tmp+0x30));
					lu8column += 3;
				}
			
			}
			else{
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

		}
		ShowChar(lu8line,lu8column,0x2e);  //显示小数点
		lu8column += 3;
		lu16tmp = lu8tmpArray[0] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowHZ( lu8line, lu8column,0x01a7 );  //显示m3。
//end:显示累计流量。

//begin:显示进回水温度。
		lu8line = 6;   //从第6行第0列开始。
		lu8column = 0;
		ShowHZ(lu8line,lu8column,0x013d);  //显示濉板"进"
		lu8column += 5;
		
		lu8tmp = *(DataBuf + 17);  //判断热表数据长度判断是否抄表成功，如不成功显示全0.
		if(lu8tmp > 40)  //当前如果抄表成功，热表数据长度为43，此处判断40.
			memcpy(lu8tmpArray,(DataBuf+43),6); //找到进回水温度。
		else
			memset(lu8tmpArray,0,7); 
				
		lu16tmp = lu8tmpArray[1] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		lu16tmp = lu8tmpArray[1] & 0x0f;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowChar(lu8line,lu8column,0x2e);  //显示小数点
		lu8column += 3;
		lu16tmp = lu8tmpArray[0] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowHZ(lu8line,lu8column,0x01a5);  //显示℃
		lu8column += 5;
		lu8column += 7;  //跳过一部分空显示。
		
		ShowHZ(lu8line,lu8column,0x0123);  //显示濉板"出"
		lu8column += 5;
		lu16tmp = lu8tmpArray[4] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		lu16tmp = lu8tmpArray[4] & 0x0f;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowChar(lu8line,lu8column,0x2e);  //显示小数点
		lu8column += 3;
		lu16tmp = lu8tmpArray[3] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowHZ(lu8line,lu8column,0x01a5);  //显示℃
		lu8column += 5;

//end:显示进回水温度。

//begin:显示流速。
		lu8DisplayFlag = 0;
		lu8line = 7;   //从第7行第0列开始。
		lu8column = 0;
		ShowLineChar_HZ(lu8line,lu8column,ZCLiuSu,3);
		lu8column += 13;
		
		lu8tmp = *(DataBuf + 17);  //判断热表数据长度判断是否抄表成功，如不成功显示全0.
		if(lu8tmp > 40)  //当前如果抄表成功，热表数据长度为43，此处判断40.
			memcpy(lu8tmpArray,(DataBuf+33),5); //找到累计流量。
		else
			memset(lu8tmpArray,0,7); 
				
		for(i=0;i<2;i++){	
			lu16tmp = lu8tmpArray[3-i] >> 4;
			if(lu8DisplayFlag == 0){
				if(lu16tmp != 0){
					lu8DisplayFlag = 1; 
					ShowChar(lu8line,lu8column,(lu16tmp+0x30));
					lu8column += 3;
				}
			
			}
			else{
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

			lu16tmp = lu8tmpArray[3-i] & 0x0f;
			if(lu8DisplayFlag == 0){
				if((lu16tmp != 0) || (i==1)){  //如果是最后一个数字，0也要显示。
					lu8DisplayFlag = 1; 
					ShowChar(lu8line,lu8column,(lu16tmp+0x30));
					lu8column += 3;
				}
			
			}
			else{
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

		}
		ShowChar(lu8line,lu8column,0x2e);  //显示小数点
		lu8column += 3;

		lu16tmp = lu8tmpArray[1] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		lu16tmp = lu8tmpArray[1] & 0x0f;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowLineChar_HZ(lu8line,lu8column,ZCLiFangMiperH,3);
//end:显示流速。



	

}


/****************************************************************************************************
**	函 数  名 称: ValveDataShow
**	函 数  功 能: 显示抄阀控信息
**	输 入  参 数: 
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ValveDataShow(uint16 nowNum,uint16 allNum,uint8 *DataBuf)
{
	uint8 i = 0;
	uint8 Err = 0;
	uint8 lu8line = 0;
	uint8 lu8column = 0;
	uint16 lu16qian = 0;//千位数
	uint16 lu16bai = 0; //百位数
	uint16 lu16shi = 0;
	uint16 lu16ge = 0;
	uint16 lu16tmp = 0;
	uint16 lu16tmp1 = 0;
	uint8 lu8tmp = 0;
	uint8 lu8tmpArray[7] = {0};
	uint8 lu8DisplayFlag = 0;//用于标记是否显示0.(一串数字前面的0控制显示用)

	if(nowNum > 999)   //防止超限，正常情况下总数不会多于600. 此函数按照3位整数处理。
		nowNum = 999;
	if(allNum > 999)
		allNum = 999;


//begin: 显示当前进度。
	lu16bai = nowNum / 100;
	lu16shi = (nowNum - lu16bai*100) / 10;
	lu16ge = nowNum % 10;

	lu8line = 1;   //从第1行第0列开始。
	lu8column = 0;

	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; //每个ASCII码占3列。
	}
	

	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}
	

	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;

	ShowChar(lu8line,lu8column,0x2f);  //显示斜线。
	lu8column += 3;

	lu16bai = allNum / 100;
	lu16shi = (allNum - lu16bai*100) / 10;
	lu16ge = allNum % 10;

	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; 
	}

	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}

	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;

//end: 显示当前进度。

//begin: 显示住户信息。
	lu8line = 1;   //从第1行第0列开始。
	lu8column = 34;
	ShowChar(lu8line,lu8column,(*(DataBuf+11)+0x30));  //显示单元号。
	lu8column += 3;
	ShowChar(lu8line,lu8column,0x2d);  //显示短横线。
	lu8column += 3;

    lu16tmp = *((uint16 *)(DataBuf+12));  //取出房号信息。
    if(lu16tmp > 9999)   //最多支持4位房号。
		lu16tmp = 9999;
	
    lu16qian = lu16tmp / 1000;
	lu16bai = (lu16tmp % 1000) / 100;
	lu16shi = (lu16tmp % 100) / 10;
	lu16ge = lu16tmp % 10;

	if(lu16qian){
		ShowChar(lu8line,lu8column,(lu16qian+0x30));
		lu8column += 3;
	}
	if(lu16bai || lu16qian){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3;
	}
	if(lu16shi || lu16bai || lu16qian){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3;
	}
	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;
//end: 显示住户信息。

//begin: 显示当前表的ID号"计量点号"
	lu8line = 2;   //从第2行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCJiLiangDian,4);
	lu8column += 18;

	memcpy(lu8tmpArray,DataBuf,2); //找到计量点号

	lu16tmp = *((uint16 *)lu8tmpArray);

	lu16bai = lu16tmp / 100;
	lu16shi = (lu16tmp % 100) / 10;
	lu16ge = lu16tmp % 10;
	
	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; //每个ASCII码占3列。
	}
	
	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}
	
	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;
	
//end: 显示当前表的ID号"计量点号"


//begin:显示阀控号。
	lu8line = 3;   //从第3行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCFaKongHao,4);
	lu8column += 18;
	
	memcpy(lu8tmpArray,(DataBuf+3),7); //找到阀控编号。
	for(i=0;i<4;i++){
		lu16tmp = lu8tmpArray[3-i] >> 4;
		if(lu16tmp <= 9)
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		else
			ShowChar(lu8line,lu8column,(lu16tmp+0x37));
			
		lu8column += 3;
		
		lu16tmp = lu8tmpArray[3-i] & 0x0f;
		if(lu16tmp <= 9)
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		else
			ShowChar(lu8line,lu8column,(lu16tmp+0x37));
		
		lu8column += 3;
	}

//end:显示阀控号。

//begin:显示开阀时间。
	lu8DisplayFlag = 0;
	lu8line = 4;   //从第4行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCValveOpenTime,4);
	lu8column += 18;

	lu8tmp = *(DataBuf + 17);  //判断数据长度判断是否抄成功，如不成功显示全0.
	if(lu8tmp > 10)  //当前如果抄阀成功，数据长度为13，此处判断10.
		memcpy(lu8tmpArray,(DataBuf+18),4); //找到开阀时间。
	else
		memset(lu8tmpArray,0,7); 
	
	for(i=1;i<4;i++){	//开阀时间有2位小数。
		lu16tmp = lu8tmpArray[4-i] >> 4;
		if(lu8DisplayFlag == 0){
			if(lu16tmp != 0){
				lu8DisplayFlag = 1; 
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

		}
		else{
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			lu8column += 3;
		}

		lu16tmp = lu8tmpArray[4-i] & 0x0f;
		if(lu8DisplayFlag == 0){
			if((lu16tmp != 0) || (i==3)){  //如果是最后一个数字，0也要显示。
				lu8DisplayFlag = 1; 
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
				lu8column += 3;
			}

		}
		else{
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			lu8column += 3;
		}

	}

	ShowChar(lu8line,lu8column,0x2e);  //显示小数点
	lu8column += 3;

	lu16tmp = lu8tmpArray[0] >> 4;    //显示小数部分。
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;
	lu16tmp = lu8tmpArray[0] & 0x0f;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;

	ShowHZ( lu8line, lu8column, 0xd1);  //时，表示小时。

//end: 显示开阀时间。



//begin:显示室温
	lu8DisplayFlag = 0;
	lu8line = 5;   //从第5行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCShiWen,2);
	lu8column += 13;

	lu8tmp = *(DataBuf + 17);  //判断数据长度判是否抄成功成功，如不成功显示全0.
	if(lu8tmp > 10)  //当前如果抄成功，数据长度为13，此处判断13.
		memcpy(lu8tmpArray,(DataBuf+26),2); //找到室内温度。
	else
		memset(lu8tmpArray,0,7); 

	//室温只显示整数部分.
	lu16tmp = lu8tmpArray[1] >> 4;
	if(lu16tmp > 0){
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
	}

	lu16tmp = lu8tmpArray[1] & 0x0f;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;

	ShowHZ(lu8line,lu8column,0x01a5);  //显示℃



//end:  显示室温



//begin:显示进回水温度。
		lu8line = 6;   //从第6行第0列开始。
		lu8column = 0;
		ShowHZ(lu8line,lu8column,0x013d);  //显示濉板"进"
		lu8column += 5;
		
		lu8tmp = *(DataBuf + 17);  //判断数据长度判断是否抄表成功，如不成功显示全0.
		if(lu8tmp > 10)  //如果抄成功，数据长度为13，此处判断10.
			memcpy(lu8tmpArray,(DataBuf+22),4); //找到进回水温度。
		else
			memset(lu8tmpArray,0,7); 
				
		lu16tmp = lu8tmpArray[1] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		lu16tmp = lu8tmpArray[1] & 0x0f;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowChar(lu8line,lu8column,0x2e);  //显示小数点
		lu8column += 3;
		lu16tmp = lu8tmpArray[0] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowHZ(lu8line,lu8column,0x01a5);  //显示℃
		lu8column += 5;
		lu8column += 7;  //跳过一部分空显示。
		
		ShowHZ(lu8line,lu8column,0x0123);  //显示濉板"出"
		lu8column += 5;
		lu16tmp = lu8tmpArray[3] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		lu16tmp = lu8tmpArray[3] & 0x0f;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowChar(lu8line,lu8column,0x2e);  //显示小数点
		lu8column += 3;
		lu16tmp = lu8tmpArray[2] >> 4;
		ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		lu8column += 3;
		ShowHZ(lu8line,lu8column,0x01a5);  //显示℃
		lu8column += 5;

//end:显示进回水温度。





//begin:显示累计热量。
	lu8DisplayFlag = 0;
	lu8line = 7;   //从第7行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCZongReLiang,4);
	lu8column += 18;

	lu16tmp = *((uint16 *)(DataBuf + 0));  //取出2个计量点对比是否一致。
	lu16tmp1 = *((uint16 *)(DataBuf + TA_HEATVALVE_SAVEOFFSET - 1));

	 //通过计量点是否相同判断是否下发分摊热量。
	if(lu16tmp == lu16tmp1){
		memcpy(lu8tmpArray,(DataBuf+TA_HEATVALVE_SAVEOFFSET+7-1),4); //找到累计热量。
	}
	else{
		memset(lu8tmpArray,0xee,4);  //如果没下发分摊热量，显示全E.

	}

	for(i=1;i<4;i++){   //累计热量只显示整数部分。
		lu16tmp = lu8tmpArray[4-i] >> 4;
		if(lu8DisplayFlag == 0){
			if(lu16tmp != 0){
				lu8DisplayFlag = 1; 

			if(lu16tmp <= 9)
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			else
				ShowChar(lu8line,lu8column,(lu16tmp+0x37));
		
				lu8column += 3;
			}

		}
		else{
			if(lu16tmp <= 9)
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			else
				ShowChar(lu8line,lu8column,(lu16tmp+0x37));
			lu8column += 3;
		}

		lu16tmp = lu8tmpArray[4-i] & 0x0f;
		if(lu8DisplayFlag == 0){
			if((lu16tmp != 0) || (i==3)){  //如果是最后一个数字，0也要显示。
				lu8DisplayFlag = 1; 
				
			if(lu16tmp <= 9)
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			else
				ShowChar(lu8line,lu8column,(lu16tmp+0x37));
			
				lu8column += 3;
			}

		}
		else{
			if(lu16tmp <= 9)
				ShowChar(lu8line,lu8column,(lu16tmp+0x30));
			else
				ShowChar(lu8line,lu8column,(lu16tmp+0x37));
			lu8column += 3;
		}

	}
	ShowLineChar_HZ(lu8line,lu8column,ZCkWh,3);
//end:显示累计热量。





	

}


/****************************************************************************************************
**	函 数  名 称: LCDDataShow
**	函 数  功 能: LCD屏显，在通断时间面积法中，根据设备类型决定显示
						热表数据或阀控器数据。
**	输 入  参 数: 
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void LCDDataShow(uint16 nowNum,uint16 allNum,uint8 *DataBuf)
{
	uint8 Err = 0;
	uint8 lu8EquipmentType = 0; //设备类型。

	lu8EquipmentType = *(DataBuf + 2);

	if(lu8EquipmentType == HEAT_METER_TYPE)
		MeterDataShow(nowNum,allNum,DataBuf);
	else
		ValveDataShow(nowNum,allNum,DataBuf);
	
	

}


/****************************************************************************************************
**	函 数  名 称: ReadMeterStateShow
**	函 数  功 能: 读取热量表状态
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ReadMeterStateShow(void)
{
	uint8 Err = 0;
	uint8 Res = 0;
	uint8 status;
//	uint8 i;
//	uint8 j = 0;
	uint8 line, column;
//	uint8 len;
//	uint8 DateTemp[6] = {0x00};
//	uint8 ASCIIData[6] = {0x00};
	uint16 lu16ReadCmpNum = 0;
	uint16 lu16MeterNum = 0;  //记录表总数。
	uint16 lu16DisplayNum = 0;  //抄表查询记录显示第几个。
	uint8 lu8UpdataFlag = 1;   //当不在抄表时显示户信息，此变量用于控制是否更新显示。
	CPU_SR		cpu_sr;

	char  DataPath[]="/2012/12/24/1530";
	char  NodePath[]="/2012/12/24/timenode";
	uint8 lu8BCDTime[6] = {0};
	uint8 lu8HexTime[6] = {0};
	DataStoreParaType	History_Para;
	
	line 	= 1;
	column 	= 0;

	
  while(1)
  {
	ShowMenuTop();
	ShowMenuBottom(1);
	
	if(1 == gu8ReadAllMeterFlag){   //如果正在全抄表，则轮显。
	  	display_white();
		ShowMenuTop();
		ShowMenuBottom(1);
		LCDDataShow(gu16ReadMeterSuccessNum,gu16ReadMeterNum,gu8ReadDataStore);   //调用显示。

		lu8UpdataFlag = 1;
	}
	else{	//如果不在全抄表，则可查询。
		if(lu8UpdataFlag){  //lu8UpdataFlag防止在不需要更新的时候读SD卡。
			ReadDateTime(lu8BCDTime);
			BcdTimeToHexTime(lu8BCDTime,lu8HexTime);
			GetFilePath(NodePath, lu8HexTime, ARRAY_DAY);  /*获取固定点timenode path*/
			OSMutexPend (FlashMutex, 0, &Err);
			Res = SDReadData(NodePath, (uint8 *)&History_Para, sizeof(History_Para),0);
			OSMutexPost (FlashMutex);
	
			if(Res == NO_ERR){
				//GetTimeNodeFilePath(DataPath,lu8HexTime,History_Para.TimeNode);
				GetTimeNodeFilePath_Forward(DataPath,lu8HexTime,&History_Para);  //查找给定时间点前一个定时抄数据存放路径。
				lu16MeterNum = History_Para.MeterNums;
			
				OSMutexPend (FlashMutex, 0, &Err);
				Res = SDReadData(DataPath, gu8ReadDataStore, sizeof(gu8ReadDataStore),lu16DisplayNum*128 + 1); //+1是为了错开数据长度字节。
				OSMutexPost (FlashMutex);

				display_white();
				ShowMenuTop();
				ShowMenuBottom(1);
				lu8UpdataFlag = 0; //清除更新显示标记，防止频繁读Flash。
				
				if(Res == NO_ERR)	
					LCDDataShow(lu16DisplayNum+1,lu16MeterNum,gu8ReadDataStore);  //调用显示。+1是为了从1开始显示。
				else
					ShowLineChar_HZ(5,0,ZCNoNodeData,8);//提示没有当前时间数据。
				

			}
			else{
				display_white();
				ShowMenuTop();
				ShowMenuBottom(1);
				lu8UpdataFlag = 0; //清除更新显示标记，防止频繁读Flash。
				ShowLineChar_HZ(4,0,ZCNoSetNode,8);//提示没有设置定时抄表时间
				ShowLineChar_HZ(5,0,ZCThenRe,5);//提示然后重新抄表

			}
			
		}
	
		

	}


	status = GetKeyStatuschaobiao(); //获取键值
   	if( status == KEY_EXIT ){	
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
			//KeyFuncIndex = 2;
		(*KeyTab[KeyFuncIndex].CurrentOperate)();
			
			lu8UpdataFlag = 1;  //当再次进入时使能刷屏。
			lu16DisplayNum = 0;
		break;
	}
	else if(status == KEY_UP){   //当不在抄全表时，上下按键用于切换显示抄表数据。
		if(0 == gu8ReadAllMeterFlag){
			if(lu16DisplayNum == 0)
				lu16DisplayNum = lu16MeterNum - 1;
			else
				lu16DisplayNum -= 1;

			lu8UpdataFlag = 1; //屏幕刷新使能。
		}

	}
	else if(status == KEY_DOWN){
		if(0 == gu8ReadAllMeterFlag){
			if(lu16DisplayNum >= lu16MeterNum-1)
				lu16DisplayNum = 0;
			else
				lu16DisplayNum += 1;

			lu8UpdataFlag = 1; //屏幕刷新使能。

		}

	}

	
  }
	 
}
/****************************************************************************************************
**	函 数  名 称: ReadMeterNow
**	函 数  功 能: 立即抄表
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ReadMeterNow(void)
{
		uint8 SendMsg[2]		= {0x00};

		if((gu8ReadAllMeterFlag!=0) && (gu8ReadAllMeterFlag!=1)){  //防止出错。
			gu8ReadAllMeterFlag = 0;
		}
		
		if(gu8ReadAllMeterFlag == 0){//不在全抄表，则执行。
			SendMsg[0] =  MSG_1D;
			OSQPost(CMD_Q, (void*)SendMsg);
		}
		
		ReadMeterStateShow();
}


/****************************************************************************************************
**	函 数  名 称: MeterFailInfoShow
**	函 数  功 能:显示抄失败表的具体信息。
**	输 入  参 数: 
**	输 出  参 数: MeterOrValve  选择显示热表失败信息或阀控失败信息
							1-Meter,0-Valve.
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void MeterFailInfoShow(uint16 nowNum,uint16 allNum,MeterFileType *p_mf,uint8 MeterOrValve)
{
	uint8 i = 0;
	uint8 lu8line = 0;
	uint8 lu8column = 0;
	uint16 lu16qian = 0;//千位数
	uint16 lu16bai = 0; //百位数
	uint16 lu16shi = 0;
	uint16 lu16ge = 0;
	uint16 lu16tmp = 0;
	uint8 lu8tmp = 0;
	uint8 lu8tmpArray[7] = {0};
	
	if(nowNum > 999)   //防止超限，正常情况下总数不会多于600. 此函数按照3位整数处理。
		nowNum = 999;
	if(allNum > 999)
		allNum = 999;
	
	//begin:显示第几个/共几个  失败。
	lu16bai = nowNum / 100;
	lu16shi = (nowNum % 100) / 10;
	lu16ge = nowNum % 10;

	lu8line = 1;   //从第1行第0列开始。
	lu8column = 0;

	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; //每个ASCII码占3列。
	}

	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}

	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;
	
	ShowChar(lu8line,lu8column,0x2f);  //显示斜线。
	lu8column += 3;

	lu16bai = allNum / 100;
	lu16shi = (allNum - lu16bai*100) / 10;
	lu16ge = allNum % 10;

	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; 
	}

	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}

	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;
//end:显示第几个/共几个  失败。
	
//begin: 显示当前表的ID号"计量点号"
	lu8line = 2;   //从第2行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCJiLiangDian,4);
	lu8column += 18;

	lu16tmp = p_mf->MeterID;

	lu16bai = lu16tmp / 100;
	lu16shi = (lu16tmp % 100) / 10;
	lu16ge = lu16tmp % 10;
	
	if(lu16bai){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3; //每个ASCII码占3列。
	}
	
	if(lu16shi || lu16bai){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3; 
	}
	
	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;
	
//end: 显示当前表的ID号"计量点号"


//begin:显示热表号或阀控编号
	lu8line = 3;   //从第3行第0列开始。
	lu8column = 0;

	if(MeterOrValve == 1){  //显示热表号
		ShowLineChar_HZ(lu8line,lu8column,ZCReBiaoHao,4);
		memcpy(lu8tmpArray,&(p_mf->MeterAddr[0]),7); //找到热表编号。
	}
	else{	//显示阀控号。
		ShowLineChar_HZ(lu8line,lu8column,ZCFaKongHao,4);
		memcpy(lu8tmpArray,&(p_mf->ValveAddr[0]),7); //找到热表编号.

	}
	
	lu8column += 18;
	for(i=0;i<4;i++){
		lu16tmp = lu8tmpArray[3-i] >> 4;
		if(lu16tmp <= 9)
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		else
			ShowChar(lu8line,lu8column,(lu16tmp+0x37));
		
		lu8column += 3;
		lu16tmp = lu8tmpArray[3-i] & 0x0f;
		if(lu16tmp <= 9)
			ShowChar(lu8line,lu8column,(lu16tmp+0x30));
		else
			ShowChar(lu8line,lu8column,(lu16tmp+0x37));

		lu8column += 3;
	}

//end:显示热表号或阀控编号

//begin:显示通道号
	lu8line = 4;   //从第4行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCTongDaoHao,4);
	lu8column += 18;

	lu8tmp = p_mf->ChannelIndex;

	lu16tmp = lu8tmp >> 4;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;
	lu16tmp = lu8tmp & 0x0f;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;
//end:显示通道号

//begin:显示住户信息
	lu8line = 5;   //从第5行第0列开始。
	lu8column = 0;
	ShowLineChar_HZ(lu8line,lu8column,ZCZhuHuXinXi,5);
	lu8column += 23;

	lu8tmp = p_mf->BulidID;

	lu16tmp = lu8tmp >> 4;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;
	lu16tmp = lu8tmp & 0x0f;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;

	ShowChar(lu8line,lu8column,0x2d);  //显示短横线。
	lu8column += 3;

	lu8tmp = p_mf->UnitID;

	lu16tmp = lu8tmp >> 4;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;
	lu16tmp = lu8tmp & 0x0f;
	ShowChar(lu8line,lu8column,(lu16tmp+0x30));
	lu8column += 3;

	ShowChar(lu8line,lu8column,0x2d);  //显示短横线。
	lu8column += 3;

	lu16tmp = p_mf->RoomID;
	if(lu16tmp > 9999)  //最大允许4位整数。
		lu16tmp = 9999;
	
	lu16qian = lu16tmp / 1000;
	lu16bai = (lu16tmp % 1000) / 100;
	lu16shi = (lu16tmp % 100) / 10;
	lu16ge = lu16tmp % 10;
	if(lu16qian){
		ShowChar(lu8line,lu8column,(lu16qian+0x30));
		lu8column += 3;
	}
	if(lu16bai || lu16qian){
		ShowChar(lu8line,lu8column,(lu16bai+0x30));
		lu8column += 3;
	}
	if(lu16shi || lu16bai || lu16qian){
		ShowChar(lu8line,lu8column,(lu16shi+0x30));
		lu8column += 3;
	}
	ShowChar(lu8line,lu8column,(lu16ge+0x30));
	lu8column += 3;

//end:显示住户信息





}

/****************************************************************************************************
**	函 数  名 称:ShowMeterFail
**	函 数  功 能: 显示选中查看 热表失败查询
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ShowMeterFail(void)
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(2,3,&ZCMeterFail[0],6);  //热表失败查询
	ShowLineChar_HZ(3,3,&ZCValveFail[0],6);  //阀控失败查询
	ReverseShowLine(2);	

}

/****************************************************************************************************
**	函 数  名 称:ShowValveFail
**	函 数  功 能: 显示选中查看阀控失败查询
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ShowValveFail(void)
{
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(2,3,&ZCMeterFail[0],6);  //热表失败查询
	ShowLineChar_HZ(3,3,&ZCValveFail[0],6);  //阀控失败查询
	ReverseShowLine(3);	

}



/****************************************************************************************************
**	函 数  名 称: InquiryMeterFailInfo
**	函 数  功 能: 抄表失败信息查询
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
static uint16 gsu16FailMeterArray[METER_NUM_MAX] = {0};
static uint16 gsu16MeterChannelNum[METER_CHANNEL_NUM] = {0};
static uint16 gsu16MeterChannel[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];
static uint8 gsu8MeterFailed[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];

void InquiryMeterFailInfo(void)
{
	uint8 Err;
	uint8 Res;
	uint8 status;
	uint8 i = 0;
	uint8 j = 0;
	uint16 lu16FailMeterNum = 0;  //记录抄表失败的表总数。
	uint16 lu16DisplayNum = 0;  //抄表查询记录显示第几个。
    uint8 lu8UpdataFlag = 1;   //此变量用于控制是否更新显示。
	uint16 lu16MeterSn = 0;
	MeterFileType	mf;


	CPU_SR		cpu_sr;
	
		
	ShowMenuTop();
	ShowMenuBottom(1);

//begin:查询抄表失败信息。

	//从SD卡读出失败相关信息。
	
	OSMutexPend (FlashMutex, 0, &Err);
	for(i=0;i<METER_CHANNEL_NUM;i++){
		Res = SDReadData("/METER_Failed_Info", (uint8 *)(&gsu8MeterFailed[i][0]),METER_PER_CHANNEL_NUM,i*METER_PER_CHANNEL_NUM);
	}
	OSMutexPost (FlashMutex);

	OS_ENTER_CRITICAL();
	memcpy(gsu16MeterChannelNum,gPARA_MeterChannelNum,sizeof(gsu16MeterChannelNum));
	memcpy(gsu16MeterChannel,gPARA_MeterChannel,sizeof(gsu16MeterChannel));
	OS_EXIT_CRITICAL();

	for(i=0;i<METER_CHANNEL_NUM;i++){  //依次查询7个通道的失败情况。

		if(gsu16MeterChannelNum[i] != 0){ //只有本通道有表才查询。
			for(j=0;j<gsu16MeterChannelNum[i];j++){
				if(gsu8MeterFailed[i][j] == 0){	//0说明抄表不成功
					gsu16FailMeterArray[lu16FailMeterNum++] = gsu16MeterChannel[i][j];
				}

			}
		}

	}


	

//end:查询抄表失败信息。

		
	while(1){
		if(1 == lu8UpdataFlag){  //刷新屏幕显示。
			if(lu16FailMeterNum > 0){ //有失败存在时才执行。
				//通过SN号查询表基础信息。
				lu16MeterSn = gsu16FailMeterArray[lu16DisplayNum];
				PARA_ReadMeterInfo(lu16MeterSn, &mf);

				display_white();
				ShowMenuTop();
				ShowMenuBottom(1);
				MeterFailInfoShow(lu16DisplayNum+1,lu16FailMeterNum,&mf,1);

			}
			else{  //没有失败。
				display_white();
				ShowMenuTop();
				ShowMenuBottom(1);
				memset(&mf,0,sizeof(mf));   //没有失败的信息，全清0.
				MeterFailInfoShow(0,0,&mf,1);

			}

			lu8UpdataFlag = 0;

		}


		status = GetKeyStatuschaobiao(); //获取键值
		
   		if(status == KEY_EXIT){	
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
			(*KeyTab[KeyFuncIndex].CurrentOperate)();
			lu8UpdataFlag = 1;  //当再次进入时使能刷屏。
			lu16DisplayNum = 0;
			break;
		}
		else if(status == KEY_UP){ 
				if(lu16DisplayNum == 0)
					lu16DisplayNum = lu16FailMeterNum - 1;
				else
					lu16DisplayNum -= 1;

				lu8UpdataFlag = 1; //屏幕刷新使能。

		}
		else if(status == KEY_DOWN){
				if(lu16DisplayNum >= lu16FailMeterNum-1)
					lu16DisplayNum = 0;
				else
					lu16DisplayNum += 1;

				lu8UpdataFlag = 1; //屏幕刷新使能。

		}

		  
	}




}


/****************************************************************************************************
**	函 数  名 称: InquiryValveFailInfo
**	函 数  功 能: 抄阀失败信息查询
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void InquiryValveFailInfo(void)
{
	uint8 Err;
	uint8 Res;
	uint8 status;
	uint8 i = 0;
	uint8 j = 0;
	uint16 lu16FailMeterNum = 0;  //记录抄表失败的表总数。
	uint16 lu16DisplayNum = 0;  //抄表查询记录显示第几个。
    uint8 lu8UpdataFlag = 1;   //此变量用于控制是否更新显示。
	uint16 lu16MeterSn = 0;
	MeterFileType	mf;


	CPU_SR		cpu_sr;
	
		
	ShowMenuTop();
	ShowMenuBottom(1);

//begin:查询抄表失败信息。

	//从SD卡读出失败相关信息。
	
	OSMutexPend (FlashMutex, 0, &Err);
	for(i=0;i<METER_CHANNEL_NUM;i++){
		Res = SDReadData("/METER_Failed_Info", (uint8 *)(&gsu8MeterFailed[i][0]),METER_PER_CHANNEL_NUM,i*METER_PER_CHANNEL_NUM);
	}
	OSMutexPost (FlashMutex);

	OS_ENTER_CRITICAL();
	memcpy(gsu16MeterChannelNum,gPARA_MeterChannelNum,sizeof(gsu16MeterChannelNum));
	memcpy(gsu16MeterChannel,gPARA_MeterChannel,sizeof(gsu16MeterChannel));
	OS_EXIT_CRITICAL();

	for(i=0;i<METER_CHANNEL_NUM;i++){  //依次查询7个通道的失败情况。

		if(gsu16MeterChannelNum[i] != 0){ //只有本通道有表才查询。
			for(j=0;j<gsu16MeterChannelNum[i];j++){
				if(gsu8MeterFailed[i][j] == 0xbb){	//bb说明抄阀失败。
					gsu16FailMeterArray[lu16FailMeterNum++] = gsu16MeterChannel[i][j];
				}

			}
		}

	}


	

//end:查询抄表失败信息。

		
	while(1){
		if(1 == lu8UpdataFlag){  //刷新屏幕显示。
			if(lu16FailMeterNum > 0){ //有失败存在时才执行。
				//通过SN号查询表基础信息。
				lu16MeterSn = gsu16FailMeterArray[lu16DisplayNum];
				PARA_ReadMeterInfo(lu16MeterSn, &mf);

				display_white();
				ShowMenuTop();
				ShowMenuBottom(1);
				MeterFailInfoShow(lu16DisplayNum+1,lu16FailMeterNum,&mf,0);

			}
			else{  //没有失败。
				display_white();
				ShowMenuTop();
				ShowMenuBottom(1);
				memset(&mf,0,sizeof(mf));   //没有失败的信息，全清0.
				MeterFailInfoShow(0,0,&mf,0);

			}

			lu8UpdataFlag = 0;

		}


		status = GetKeyStatuschaobiao(); //获取键值
		
   		if(status == KEY_EXIT){	
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
			(*KeyTab[KeyFuncIndex].CurrentOperate)();
			lu8UpdataFlag = 1;  //当再次进入时使能刷屏。
			lu16DisplayNum = 0;
			break;
		}
		else if(status == KEY_UP){ 
				if(lu16DisplayNum == 0)
					lu16DisplayNum = lu16FailMeterNum - 1;
				else
					lu16DisplayNum -= 1;

				lu8UpdataFlag = 1; //屏幕刷新使能。

		}
		else if(status == KEY_DOWN){
				if(lu16DisplayNum >= lu16FailMeterNum-1)
					lu16DisplayNum = 0;
				else
					lu16DisplayNum += 1;

				lu8UpdataFlag = 1; //屏幕刷新使能。

		}

		  
	}




}




#ifdef DEMO_APP
uint8 Sort_ID(uint8 ID)//查找ID 在数组中的位置
{
   uint8 num=0;
   
   uint8 i=0;
	num=gPARA_MeterChannelNum[gPARA_MeterFiles[ID].ChannelIndex-1];
    for(i=0;i<num;i++)
    	{
          if(gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][i]==ID)
            
            return i;
	    }
  return 0xFF;

}
void Pull_out_OnePara( uint8 ID)
{
   uint8 Index=0;
   
   uint8 i=0;
   Index=Sort_ID(ID);
   if(Index==0xFF) return;
   if(Index==0)
   	gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][0]=0;
   else
   	{
        for(i=Index;i<gPARA_MeterChannelNum[gPARA_MeterFiles[ID].ChannelIndex-1];i++)   
        	{
     
            	
            	gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][i-1]=gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][i];


        	}
		
		    gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][gPARA_MeterChannelNum[gPARA_MeterFiles[ID].ChannelIndex-1]-1]=0;
   }
}
/****************************************************************************************************
**	函 数  名 称: SetMeterAddr
**	函 数  功 能: 
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 SetMeterAddr(uint8 *pAddr,uint8 CH, uint8 Ver ,uint8 ID)
{
	uint8 Err = NO_ERR;
	TermParaSaveType TermPara;
	CPU_SR			cpu_sr;
//    uint8 i=0;
//	uint8 diff=0;
	uint8 SendNum =3;//默认表数量是3个
	MeterFilePara para;
	#if 0
	for(i=0;i<7;i++)
	  {
		 if(gPARA_MeterFiles[ID].MeterAddr[i]!=pAddr[i])
		  {
		  diff=1;
		  break;
		  
		  }
		  
		  
	  }
	#endif
    if( gPARA_MeterFiles[ID].ChannelIndex!=CH)
    	{

		#if 1
		   if(gPARA_MeterChannelNum[gPARA_MeterFiles[ID].ChannelIndex-1]==3)//下面的代码就是实现了一个从原队列中抽取一个的功能
		   	{
              if(ID==0)
              	{
			  	gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][0]=gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1];
			  	gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1]=gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][2];
                gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][2]=0;
			    }
			  else if(ID==1)
              	{
			  	gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1]=gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][2];
                gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][2]=0;
			    }
			  else if(ID==2)
			  	{
                  
				  gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][2]=0;
			    }
			  	
			  }
		   	else   if(gPARA_MeterChannelNum[gPARA_MeterFiles[ID].ChannelIndex-1]==2)
		   	{
		   	  if(ID==0)
			  	
			  {
			  gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][0]=gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1];
			  gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1]=0;
			  }
			  else if(ID==1)
              	{
              	  if(gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][2]>0)//说明后面没有数据了ID ==1是第二位
              	  
				  gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1]=0;
				  else
				  	{
              	  
        			  	gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][0]=gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1];
                        gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1]=0;
 				  	}
			    }
			  else if(ID==2)
			  	{
                  
				  gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][1]=0;
			    }
			  	
			  }
		   else if(gPARA_MeterChannelNum[gPARA_MeterFiles[ID].ChannelIndex-1]==1)
		   	  {
                 
				 gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][0]=0;
		      }
		   	#else
			Pull_out_OnePara(ID);
			#endif
			gPARA_MeterChannelNum[gPARA_MeterFiles[ID].ChannelIndex-1]--;//原来通道CH	   表的数量
		  // gPARA_MeterChannel[gPARA_MeterFiles[ID].ChannelIndex-1][ID] = 0;
		   
		   gPARA_MeterFiles[ID].ChannelIndex = CH;

		   
		   gPARA_MeterFiles[ID].ProtocolVer = Ver;	
		   
		   memcpy(gPARA_MeterFiles[ID].MeterAddr, pAddr, 7);
		   
		   gPARA_MeterChannelNum[CH-1]++;//新通道CH	 表的数量
		   
		   gPARA_MeterChannel[CH-1][gPARA_MeterChannelNum[CH-1]-1] = ID;
	    }
	#if 0
     else if((gPARA_MeterFiles[ID].ProtocolVer!=Ver) && (diff ==0))
     	{
          
		  gPARA_MeterFiles[ID].ProtocolVer = Ver;  
	    }
	#endif 
	 else
	 	{
           
		//  if (diff)	
		
		  gPARA_MeterFiles[ID].ProtocolVer = Ver;  
		  memcpy(gPARA_MeterFiles[ID].MeterAddr, pAddr, 7);
	    }

/*   写卡操作                          */	 
	 para.WriteFlag = 0xAA55;
	 para.DEMOWriteFlag= 0xAA55;

	 OS_ENTER_CRITICAL();	 
	 para.MeterStoreNums = SendNum;
	 /*begin:yangfei changed 2012-12-15 */
	// memcpy(SystemTime, gSystemTime, 6);
	 /*end	 :yangfei changed 2012-12-15 */
	 OS_EXIT_CRITICAL();
	 //存储表档案信息索引
	 OSMutexPend (FlashMutex,0,&Err);
	 
	 Err = SDSaveData("/METER_FILE_ADDR", gPARA_MeterFiles, sizeof(MeterFileType)*SendNum, 0);	 
	 /*begin:yangfei changed 2012-12-15 for 表档案存储*/
	 Err = SDSaveData("/METER_FILE_PARA", &para, sizeof(MeterFilePara), 0);
	// GetFilePath(NodePath, SystemTime, ARRAY_DAY);
	// Err = SDSaveData(NodePath, &gPARA_MeterNum, sizeof(gPARA_MeterNum), 6);
	 /*end:yangfei changed 2012-12-15 for 表档案存储*/
	
	Err = SDSaveData("/METER_FILE_DEMO", gPARA_MeterFiles, sizeof(MeterFileType)*SendNum, 0);	
	 OSMutexPost (FlashMutex);
	/*	 写卡操作						   */	 


	Demo_param_set =1;

	

	return Err;
}


/****************************************************************************************************
**	函 数  名 称: FreshMeterAddrSet
**	函 数  功 能: 刷新项目地址设置
**	输 入  参 数: uint8* InputBuff
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: muxiaoqing modify protocal
*****************************************************************************************************/
void FreshMeterAddrSet(uint8* MeterAddress,uint8* channle,uint8* Ver)
{
	uint8 i,j,line,column;
#if 0
    LOG_assert_param( InputBuff == NULL );

#else
LOG_assert_param( MeterAddress == NULL );
#endif
	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(21);
	ShowLineChar_HZ(1,20,&MenuHZ16_21_31[0],2);		//德鲁
	
	ShowLineChar_HZ(2,0,&Meter_ST_MeterNum[0],2);	//楼号:	//区号
	
	ShowLineChar_HZ(3,0,&Meter_ST_CH[0],2);	//单元:	//楼号
	
	ShowLineChar_HZ(4,0,&IPVer[0],3);		//协议
	//ShowPicture(48,22,16,32,&MenuPic_OK[0]);//3行 OK
	
	ShowPicture(80,22,16,32,&MenuPic_OK[0]);//5行 OK
	#if 0
	line = 2;
	column = 20;
	for(i=0;i<4;i++)
	{
		ShowChar(line,column+3*i,InputBuff[i]);
	}	
	#endif
	
	line = 2;
	column = 10;
	
	for(i=0;i<14;i++)//显示14位表地址
	{ 
        		ShowChar(line,column+3*i,MeterAddress[i]);
	}	
	
        		ShowChar(line+1,column,channle[0]);//显示通道号
        		
        		ShowChar(line+2,column,Ver[0]);//显示协议号
	return;
}

/****************************************************************************************************
**	函 数  名 称: Set_MeterParam_1
**	函 数  功 能: 热表参数设置
**	输 入  参 数: none
**	输 出  参 数: 无					                          
**  返   回   值: 无
**	备		  注: 慕肖清添加 演示用
*****************************************************************************************************/
void Set_MeterParam_1(void)// 热表参数设置
{
	uint8 status;
	uint8 line;
	uint8 column;
	uint8 column1;
	uint8 line1;
	
	uint8 column2;
	uint8 line2;
	
	uint8 column3;
	uint8 line3;
	int8 i;

	uint8 ASCIIMeterAddress[14];	
	uint8 ASCIIChannle[1];	
	uint8 ASCIIVer[1];	

	uint8 TermAddr[7] = {0};
//	uint8 TermAddrReverse[6] = {0x00};
	
//	MeterFilePara	para;
//	uint8 Err	= 0;
//	uint8 datalen;
#if 0	
	SelectInfo TabMenu[] = {{2,20,12},{3,22,11}};//行，列，长度
#else
   // SelectInfo TabMenu[] = {{2,25,13},{3,25,13},{4,25,13},{5,22,11}};//行，列，长度
	SelectInfo TabMenu[] = {{2,10,43},{3,10,13},{4,10,13},{5,22,11}};//行，列，长度
	
#endif
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
	CPU_SR			cpu_sr;
				
	uint8 addrtemp_DeLu[7] = {0x09, 0x78, 0x13, 0x21, 0x00, 0x11, 0x11};
	uint8 addrtemp_LanJE[7] = {0x35, 0x25, 0x10, 0x67, 0x00, 0x00, 0x00};
	uint8 addrtemp_TG[7] = {0x22, 0x17, 0x05, 0x14, 0x00, 0x11, 0x11};

	uint8 Mertersn_ID =0;
#if 1
  if(Demo_param_set==0)
  	{

	//Err = SDReadData("/METER_FILE_PARA", &para, sizeof(MeterFilePara), 0);
	
	//if(para.DEMOWriteFlag== 0xAA55)
	
	if(DemoDateIsGet)
		{
	
		   
		
		}
	else
     		{
     
     	
         memset((uint8 *)gPARA_MeterFiles, 0x00, sizeof(gPARA_MeterFiles));
         memset((uint8 *)gPARA_MeterChannel, 0x00, sizeof(gPARA_MeterChannel));
         memset((uint8 *)gPARA_Meter_Failed, 0x00, sizeof(gPARA_Meter_Failed));
         memset((uint8 *)gPARA_MeterChannelNum, 0x00, sizeof(gPARA_MeterChannelNum));
         memset((uint8 *)gPARA_ConPanelChannel, 0x00, sizeof(gPARA_ConPanelChannel));
         memset((uint8 *)gPARA_ConPanelChannelNum, 0x00, sizeof(gPARA_ConPanelChannelNum));
         memset((uint8 *)gPARA_ValveConChannel, 0x00, sizeof(gPARA_ValveConChannel));
         memset((uint8 *)gPARA_ValveConChannelNum, 0x00, sizeof(gPARA_ValveConChannelNum));
         gPARA_MeterFiles[0].ChannelIndex = 3;
         gPARA_MeterFiles[0].ProtocolVer = 0;  
         memcpy(gPARA_MeterFiles[0].MeterAddr, addrtemp_DeLu, 7);  
      
      
         gPARA_MeterFiles[1].ChannelIndex = 3;
         gPARA_MeterFiles[1].ProtocolVer = 8;  
         memcpy(gPARA_MeterFiles[1].MeterAddr, addrtemp_LanJE, 7);	 
      
         gPARA_MeterFiles[2].ChannelIndex = 3;
         gPARA_MeterFiles[2].ProtocolVer = 0;  
         memcpy(gPARA_MeterFiles[2].MeterAddr, addrtemp_TG, 7);	 
     			
     				   
             #if 0//每个通道一个表的时候
             				 for(i=0;i<3;i++)
             					{
             					  gPARA_MeterChannelNum[i]=1;
             					  gPARA_MeterChannel[i][0] = i;
             					  gPARA_ConPanelChannel[i][0] = i;
             					  gPARA_ConPanelChannelNum[i]=1;
             					  gPARA_ValveConChannelNum[i]=1;
             					  gPARA_ValveConChannel[i][0] = i;
             					}
             #else 
             			gPARA_MeterChannelNum[2]=3;//通道3	  表的数量
             			for(i=0;i<3;i++)
             			{
             			  gPARA_MeterChannel[2][i] = i;
             			}
             #endif
     		
     				 gPARA_MeterNum = 3;
       	}	
  	}
#endif
	
	
	
	//由于存储低字节在前，所以现在将字节顺序调换
	for(i=0; i<7; i++)
	{
		TermAddr[i] = gPARA_MeterFiles[Mertersn_ID].MeterAddr[6-i];
	}
#if 0	
	//取调换顺序后的后2字节有效
	for(i=0;i<2;i++)
	{
		ASCIIData[2*i] = HexToAscii(TermAddr[4+i]>>4);
		ASCIIData[2*i+1] = HexToAscii(TermAddr[4+i]&0x0f);
	}
#else
	
//取调换顺序后的后2字节有效
for(i=0;i<7;i++)
{
	ASCIIMeterAddress[2*i] = HexToAscii(TermAddr[i]>>4);
	ASCIIMeterAddress[2*i+1] = HexToAscii(TermAddr[i]&0x0f);
}
 ASCIIChannle[0] = HexToAscii(gPARA_MeterFiles[Mertersn_ID].ChannelIndex);
 ASCIIVer[0]=HexToAscii(gPARA_MeterFiles[Mertersn_ID].ProtocolVer) ;
#endif
	i = 0;
	line = TabMenu[0].startline;
	column = TabMenu[0].startcolumn;
	line1 = TabMenu[1].startline;
	column1 = TabMenu[1].startcolumn;
	
	line2 = TabMenu[2].startline;
	column2 = TabMenu[2].startcolumn;
	
	line3 = TabMenu[3].startline;
	column3 = TabMenu[3].startcolumn;
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
	   #if 0
		FreshTermAddrSet(&ASCIIData[0]);
	   #else	   
	   FreshMeterAddrSet(&ASCIIMeterAddress[0],&ASCIIChannle[0],&ASCIIVer[0]);
	   #endif
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
						KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
						//KeyFuncIndex = 2;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else if(TabNum == 0)		
		{
			ReverseShowChar(line,column);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIMeterAddress[i] >= 0x39)
					{
						ASCIIMeterAddress[i] = 0x30;
					}
					else
					{
						ASCIIMeterAddress[i] = ASCIIMeterAddress[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIMeterAddress[i] <= 0x30)||(ASCIIMeterAddress[i] >0x39))
					{
						ASCIIMeterAddress[i] = 0x39;
					}
					else
					{
						ASCIIMeterAddress[i] = ASCIIMeterAddress[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line,column,ASCIIMeterAddress[i]);
					if((i > 0)&&(i<14))
					{
						i = i - 1;
						column = column - 3;
					}
					else
						{
                          i =0;
						  column = TabMenu[0].startcolumn;
					    }
					break;
				case KEY_RIGHT://向右键
					ShowChar(line,column,ASCIIMeterAddress[i]);
					if(i < 13)
					{
						i = i + 1;
						column = column + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		
		else if(TabNum == 1)		
		{
			ReverseShowChar(line1,column1);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIChannle[i] >= 0x39)
					{
						ASCIIChannle[i] = 0x30;
					}
					else
					{
						ASCIIChannle[i] = ASCIIChannle[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIChannle[i] <= 0x30)||(ASCIIChannle[i] >0x39))
					{
						ASCIIChannle[i] = 0x39;
					}
					else
					{
						ASCIIChannle[i] = ASCIIChannle[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line1,column1,ASCIIChannle[0]);
					#if 0
					if(i > 0)
					{
						i = i - 1;
						column1 = column1 - 3;
					}
					#endif
					break;
				case KEY_RIGHT://向右键
					ShowChar(line1,column1,ASCIIChannle[0]);
					#if 0
					if(i < 3)
					{
						i = i + 1;
						column1 = column1 + 3;
					}
					#endif
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		else if(TabNum ==2)
		{
			ReverseShowChar(line2,column2);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIVer[i] >= 0x39)
					{
						ASCIIVer[i] = 0x30;
					}
					else
					{
						ASCIIVer[i] = ASCIIVer[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIVer[i] <= 0x30)||(ASCIIVer[i] >0x39))
					{
						ASCIIVer[i] = 0x39;
					}
					else
					{
						ASCIIVer[i] = ASCIIVer[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line2,column2,ASCIIVer[0]);
					break;
				case KEY_RIGHT://向右键
					ShowChar(line2,column2,ASCIIVer[0]);
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}

	}//end while

	memset(TermAddr, 0x00, 7); //

for(i=0;i<7;i++)
{
	TermAddr[6-i] =  ((ASCIIMeterAddress[2*i]-0x30)<<4) + ((ASCIIMeterAddress[2*i+1]-0x30)&0x0F);
}


	SetMeterAddr(TermAddr,(ASCIIChannle[0]-0x30),(ASCIIVer[0]-0x30),Mertersn_ID); //
	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;
}


/****************************************************************************************************
**	函 数  名 称: Set_MeterParam_2
**	函 数  功 能: 热表参数设置
**	输 入  参 数: none
**	输 出  参 数: 无					                          
**  返   回   值: 无
**	备		  注: 慕肖清添加 演示用
*****************************************************************************************************/
void Set_MeterParam_2(void)// 热表参数设置
{
	uint8 status;
	uint8 line;
	uint8 column;
	uint8 column1;
	uint8 line1;
	
	uint8 column2;
	uint8 line2;
	
	uint8 column3;
	uint8 line3;
	int8 i;

	uint8 ASCIIMeterAddress[14];	
	uint8 ASCIIChannle[1];	
	uint8 ASCIIVer[1];	

	uint8 TermAddr[7] = {0};
//	uint8 TermAddrReverse[6] = {0x00};
	
//	MeterFilePara	para;
//	uint8 Err	= 0;
//	uint8 datalen;
#if 0	
	SelectInfo TabMenu[] = {{2,20,12},{3,22,11}};//行，列，长度
#else
   // SelectInfo TabMenu[] = {{2,25,13},{3,25,13},{4,25,13},{5,22,11}};//行，列，长度
	SelectInfo TabMenu[] = {{2,10,43},{3,10,13},{4,10,13},{5,22,11}};//行，列，长度
	
#endif
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
	CPU_SR			cpu_sr;
				
	uint8 addrtemp_DeLu[7] = {0x09, 0x78, 0x13, 0x21, 0x00, 0x11, 0x11};
	uint8 addrtemp_LanJE[7] = {0x35, 0x25, 0x10, 0x67, 0x00, 0x00, 0x00};
	uint8 addrtemp_TG[7] = {0x22, 0x17, 0x05, 0x14, 0x00, 0x11, 0x11};

	uint8 Mertersn_ID =1;
#if 1
	  if(Demo_param_set==0)
		{

	//	Err = SDReadData("/METER_FILE_PARA", &para, sizeof(MeterFilePara), 0);
		
	//	if(para.DEMOWriteFlag== 0xAA55)
	
	if(DemoDateIsGet)
			{
		
			   
			
			}
		else
     			{
     
     		
     		memset((uint8 *)gPARA_MeterFiles, 0x00, sizeof(gPARA_MeterFiles));
     		memset((uint8 *)gPARA_MeterChannel, 0x00, sizeof(gPARA_MeterChannel));
     		memset((uint8 *)gPARA_Meter_Failed, 0x00, sizeof(gPARA_Meter_Failed));
     		memset((uint8 *)gPARA_MeterChannelNum, 0x00, sizeof(gPARA_MeterChannelNum));
     		memset((uint8 *)gPARA_ConPanelChannel, 0x00, sizeof(gPARA_ConPanelChannel));
     		memset((uint8 *)gPARA_ConPanelChannelNum, 0x00, sizeof(gPARA_ConPanelChannelNum));
     		memset((uint8 *)gPARA_ValveConChannel, 0x00, sizeof(gPARA_ValveConChannel));
     		memset((uint8 *)gPARA_ValveConChannelNum, 0x00, sizeof(gPARA_ValveConChannelNum));
     	 
     		gPARA_MeterFiles[0].ChannelIndex = 3;
     		gPARA_MeterFiles[0].ProtocolVer = 0;  
     		memcpy(gPARA_MeterFiles[0].MeterAddr, addrtemp_DeLu, 7);  
     	 
     	 
     		gPARA_MeterFiles[1].ChannelIndex = 3;
     		gPARA_MeterFiles[1].ProtocolVer = 8;  
     		memcpy(gPARA_MeterFiles[1].MeterAddr, addrtemp_LanJE, 7);	 
     	 
     		gPARA_MeterFiles[2].ChannelIndex = 3;
     		gPARA_MeterFiles[2].ProtocolVer = 0;  
     		memcpy(gPARA_MeterFiles[2].MeterAddr, addrtemp_TG, 7);	 
     				
     					   
             #if 0//每个通道一个表的时候
     							 for(i=0;i<3;i++)
     								{
     								  gPARA_MeterChannelNum[i]=1;
     								  gPARA_MeterChannel[i][0] = i;
     								  gPARA_ConPanelChannel[i][0] = i;
     								  gPARA_ConPanelChannelNum[i]=1;
     								  gPARA_ValveConChannelNum[i]=1;
     								  gPARA_ValveConChannel[i][0] = i;
     								}
             #else 
     						gPARA_MeterChannelNum[2]=3;//通道1	  表的数量
     						for(i=0;i<3;i++)
     						{
     						  gPARA_MeterChannel[2][i] = i;
     						}
             #endif
     			
     					 gPARA_MeterNum = 3;
     		}	
	  	}
#endif
	
	
	
	//由于存储低字节在前，所以现在将字节顺序调换
	for(i=0; i<7; i++)
	{
		TermAddr[i] = gPARA_MeterFiles[Mertersn_ID].MeterAddr[6-i];
	}
#if 0	
	//取调换顺序后的后2字节有效
	for(i=0;i<2;i++)
	{
		ASCIIData[2*i] = HexToAscii(TermAddr[4+i]>>4);
		ASCIIData[2*i+1] = HexToAscii(TermAddr[4+i]&0x0f);
	}
#else
	
//取调换顺序后的后2字节有效
for(i=0;i<7;i++)
{
	ASCIIMeterAddress[2*i] = HexToAscii(TermAddr[i]>>4);
	ASCIIMeterAddress[2*i+1] = HexToAscii(TermAddr[i]&0x0f);
}
 ASCIIChannle[0] = HexToAscii(gPARA_MeterFiles[Mertersn_ID].ChannelIndex);
 ASCIIVer[0]=HexToAscii(gPARA_MeterFiles[Mertersn_ID].ProtocolVer) ;
#endif
	i = 0;
	line = TabMenu[0].startline;
	column = TabMenu[0].startcolumn;
	line1 = TabMenu[1].startline;
	column1 = TabMenu[1].startcolumn;
	
	line2 = TabMenu[2].startline;
	column2 = TabMenu[2].startcolumn;
	
	line3 = TabMenu[3].startline;
	column3 = TabMenu[3].startcolumn;
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
	   #if 0
		FreshTermAddrSet(&ASCIIData[0]);
	   #else	   
	   FreshMeterAddrSet(&ASCIIMeterAddress[0],&ASCIIChannle[0],&ASCIIVer[0]);
	   #endif
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
						KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
						//KeyFuncIndex = 2;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else if(TabNum == 0)		
		{
			ReverseShowChar(line,column);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIMeterAddress[i] >= 0x39)
					{
						ASCIIMeterAddress[i] = 0x30;
					}
					else
					{
						ASCIIMeterAddress[i] = ASCIIMeterAddress[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIMeterAddress[i] <= 0x30)||(ASCIIMeterAddress[i] >0x39))
					{
						ASCIIMeterAddress[i] = 0x39;
					}
					else
					{
						ASCIIMeterAddress[i] = ASCIIMeterAddress[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line,column,ASCIIMeterAddress[i]);
					if((i > 0)&&(i<14))
					{
						i = i - 1;
						column = column - 3;
					}
					else
						{
                          i =0;
						  column = TabMenu[0].startcolumn;
					    }
					break;
				case KEY_RIGHT://向右键
					ShowChar(line,column,ASCIIMeterAddress[i]);
					if(i < 13)
					{
						i = i + 1;
						column = column + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		
		else if(TabNum == 1)		
		{
			ReverseShowChar(line1,column1);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIChannle[i] >= 0x39)
					{
						ASCIIChannle[i] = 0x30;
					}
					else
					{
						ASCIIChannle[i] = ASCIIChannle[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIChannle[i] <= 0x30)||(ASCIIChannle[i] >0x39))
					{
						ASCIIChannle[i] = 0x39;
					}
					else
					{
						ASCIIChannle[i] = ASCIIChannle[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line1,column1,ASCIIChannle[0]);
					break;
				case KEY_RIGHT://向右键
					ShowChar(line1,column1,ASCIIChannle[0]);
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		else if(TabNum ==2)
		{
			ReverseShowChar(line2,column2);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIVer[i] >= 0x39)
					{
						ASCIIVer[i] = 0x30;
					}
					else
					{
						ASCIIVer[i] = ASCIIVer[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIVer[i] <= 0x30)||(ASCIIVer[i] >0x39))
					{
						ASCIIVer[i] = 0x39;
					}
					else
					{
						ASCIIVer[i] = ASCIIVer[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line2,column2,ASCIIVer[0]);
					break;
				case KEY_RIGHT://向右键
					ShowChar(line2,column2,ASCIIVer[0]);
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}

	}//end while

	memset(TermAddr, 0x00, 7); //

	
	for(i=0;i<7;i++)
	{
		TermAddr[6-i] =  ((ASCIIMeterAddress[2*i]-0x30)<<4) + ((ASCIIMeterAddress[2*i+1]-0x30)&0x0F);
	}
	
	
		SetMeterAddr(TermAddr,(ASCIIChannle[0]-0x30),(ASCIIVer[0]-0x30),Mertersn_ID); //
	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;
}


/****************************************************************************************************
**	函 数  名 称: Set_MeterParam_3
**	函 数  功 能: 热表参数设置
**	输 入  参 数: none
**	输 出  参 数: 无					                          
**  返   回   值: 无
**	备		  注: 慕肖清添加 演示用
*****************************************************************************************************/
void Set_MeterParam_3(void)// 热表参数设置
{
	uint8 status;
	uint8 line;
	uint8 column;
	uint8 column1;
	uint8 line1;
	
	uint8 column2;
	uint8 line2;
	
	uint8 column3;
	uint8 line3;
	int8 i;

	uint8 ASCIIMeterAddress[14];	
	uint8 ASCIIChannle[1];	
	uint8 ASCIIVer[1];	

	uint8 TermAddr[7] = {0};
//	uint8 TermAddrReverse[6] = {0x00};
	
//	MeterFilePara	para;
//	uint8 Err	= 0;
//	uint8 datalen;
#if 0	
	SelectInfo TabMenu[] = {{2,20,12},{3,22,11}};//行，列，长度
#else
   // SelectInfo TabMenu[] = {{2,25,13},{3,25,13},{4,25,13},{5,22,11}};//行，列，长度
	SelectInfo TabMenu[] = {{2,10,43},{3,10,13},{4,10,13},{5,22,11}};//行，列，长度
	
#endif
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
	CPU_SR			cpu_sr;
				
	uint8 addrtemp_DeLu[7] = {0x09, 0x78, 0x13, 0x21, 0x00, 0x11, 0x11};
	uint8 addrtemp_LanJE[7] = {0x35, 0x25, 0x10, 0x67, 0x00, 0x00, 0x00};
	uint8 addrtemp_TG[7] = {0x22, 0x17, 0x05, 0x14, 0x00, 0x11, 0x11};

	uint8 Mertersn_ID =2;
#if 1
	  if(Demo_param_set==0)
		{
		 // Err = SDReadData("/METER_FILE_PARA", &para, sizeof(MeterFilePara), 0);
		  
		//  if(para.DEMOWriteFlag== 0xAA55)
		
		 if(DemoDateIsGet)
			  {
		  
				 
			  
			  }
		  else
     			  {
     		
     		memset((uint8 *)gPARA_MeterFiles, 0x00, sizeof(gPARA_MeterFiles));
     		memset((uint8 *)gPARA_MeterChannel, 0x00, sizeof(gPARA_MeterChannel));
     		memset((uint8 *)gPARA_Meter_Failed, 0x00, sizeof(gPARA_Meter_Failed));
     		memset((uint8 *)gPARA_MeterChannelNum, 0x00, sizeof(gPARA_MeterChannelNum));
     		memset((uint8 *)gPARA_ConPanelChannel, 0x00, sizeof(gPARA_ConPanelChannel));
     		memset((uint8 *)gPARA_ConPanelChannelNum, 0x00, sizeof(gPARA_ConPanelChannelNum));
     		memset((uint8 *)gPARA_ValveConChannel, 0x00, sizeof(gPARA_ValveConChannel));
     		memset((uint8 *)gPARA_ValveConChannelNum, 0x00, sizeof(gPARA_ValveConChannelNum));
     	 
     		gPARA_MeterFiles[0].ChannelIndex = 3;
     		gPARA_MeterFiles[0].ProtocolVer = 0;  
     		memcpy(gPARA_MeterFiles[0].MeterAddr, addrtemp_DeLu, 7);  
     	 
     	 
     		gPARA_MeterFiles[1].ChannelIndex = 3;
     		gPARA_MeterFiles[1].ProtocolVer = 8;  
     		memcpy(gPARA_MeterFiles[1].MeterAddr, addrtemp_LanJE, 7);	 
     	 
     		gPARA_MeterFiles[2].ChannelIndex = 3;
     		gPARA_MeterFiles[2].ProtocolVer = 0;  
     		memcpy(gPARA_MeterFiles[2].MeterAddr, addrtemp_TG, 7);	 
     				
     					   
             #if 0//每个通道一个表的时候
     							 for(i=0;i<3;i++)
     								{
     								  gPARA_MeterChannelNum[i]=1;
     								  gPARA_MeterChannel[i][0] = i;
     								  gPARA_ConPanelChannel[i][0] = i;
     								  gPARA_ConPanelChannelNum[i]=1;
     								  gPARA_ValveConChannelNum[i]=1;
     								  gPARA_ValveConChannel[i][0] = i;
     								}
             #else 
     						gPARA_MeterChannelNum[2]=3;//通道1	  表的数量
     						for(i=0;i<3;i++)
     						{
     						  gPARA_MeterChannel[2][i] = i;
     						}
             #endif
     			
     					 gPARA_MeterNum = 3;
     		}	
	  	}
#endif
	
	
	
	//由于存储低字节在前，所以现在将字节顺序调换
	for(i=0; i<7; i++)
	{
		TermAddr[i] = gPARA_MeterFiles[Mertersn_ID].MeterAddr[6-i];
	}
#if 0	
	//取调换顺序后的后2字节有效
	for(i=0;i<2;i++)
	{
		ASCIIData[2*i] = HexToAscii(TermAddr[4+i]>>4);
		ASCIIData[2*i+1] = HexToAscii(TermAddr[4+i]&0x0f);
	}
#else
	
//取调换顺序后的后2字节有效
for(i=0;i<7;i++)
{
	ASCIIMeterAddress[2*i] = HexToAscii(TermAddr[i]>>4);
	ASCIIMeterAddress[2*i+1] = HexToAscii(TermAddr[i]&0x0f);
}
 ASCIIChannle[0] = HexToAscii(gPARA_MeterFiles[Mertersn_ID].ChannelIndex);
 ASCIIVer[0]=HexToAscii(gPARA_MeterFiles[Mertersn_ID].ProtocolVer) ;
#endif
	i = 0;
	line = TabMenu[0].startline;
	column = TabMenu[0].startcolumn;
	line1 = TabMenu[1].startline;
	column1 = TabMenu[1].startcolumn;
	
	line2 = TabMenu[2].startline;
	column2 = TabMenu[2].startcolumn;
	
	line3 = TabMenu[3].startline;
	column3 = TabMenu[3].startcolumn;
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
	   #if 0
		FreshTermAddrSet(&ASCIIData[0]);
	   #else	   
	   FreshMeterAddrSet(&ASCIIMeterAddress[0],&ASCIIChannle[0],&ASCIIVer[0]);
	   #endif
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
						KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
						//KeyFuncIndex = 2;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else if(TabNum == 0)		
		{
			ReverseShowChar(line,column);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIMeterAddress[i] >= 0x39)
					{
						ASCIIMeterAddress[i] = 0x30;
					}
					else
					{
						ASCIIMeterAddress[i] = ASCIIMeterAddress[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIMeterAddress[i] <= 0x30)||(ASCIIMeterAddress[i] >0x39))
					{
						ASCIIMeterAddress[i] = 0x39;
					}
					else
					{
						ASCIIMeterAddress[i] = ASCIIMeterAddress[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line,column,ASCIIMeterAddress[i]);
					if((i > 0)&&(i<14))
					{
						i = i - 1;
						column = column - 3;
					}
					else
						{
                          i =0;
						  column = TabMenu[0].startcolumn;
					    }
					break;
				case KEY_RIGHT://向右键
					ShowChar(line,column,ASCIIMeterAddress[i]);
					if(i < 13)
					{
						i = i + 1;
						column = column + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		
		else if(TabNum == 1)		
		{
			ReverseShowChar(line1,column1);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIChannle[i] >= 0x39)
					{
						ASCIIChannle[i] = 0x30;
					}
					else
					{
						ASCIIChannle[i] = ASCIIChannle[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIChannle[i] <= 0x30)||(ASCIIChannle[i] >0x39))
					{
						ASCIIChannle[i] = 0x39;
					}
					else
					{
						ASCIIChannle[i] = ASCIIChannle[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line1,column1,ASCIIChannle[0]);
					break;
				case KEY_RIGHT://向右键
					ShowChar(line1,column1,ASCIIChannle[0]);
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;

					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}
		else if(TabNum ==2)
		{
			ReverseShowChar(line2,column2);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIVer[i] >= 0x39)
					{
						ASCIIVer[i] = 0x30;
					}
					else
					{
						ASCIIVer[i] = ASCIIVer[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if((ASCIIVer[i] <= 0x30)||(ASCIIVer[i] >0x39))
					{
						ASCIIVer[i] = 0x39;
					}
					else
					{
						ASCIIVer[i] = ASCIIVer[i] - 1;
					}	
					break;			
				case KEY_LEFT://向左键
					ShowChar(line2,column2,ASCIIVer[i]);
					if(i > 0)
					{
						i = i - 1;
						column2 = column2 - 3;
					}
					break;
				case KEY_RIGHT://向右键
					ShowChar(line2,column2,ASCIIVer[i]);
					if(i < 3)
					{
						i = i + 1;
						column2 = column2 + 3;
					}
					break;
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					column1 = TabMenu[1].startcolumn;
					
					line2 = TabMenu[2].startline;
					column2 = TabMenu[2].startcolumn;
					
					line3 = TabMenu[3].startline;
					column3 = TabMenu[3].startcolumn;
					line1 = line1 ;
					column1 = column1;
					line2 = line2 ;
					column2 = column2;
					line3 = line3 ;
					column3 = column3;
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}

	}//end while

	memset(TermAddr, 0x00, 7); //

for(i=0;i<7;i++)
{
	TermAddr[6-i] =  ((ASCIIMeterAddress[2*i]-0x30)<<4) + ((ASCIIMeterAddress[2*i+1]-0x30)&0x0F);
}


	SetMeterAddr(TermAddr,(ASCIIChannle[0]-0x30),(ASCIIVer[0]-0x30),Mertersn_ID); //
	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;
}



void CJ188_BCD2Hex(CJ188_Format *MeterFormatCJ188Data)
{
	MeterFormatCJ188Data->DailyHeat =BcdToHex_16bit1(MeterFormatCJ188Data->DailyHeat);
	MeterFormatCJ188Data->CurrentHeat=BcdToHex_16bit1(MeterFormatCJ188Data->CurrentHeat);
	MeterFormatCJ188Data->HeatPower=BcdToHex_16bit1(MeterFormatCJ188Data->HeatPower);
	MeterFormatCJ188Data->Flow=BcdToHex_16bit1(MeterFormatCJ188Data->Flow);
	MeterFormatCJ188Data->AccumulateFlow=BcdToHex_16bit1(MeterFormatCJ188Data->AccumulateFlow);
	MeterFormatCJ188Data->WaterInTemp[0]=BcdToHex(MeterFormatCJ188Data->WaterInTemp[0]);
	
	MeterFormatCJ188Data->WaterInTemp[1]=BcdToHex(MeterFormatCJ188Data->WaterInTemp[1]);
	
	MeterFormatCJ188Data->WaterInTemp[2]=BcdToHex(MeterFormatCJ188Data->WaterInTemp[2]);
	MeterFormatCJ188Data->WaterOutTemp[0]=BcdToHex(MeterFormatCJ188Data->WaterOutTemp[0]);
	MeterFormatCJ188Data->WaterOutTemp[1]=BcdToHex(MeterFormatCJ188Data->WaterOutTemp[1]);
	MeterFormatCJ188Data->WaterOutTemp[2]=BcdToHex(MeterFormatCJ188Data->WaterOutTemp[2]);

}
/****************************************************************************************************
**	函 数  名 称: ReadMeterStateShowData
**	函 数  功 能: 
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ReadMeterStateShowData(void)

{
	uint8 status;
//	uint8 i  ;
//	uint8 j = 0  ;
	uint8 line, column;
	uint8 len,len1,len2,len3;
//	uint8 DateTemp[6] = {0x00};
	uint8 ASCIIData[6] = {0x00};
	uint8 ASCIIZero[1] = {0x30};

//	TimingState	lTimingState;  //抄表状态信息结构体
	CPU_SR		cpu_sr;
	
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	uint32 tempData=0;
	line 	= 1;
	column 	= 0;
	uint8 index_keydown=0;
	//Demo_CJ188;
	CJ188_BCD2Hex(&Demo_CJ188_ch1);
	
	CJ188_BCD2Hex(&Demo_CJ188_ch2);
	
	CJ188_BCD2Hex(&Demo_CJ188_ch3);
	Show_Data_Status =1;
 while(1)
  {
  #if 0
	OS_ENTER_CRITICAL();
	lTimingState = gREAD_TimingState;
	OS_EXIT_CRITICAL();
	
	//抄表开始时间：
	for(i=0;i<3;i++)
	{
		ASCIIData[2*i] = HexToAscii(lTimingState.TimingStartTime[2-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(lTimingState.TimingStartTime[2-i]&0x0f);
	}
	
	ShowLineChar_HZ(line, column, &MeterState[0][0], 7); //显示汉字，“抄表开始时间”
	ShowIniTime(line + 1, column+10, &ASCIIData[0]);  //显示当前时间
   #endif 
 //if(Channel_1_SUCESS)
 	//{
	// ShowLineChar_HZ(line, column, &Meter_ST_OK_1[0], 7); //显示汉字，"德鲁抄表: 成功"
	// ShowIniTime(line + 1, column+10, &ASCIIData[0]);  //显示当前时间

  //  }
  
 if( index_keydown==0)
 	{
   ShowLineChar_HZ(line, column, &MenuHZ16_21_31[0],2);//德鲁
   
	ShowLineChar_HZ(line, column+12, &Meter_ST_Building[0],3);//楼号:
	len = sprintf((char*)&ASCIIData[0],"%d",10);
	ShowLineChar(line, column+25, &ASCIIData[0], len); 

	
	ShowLineChar_HZ(line, column+32, &Meter_ST_Unit[0],3);//单元:
	
	len = sprintf((char*)&ASCIIData[0],"%d",1);
	ShowLineChar(line, column+45, &ASCIIData[0], len); 
	
   ShowLineChar_HZ(line+1, column, &Meter_ST_AIM_CHeat[0],5);
   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch1.CurrentHeat);
   if(len>2)//去掉小数
   ShowLineChar(line+1, column+22, &ASCIIData[0], len-2); 
   else
   	
   ShowLineChar(line+1, column+22, &ASCIIZero[0], 1); 
   
   ShowLineChar_HZ(line+2, column, &Meter_ST_AIM_Power[0],4);
   
   tempData=BcdToHex_16bit1(Demo_CJ188_ch1.HeatPower);
   len = sprintf((char*)&ASCIIData[0],"%d",(uint16)Demo_CJ188_ch1.HeatPower);
   ShowLineChar(line+2, column+22, &ASCIIData[0], len); 

   ShowLineChar_HZ(line+3, column, &Meter_ST_AIM_Flow[0],3);
   
   tempData=BcdToHex_16bit1(Demo_CJ188_ch1.Flow);
   len = sprintf((char*)&ASCIIData[0],"%d",(uint16)Demo_CJ188_ch1.Flow);
   ShowLineChar(line+3, column+22, &ASCIIData[0], len); 

   ShowLineChar_HZ(line+4, column, &Meter_ST_AIM_CFlow[0],5);
   
 //  tempData=BcdToHex_16bit1(Demo_CJ188_ch1.AccumulateFlow);
 //  len = sprintf((char*)&ASCIIData[0],"%d",(uint16)Demo_CJ188_ch1.AccumulateFlow);
 
 len = sprintf((char*)&ASCIIData[0],"%d",0);
 
 len1 = sprintf((char*)&ASCIIData[1],"%s",".");
 
 len2 = sprintf((char*)&ASCIIData[2],"%d",Demo_CJ188_ch1.AccumulateFlow);
   ShowLineChar(line+4, column+22, &ASCIIData[0], len+len1+len2); 
//进水温度
   ShowLineChar_HZ(line+5, column, &Meter_ST_AIM_INwater[0],5);

   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch1.WaterInTemp[2]);
   len1 = sprintf((char*)&ASCIIData[len],"%d",Demo_CJ188_ch1.WaterInTemp[1]);
    len2 = sprintf((char*)&ASCIIData[len+len1],"%s",".");
	
	len3 = sprintf((char*)&ASCIIData[len+len1+len2],"%d",Demo_CJ188_ch1.WaterInTemp[0]);
	
   ShowLineChar(line+5, column+22, &ASCIIData[0], (len+len1+len2+len3)); 
   //进水温度

   //回水温度
   ShowLineChar_HZ(line+6, column, &Meter_ST_AIM_Owater[0],5);
   
      len=len1=len2=len3=0;
	  len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch1.WaterOutTemp[2]);
	  len1 = sprintf((char*)&ASCIIData[len],"%d",Demo_CJ188_ch1.WaterOutTemp[1]);
	   len2 = sprintf((char*)&ASCIIData[len+len1],"%s",".");
	   
	   len3 = sprintf((char*)&ASCIIData[len+len1+len2],"%d",Demo_CJ188_ch1.WaterOutTemp[0]);
	   
	  ShowLineChar(line+6, column+22, &ASCIIData[0], (len+len1+len2+len3)); 
   //回水温度

 	}
 else if ( index_keydown==1)
 	{
   ShowLineChar_HZ(line, column, &MenuHZ16_21_32[0],3);//兰吉尔

   ShowLineChar_HZ(line, column+17, &Meter_ST_Building[0],3);//楼号:
   len = sprintf((char*)&ASCIIData[0],"%d",11);
   ShowLineChar(line, column+30, &ASCIIData[0], len); 
   
   
   ShowLineChar_HZ(line, column+37, &Meter_ST_Unit[0],3);//单元:
   
   len = sprintf((char*)&ASCIIData[0],"%d",1);
   ShowLineChar(line, column+50, &ASCIIData[0], len); 


   
   ShowLineChar_HZ(line+1, column, &Meter_ST_AIM_CHeat[0],5);
   
   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch2.CurrentHeat);
   
   if(len>2)//去掉小数
   ShowLineChar(line+1, column+22, &ASCIIZero[0], 1); 
  else
  	
  ShowLineChar(line+1, column+22, &ASCIIZero[0], 1); 
   ShowLineChar_HZ(line+2, column, &Meter_ST_AIM_Power[0],4);
   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch2.HeatPower);
   ShowLineChar(line+2, column+22, &ASCIIData[0], len); 

   ShowLineChar_HZ(line+3, column, &Meter_ST_AIM_Flow[0],3);
   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch2.Flow);
   ShowLineChar(line+3, column+22, &ASCIIData[0], len); 

   ShowLineChar_HZ(line+4, column, &Meter_ST_AIM_CFlow[0],5);
  // len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch2.AccumulateFlow);
    len = sprintf((char*)&ASCIIData[0],"%d",0);
    
    len1 = sprintf((char*)&ASCIIData[1],"%s",".");

	len2 = sprintf((char*)&ASCIIData[2],"%d",Demo_CJ188_ch2.AccumulateFlow);
   ShowLineChar(line+4, column+22, &ASCIIData[0], len+len1+len2); 
//进水温度
   ShowLineChar_HZ(line+5, column, &Meter_ST_AIM_INwater[0],5);

   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch2.WaterInTemp[2]);
   len1 = sprintf((char*)&ASCIIData[len],"%d",Demo_CJ188_ch2.WaterInTemp[1]);
    len2 = sprintf((char*)&ASCIIData[len+len1],"%s",".");
	
	len3 = sprintf((char*)&ASCIIData[len+len1+len2],"%d",Demo_CJ188_ch2.WaterInTemp[0]);
	
   ShowLineChar(line+5, column+22, &ASCIIData[0], (len+len1+len2+len3)); 
   //进水温度

   //回水温度
   ShowLineChar_HZ(line+6, column, &Meter_ST_AIM_Owater[0],5);
   
      len=len1=len2=len3=0;
	  len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch2.WaterOutTemp[2]);
	  len1 = sprintf((char*)&ASCIIData[len],"%d",Demo_CJ188_ch2.WaterOutTemp[1]);
	   len2 = sprintf((char*)&ASCIIData[len+len1],"%s",".");
	   
	   len3 = sprintf((char*)&ASCIIData[len+len1+len2],"%d",Demo_CJ188_ch2.WaterOutTemp[0]);
	   
	  ShowLineChar(line+6, column+22, &ASCIIData[0], (len+len1+len2+len3)); 
   //回水温度

 	}
 else if(index_keydown ==2)
 	{
   ShowLineChar_HZ(line, column, &MenuHZ16_21_33[0],2);


   ShowLineChar_HZ(line, column+12, &Meter_ST_Building[0],3);//楼号:
   len = sprintf((char*)&ASCIIData[0],"%d",12);
   ShowLineChar(line, column+25, &ASCIIData[0], len); 
   
   
   ShowLineChar_HZ(line, column+32, &Meter_ST_Unit[0],3);//单元:
   
   len = sprintf((char*)&ASCIIData[0],"%d",1);
   ShowLineChar(line, column+45, &ASCIIData[0], len); 

   
   ShowLineChar_HZ(line+1, column, &Meter_ST_AIM_CHeat[0],5);
   
   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch3.CurrentHeat);
	if(len>2)//去掉小数
	ShowLineChar(line+1, column+22, &ASCIIData[0], len-2); 
   else
	 
   ShowLineChar(line+1, column+22, &ASCIIZero[0], 1); 
   
   ShowLineChar_HZ(line+2, column, &Meter_ST_AIM_Power[0],4);
   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch3.HeatPower);
   ShowLineChar(line+2, column+22, &ASCIIData[0], len); 

   ShowLineChar_HZ(line+3, column, &Meter_ST_AIM_Flow[0],3);
   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch3.Flow);
   ShowLineChar(line+3, column+22, &ASCIIData[0], len); 

   ShowLineChar_HZ(line+4, column, &Meter_ST_AIM_CFlow[0],5);
//   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch3.AccumulateFlow);
   len = sprintf((char*)&ASCIIData[0],"%d",0);
   
   len1 = sprintf((char*)&ASCIIData[1],"%s",".");
   
   len2 = sprintf((char*)&ASCIIData[2],"%d",Demo_CJ188_ch3.AccumulateFlow);

   ShowLineChar(line+4, column+22, &ASCIIData[0], len+len1+len2); 
//进水温度
   ShowLineChar_HZ(line+5, column, &Meter_ST_AIM_INwater[0],5);

   len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch3.WaterInTemp[2]);
   len1 = sprintf((char*)&ASCIIData[len],"%d",Demo_CJ188_ch3.WaterInTemp[1]);
    len2 = sprintf((char*)&ASCIIData[len+len1],"%s",".");
	
	len3 = sprintf((char*)&ASCIIData[len+len1+len2],"%d",Demo_CJ188_ch3.WaterInTemp[0]);
	
   ShowLineChar(line+5, column+22, &ASCIIData[0], (len+len1+len2+len3)); 
   //进水温度

   //回水温度
   ShowLineChar_HZ(line+6, column, &Meter_ST_AIM_Owater[0],5);
   
      len=len1=len2=len3=0;
	  len = sprintf((char*)&ASCIIData[0],"%d",Demo_CJ188_ch3.WaterOutTemp[2]);
	  len1 = sprintf((char*)&ASCIIData[len],"%d",Demo_CJ188_ch3.WaterOutTemp[1]);
	   len2 = sprintf((char*)&ASCIIData[len+len1],"%s",".");
	   
	   len3 = sprintf((char*)&ASCIIData[len+len1+len2],"%d",Demo_CJ188_ch3.WaterOutTemp[0]);
	   
	  ShowLineChar(line+6, column+22, &ASCIIData[0], (len+len1+len2+len3)); 
   //回水温度

 	}
#if 0
    //抄表结束时间：
	for(i=0;i<3;i++)
	{
		ASCIIData[2*i] = HexToAscii(lTimingState.TimingEndTime[2-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(lTimingState.TimingEndTime[2-i]&0x0f);
	}
	
	ShowLineChar_HZ(line+2, column, &MeterState[3][0], 7);
	ShowIniTime(line + 3, column+10, &ASCIIData[0]);
	
 
 
	//抄表总数:
	len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
    ShowLineChar_HZ(line+5, column, &MeterState[1][0], 7); //显示 “抄表总数：”
	ShowLineChar(line+5, column+30, &ASCIIData[0], len); //显示“抄表的总个数”

	
	//抄表成功数:
   	len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.ReadCmplNums);
	ShowLineChar_HZ(line+6, column, &MeterState[2][0], 7);  //显示 “抄表成功数：”
	ShowLineChar(line+6, column+35, &ASCIIData[0], len);  //显示“抄表的成功个数”
	
	status = GetKeyStatuschaobiao(); //获取键值
#endif	
	status = GetKeyStatuschaobiao(); //获取键值
    
   	if( status == KEY_DOWN )
	{	
	index_keydown++;
	if(index_keydown>2)
		index_keydown=0;
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	}
	else if( status == KEY_UP )
	{	
	index_keydown--;
	if(index_keydown<0||index_keydown>2)
		index_keydown=2;
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	}
   	else if( status == KEY_EXIT )
	{	
	       
		   Show_Data_Status =0;
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
			//KeyFuncIndex = 2;
		(*KeyTab[KeyFuncIndex].CurrentOperate)();
		break;
	}	
  }
	 
}

/****************************************************************************************************
**	函 数  名 称: ReadMeterStateShow_Demo
**	函 数  功 能: 读取热量表状态
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ReadMeterStateShow_Demo(void)
{
	uint8 status;
//	uint8 i  ;
//	uint8 j = 0  ;
	uint8 line, column;
	uint8 len;
//	uint8 DateTemp[6] = {0x00};
	uint8 ASCIIData[6] = {0x00};
	TimingState	lTimingState;  //抄表状态信息结构体
	CPU_SR		cpu_sr;
	
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	
	line 	= 1;
	column 	= 0;
	Meter_R_End =0;
 while(1)
  {
  #if 0
	OS_ENTER_CRITICAL();
	lTimingState = gREAD_TimingState;
	OS_EXIT_CRITICAL();
	
	//抄表开始时间：
	for(i=0;i<3;i++)
	{
		ASCIIData[2*i] = HexToAscii(lTimingState.TimingStartTime[2-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(lTimingState.TimingStartTime[2-i]&0x0f);
	}
	
	ShowLineChar_HZ(line, column, &MeterState[0][0], 7); //显示汉字，“抄表开始时间”
	ShowIniTime(line + 1, column+10, &ASCIIData[0]);  //显示当前时间
   #endif 
 //if(Channel_1_SUCESS)
 	//{
	// ShowLineChar_HZ(line, column, &Meter_ST_OK_1[0], 7); //显示汉字，"德鲁抄表: 成功"
	// ShowIniTime(line + 1, column+10, &ASCIIData[0]);  //显示当前时间

  //  }
  
   if(Meter_R_End ==0)
   	{
   	OS_ENTER_CRITICAL();
	lTimingState = gREAD_TimingState;
	OS_EXIT_CRITICAL();
      ShowLineChar_HZ(line, column, &Meter_ST_WAIT[0], 6);
	  
	  len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
	  ShowLineChar_HZ(line+5, column, &MeterState[1][0], 7); //显示 “抄表总数：”
	  ShowLineChar(line+5, column+30, &ASCIIData[0], len); //显示“抄表的总个数”
	  
	  
	  //抄表成功数:
	  len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.ReadCmplNums);
	  ShowLineChar_HZ(line+6, column, &MeterState[2][0], 7);  //显示 “抄表成功数：”
	  ShowLineChar(line+6, column+35, &ASCIIData[0], len);	//显示“抄表的成功个数”
    }
  else if(Meter_R_End ==1)
  	{
  	
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	(Channel_1_SUCESS ==1)?(ShowLineChar_HZ(line, column, &Meter_ST_OK_1[0], 7)):(ShowLineChar_HZ(line, column, &Meter_ST_FAL_1[0], 7));
	 (Channel_2_SUCESS ==1)?(ShowLineChar_HZ(line+1, column, &Meter_ST_OK_2[0], 8)):(ShowLineChar_HZ(line+1, column, &Meter_ST_FAL_2[0], 8));
	 (Channel_3_SUCESS ==1)?(ShowLineChar_HZ(line+2, column, &Meter_ST_OK_3[0], 7)):(ShowLineChar_HZ(line+2, column, &Meter_ST_FAL_3[0], 7));
     
	 ShowLineChar_HZ(line+5, column, &Meter_ST_AIM[0],11);
    }
#if 0
    //抄表结束时间：
	for(i=0;i<3;i++)
	{
		ASCIIData[2*i] = HexToAscii(lTimingState.TimingEndTime[2-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(lTimingState.TimingEndTime[2-i]&0x0f);
	}
	
	ShowLineChar_HZ(line+2, column, &MeterState[3][0], 7);
	ShowIniTime(line + 3, column+10, &ASCIIData[0]);
	
 
 
	//抄表总数:
	len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
    ShowLineChar_HZ(line+5, column, &MeterState[1][0], 7); //显示 “抄表总数：”
	ShowLineChar(line+5, column+30, &ASCIIData[0], len); //显示“抄表的总个数”

	
	//抄表成功数:
   	len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.ReadCmplNums);
	ShowLineChar_HZ(line+6, column, &MeterState[2][0], 7);  //显示 “抄表成功数：”
	ShowLineChar(line+6, column+35, &ASCIIData[0], len);  //显示“抄表的成功个数”
	
	status = GetKeyStatuschaobiao(); //获取键值
#endif	
	status = GetKeyStatuschaobiao(); //获取键值
    
   	if( status == KEY_DOWN )
	{	
	    if(Meter_R_End ==1)//数据采集完成前按键不起作用
	    	{
        		ReadMeterStateShowData();
        		break;
	    	}
	}	
   	else if( status == KEY_EXIT )
	{	
	    if(Meter_R_End ==1)//数据采集完成前按键不起作用
	    	{
        			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
        			//KeyFuncIndex = 2;
        		(*KeyTab[KeyFuncIndex].CurrentOperate)();
        		break;
	    	}
	}	
  }
	 
}

/****************************************************************************************************
**	函 数  名 称: ReadMeter_Demo
**	函 数  功 能: 立即抄表
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ReadMeter_Demo(void)
{
		uint8 SendMsg[2]		= {0x00};
		SendMsg[0] =  MSG_1D;
		uint8 i=0;
#ifdef DEMO_APP
//			MeterFilePara	para;
//			uint8 Err	= 0;
			
			uint8 addrtemp_DeLu[7] = {0x09, 0x78, 0x13, 0x21, 0x00, 0x11, 0x11};
			uint8 addrtemp_LanJE[7] = {0x35, 0x25, 0x10, 0x67, 0x00, 0x00, 0x00};
			uint8 addrtemp_TG[7] = {0x22, 0x17, 0x05, 0x14, 0x00, 0x11, 0x11};
#endif			
		
		memset((uint8 *)&Demo_CJ188_ch1, 0, sizeof(Demo_CJ188_ch1));
		
		memset((uint8 *)&Demo_CJ188_ch2, 0, sizeof(Demo_CJ188_ch2));
		memset((uint8 *)&Demo_CJ188_ch3, 0, sizeof(Demo_CJ188_ch3));
#if 1
		  if(Demo_param_set==0)
			{
			
			//Err = SDReadData("/METER_FILE_PARA", &para, sizeof(MeterFilePara), 0);
			
		//	if(para.DEMOWriteFlag== 0xAA55)
		    if(DemoDateIsGet)
				{

                   
				
				}
			else
				{
           			memset((uint8 *)gPARA_MeterFiles, 0x00, sizeof(gPARA_MeterFiles));
           			memset((uint8 *)gPARA_MeterChannel, 0x00, sizeof(gPARA_MeterChannel));
           			memset((uint8 *)gPARA_Meter_Failed, 0x00, sizeof(gPARA_Meter_Failed));
           			memset((uint8 *)gPARA_MeterChannelNum, 0x00, sizeof(gPARA_MeterChannelNum));
           			memset((uint8 *)gPARA_ConPanelChannel, 0x00, sizeof(gPARA_ConPanelChannel));
           			memset((uint8 *)gPARA_ConPanelChannelNum, 0x00, sizeof(gPARA_ConPanelChannelNum));
           			memset((uint8 *)gPARA_ValveConChannel, 0x00, sizeof(gPARA_ValveConChannel));
           			memset((uint8 *)gPARA_ValveConChannelNum, 0x00, sizeof(gPARA_ValveConChannelNum));
           		 
           			gPARA_MeterFiles[0].ChannelIndex = 3;
           			gPARA_MeterFiles[0].ProtocolVer = 0;  
           			memcpy(gPARA_MeterFiles[0].MeterAddr, addrtemp_DeLu, 7);  
           		 
           		 
           			gPARA_MeterFiles[1].ChannelIndex = 3;
           			gPARA_MeterFiles[1].ProtocolVer = 8;  
           			memcpy(gPARA_MeterFiles[1].MeterAddr, addrtemp_LanJE, 7);	 
           		 
           			gPARA_MeterFiles[2].ChannelIndex = 3;
           			gPARA_MeterFiles[2].ProtocolVer = 0;  
           			memcpy(gPARA_MeterFiles[2].MeterAddr, addrtemp_TG, 7);	 
           					
           						   
                   #if 0//每个通道一个表的时候
           								 for(i=0;i<3;i++)
           									{
           									  gPARA_MeterChannelNum[i]=1;
           									  gPARA_MeterChannel[i][0] = i;
           									  gPARA_ConPanelChannel[i][0] = i;
           									  gPARA_ConPanelChannelNum[i]=1;
           									  gPARA_ValveConChannelNum[i]=1;
           									  gPARA_ValveConChannel[i][0] = i;
           									}
                   #else 
           							gPARA_MeterChannelNum[2]=3;//通道1	  表的数量
           							for(i=0;i<3;i++)
           							{
           							  gPARA_MeterChannel[2][i] = i;
           							}
                   #endif
				   
				   gPARA_MeterNum = 3;
				}
				
			}	
#endif
     
		OSQPost(CMD_Q, (void*)SendMsg);
		ReadMeterStateShow_Demo();
}
#endif
/****************************************************************************************************
**	函 数  名 称: SlectReadMeterChannel
**	函 数  功 能: 选择抄表通道
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void SlectReadMeterChannel(void)
{	
	uint8 status;
	uint8 line;
	uint8 column;
	uint8 column1;
	uint8 line1;
	int8 i;
	uint8 ASCIIData[4] = {0x31,0,0,0};
	
//	uint8 datalen;
	
	SelectInfo TabMenu[] = {{2,20,12},{3,22,11}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
	
	i = 0;
	line = TabMenu[0].startline;
	column = TabMenu[0].startcolumn;
	line1 = TabMenu[1].startline;
	column1 = TabMenu[1].startcolumn;
	
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
		FreshSlectReadMeterChannel(&ASCIIData[0]);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 10;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else if(TabNum == 0)
		{
			ReverseShowChar(line,column);
			status = GetKeyStatus();//有待修改为得到键盘的状态函数
			
			switch(status)
			{
				case KEY_UP://向上键
					if(ASCIIData[i] == 0x36)
					{
						ASCIIData[i] = 0x31;
					}
					else
					{
						ASCIIData[i] = ASCIIData[i] + 1;
					}
					break;
				case KEY_DOWN://向下键
					if(ASCIIData[i] == 0x31)
					{
						ASCIIData[i] = 0x36;
					}
					else
					{
						ASCIIData[i] = ASCIIData[i] - 1;
					}	
					break;			
				
				case KEY_ENTER://回车键
					Tab_Flag = (!Tab_Flag);
					//返回原始列，下次进入时可以从头开始
					i = 0;
					line = TabMenu[0].startline;
					column = TabMenu[0].startcolumn;
					line1 = TabMenu[1].startline;
					line1 = line1;/*防止编译警告*/
					column1 = TabMenu[1].startcolumn;
					column1 = column1;/*防止编译警告*/
					TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 4;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}
		}//end else if(TabNum == 0)

	}//end while
	
	{
          uint8 channel=0;
	   channel = ASCIIData[0]-0x30;
	    if(channel>=1&&channel<=6)
          {
           //切换Mbus通道
	     METER_ChangeChannel(channel);		
            Uart_Printf("METER_ChangeChannel  to %d\r\n",channel);
          }
        else
          {
            Uart_Printf("Error:METER_ChangeChannel = %d \r\n",channel);
          }
	}
	//==========================================
	ShowMenuBottom(2);//设置成功

	KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();

	return;
}


/****************************************************************************************************
**	函 数  名 称: restart
**	函 数  功 能: 重新启动
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void restart(void)
{
	;
}

/****************************************************************************************************
**	函 数  名 称: ResetSystem
**	函 数  功 能: 重启终端，确认或者否认
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void ResetSystem(void)  //重启终端，确认或者否认
{
	uint8 status;
	
	display_white();
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowPicture(16,0,48,160,&MenuPic1_3_1[0]);
	
	status = GetKeyStatus();
	switch(status)
	{
		case KEY_ENTER://回车键
			//====================
			//加入重启终端的函数
			    //RestartHardWare();//复位函数
			     //gRestart = 1;
				 /*begin:yangfei added 2013-01-29*/
				 	NVIC_SETFAULTMASK();
					NVIC_GenerateSystemReset();
				 /*end:yangfei added 2013-01-29*/
				ShowMenuBottom(4);
				OSTimeDly(OS_TICKS_PER_SEC*5);
			//====================
			KeyFuncIndex = 11;
			(*KeyTab[KeyFuncIndex].CurrentOperate)();
			break;
		case KEY_EXIT://返回键
			KeyFuncIndex = 11;
			(*KeyTab[KeyFuncIndex].CurrentOperate)();
			break;	
		default:
			KeyFuncIndex = 24;
			(*KeyTab[KeyFuncIndex].CurrentOperate)();		
			break;
	}
	return;
}

/****************************************************************************************************
**	函 数  名 称: FindKey
**	函 数  功 能: 查找关键字key,返回值为关键字位置
**	输 入  参 数: uint8* StrBuff
**                uint8 Key
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
int FindKey(uint8* StrBuff, uint8 Key)//查找关键字key,返回值为关键字位置
{
	int i = 0;
	uint8 flag = 0;
	while(*StrBuff != 0x00)
	{
		if(*StrBuff != Key)
		{
			StrBuff++;
			i++;		
		}
		else
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		i = 0xffff;
	}
	return i;
}

/****************************************************************************************************
**	函 数  名 称: VerInfoShow
**	函 数  功 能: 终端版本信息显示
**	输 入  参 数: uint8* StrBuff
**                uint8 Key
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void VerInfoShow(void)  //终端版本信息显示===
{
    uint16 version = gVersion;
	uint16 version1[4];
	int i= 0;	
	uint8 lu8VersionH = 0;
	uint8 lu8VersionL = 0;
	

	
	display_white(); //显示白屏
	ShowMenuTop();	//显示液晶顶端信息
	ShowMenuBottom(1);//显示液晶底部信息
	//显示汉字
	
    version = (((((SW_VERSION)/100)<<8)&0x0F00)+((((SW_VERSION%100)/10)<<4)&0x00F0)+((SW_VERSION%10)&0x000F));
	ShowLineChar_HZ(1,0,&Menu_VerInfo[0][0],5);	//显示一行ASCII码字符和汉字的混合
	ShowLineChar_HZ(3,0,&Menu_VerInfo[1][0],5);	//显示一行ASCII码字符和汉字的混合

	//显示版本信息
	ShowLineChar1(4,0, __DATE__,11);
	ShowLineChar1(5,0, __TIME__,8);
	ShowLineChar1(2,0, &date[0],1);	//版本信息里面的"V"
	ShowLineChar_HZ(2,9,&Dot[0],1);//版本信息里面的"."

	lu8VersionH = version >> 8;
	lu8VersionH = lu8VersionH % 100;  //只显示2位.
	lu8VersionH = HexToBcd(lu8VersionH);
	lu8VersionL = version & 0x00ff;
	lu8VersionL = lu8VersionL % 100;  //只显示2位.
	lu8VersionL = HexToBcd(lu8VersionL);
	

	version1[0] = HexToAscii( lu8VersionH >> 4 );
	version1[1] = HexToAscii( lu8VersionH & 0x0f );
	version1[2] = HexToAscii( lu8VersionL >> 4 );
	version1[3] = HexToAscii( lu8VersionL & 0x0f ); 
	for( i=0; i<4; i++ )
	{
        if( i<2 )
		  {
			ShowChar1(2,3+i*3,version1[i]);
		  }
		else
		  {
			ShowChar1(2,6+i*3,version1[i]);
		  } 
	}
	return;
}

/****************************************************************************************************
**	函 数  名 称: LCDContrastSet
**	函 数  功 能: 液晶对比度调节
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void LCDContrastSet(void) //液晶对比度调节
{
	uint8 status;
	uint8 line = 4;
	uint8 column = 1;
	uint8 ContrastValue = LCD_CON_VALUE;//195
	uint8 BlackNum;
	
	bool flag = TRUE;
	uint8 len;
	uint8 ContrastBuff[5] = {0};
	uint8 Err 	= 0;
	LCDContrastPara NVLCDConrast;
    uint8 counter=0;
    ContrastValue =(uint8)NVLCDConraston.LCDContrast;


//	SelfDefinePara pSelfPara;
	
//	ContrastValue = gStore_CommPara.LCDContrastValue;
	/*
	Read_SelfDefinePara((uint8*)&pSelfPara);
	ContrastValue = pSelfPara.LCDContrastValue;
	if(ContrastValue == 0 || ContrastValue == 0xff)
	{
		ContrastValue = LCD_CON_VALUE;
	}
	*/
	while(flag)
	{
		display_white();
		ShowMenuTop();
		ShowMenuBottom(25);
			
		ShowLineChar_HZ(1,10,&MenuHZ13_26[2],7);//显示 液晶对比度调节
		ShowHZ(5,0,0x015a);//显示 暗
		ShowHZ(5,48,0x0159);//显示 亮
		
		//显示矩形图形
		ShowRectangle((16*line-1), column, 17, 153, TRUE);
		BlackNum = (153 - ((ContrastValue/5)*3)) / 3;
		BlackScreen(16*line, column, 16, BlackNum);
		
		len = sprintf((char*)&ContrastBuff[0], "%d", ContrastValue);
		ShowLineChar(line+1, 27, &ContrastBuff[0], len);
		SetContrast(ContrastValue);//设置对比值
		//存储到FLASH中
//		Read_SelfDefinePara((uint8*)&pSelfPara);
//		pSelfPara.LCDContrastValue = ContrastValue;
//		Set_SelfDefinePara((uint8*)&pSelfPara);
//		SelfDefinePara.LCDContrastValue = ContrastValue;//更新ContrastValue全局变量,在lcmdrv.c里液晶初始化的时候用到全局变量
		
		status = GetKeyStatus();//有待修改为得到键盘的状态函数
		switch(status)
		{
			case KEY_UP://向上键
				break;
			case KEY_DOWN://向下键
				break;
			case KEY_LEFT://向左键
				if(ContrastValue <= 250)
				{
					ContrastValue = ContrastValue + 5;
				}
				break;
			case KEY_RIGHT://向右键
				if(ContrastValue >= 5 )
				{
					ContrastValue = ContrastValue - 5;
				}
				break;			
			case KEY_ENTER://回车键
				KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				memset(&NVLCDConrast,0,sizeof(NVLCDConrast));
				NVLCDConrast.WriteFlag = 0xAA55;
				NVLCDConrast.LCDContrast = ContrastValue;
				Err=MakeFile("/LCD_CONTRAST_PARA");
				   if(Err)
					   {
					   
					   }
				   else
					   {
				   
						   do{
							   Err = SDReadData("/LCD_CONTRAST_PARA", &NVLCDConraston, sizeof(LCDContrastPara), 0);
							   
							   if(Err!=NO_ERR) 
								 {
								  counter++;
								  
								 }
							   if (counter ==5 )
								   {
									 counter =0;
									 break;
								   }
						   }while(Err!=NO_ERR);
						//   OSMutexPend (FlashMutex, 0, &Err);
						   Err = SDSaveData("/LCD_CONTRAST_PARA", &NVLCDConrast, sizeof(LCDContrastPara), 0);
						   
						 //  OSMutexPost (FlashMutex);
						   
					   }
				
				return;
			case KEY_EXIT://返回键
				KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				return;
		
			default:
				break;
		}
	}
	return;
}

///////////=====以下两个函数应该放在publicfunc.c中，但为了更新的方便，暂时放在这里========
////本函数为16进制的IP转换为15位的ASCII码，而ConverHexToASCIP返回长度不定
/****************************************************************************************************
**	函 数  名 称: ConverHexToASCIP_15
**	函 数  功 能: 16进制的IP转换为15位的ASCII码
**	输 入  参 数: uint8* pTemp1
**  			  uint8* pTemp2
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 ConverHexToASCIP_15(uint8* pTemp1,uint8* pTemp2)
{
	uint8 a,i;
	uint8 TempBuf[3];

	LOG_assert_param(  pTemp1 == NULL );
	LOG_assert_param(  pTemp2 == NULL );

	for(i=0;i<=3;i++)
	{
	    a = pTemp1[i];
	    memset(&TempBuf[0],0x30,3);
	    Hex_BcdToAscii((uint16)a,TempBuf);
	    Reversalmemcpy(pTemp2,TempBuf,3);
	    pTemp2 += 3;
	    if(i==3)  *pTemp2++ = 0x00;
	    else  *pTemp2++ = '.';//0x2E
	}
	return 15;
}
/****************************************************************************************************
**	函 数  名 称: Hex_BcdToAscii_5
**	函 数  功 能: 16位的HEX或者BCD码转化为ASCII码，然后反方向存放在AscBuf中，长度为5
**	输 入  参 数: uint16 Hex_bcd_code
**  			  uint8* AscBuf
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
/////本函数为16进制的port转换为5位的ASCII码，而Hex_BcdToAscii返回不定长
uint8 Hex_BcdToAscii_5(uint16 Hex_bcd_code,uint8* AscBuf)//16位的HEX或者BCD码转化为ASCII码，然后反方向存放在AscBuf中，长度为5
{
	uint8  num = 0;
	//uint8  TempBuf;
	uint16 b1,b2;

	LOG_assert_param(  AscBuf == NULL );

	b2 = Hex_bcd_code;
	memset(&AscBuf[0],0x30,5);
	//if(b2==0)//Hex_bcd_code为0的情况
	//{
	//	memset(&AscBuf[0],0x30,5);
	//}
	while(b2)
	{
		b1 = b2%10;
		AscBuf[num++] = HexToAscii(b1);
		b2 = b2/10;
	}
	return 5;
}
////////////以上两个函数为此处显示原来FLASH中的IP和PORT而设计===================================

/****************************************************************************************************
**	函 数  名 称: KeyCheck
**	函 数  功 能: 按键检测
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void KeyCheck(void)
{
	//void  *KeyFuncPtr;//功能函数指针
//	uint8 KeyFuncIndex;
	uint8 status;
	status = GetKeyStatus();
	switch(status)//判断函数有待修改为适当的条件，有待商量
	{
		case KEY_DOWN://向下键
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyDnState;
			break;
		case KEY_UP://向上键
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyUpState;
			break;
		case KEY_LEFT://向左键
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyLeftState;
			break;
		case KEY_RIGHT://向右键
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyRightState;
			break;			
		case KEY_ENTER: //回车键
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyCrState;
			break;
		case KEY_EXIT://返回键
			KeyFuncIndex = KeyTab[KeyFuncIndex].KeyBackState;
			break;
		default:
			break;
	} 
	//(*KeyTab[KeyFuncIndex].CurrentOperate)();
	//KeyFuncPtr =
	//(*KeyFuncPtr)();
}


/****************************************************************************************************
**	函 数  名 称: MenuTest
**	函 数  功 能: 目录测试
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void MenuTest(void)
{
//	uint8 i;
//	KeyFuncIndex = 1;
//	for(i = 0;i < 50;i++)
//	{	
		(*KeyTab[KeyFuncIndex].CurrentOperate)();

		KeyCheck();
		
//	}
	
}
/****************************************************************************************************
**	函 数  名 称: FreshMethodshow
**	函 数  功 能: 刷新选择方法状态
**	输 入  参 数: 
**	输 出  参 数: 无
**	返	 回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshMethodshow(uint8 line)
{
	display_white();
	ShowLineChar_HZ(1,4,&MenuHZ15[0],6);   //终端管理与维护
	ShowLineChar_HZ(2,1,&MenuHZ15_1[0],4);//热计量法
	ShowLineChar_HZ(3,1,&MenuHZ14[0],7);//时间通断面积法
	ShowLineChar_HZ(4,1,&MenuHZ15_3[0],5);//热分配表法
	
	ShowLineChar_HZ(7,1,&MenuHZ15_4[0],6);//当前计量方法
	ShowChar(7, 30, ':');
	if(gPARA_TermPara.DeviceType == HEAT_METER_TYPE)
	{
		ShowLineChar_HZ(8,1,&MenuHZ15_1[0],4);//热计量法
	}
	else if(gPARA_TermPara.DeviceType == TIME_ON_OFF_AREA_TYPE)
	{
		ShowLineChar_HZ(8,1,&MenuHZ14[0],7);//时间通断面积法
	}
	else if(gPARA_TermPara.DeviceType == HEATCOST_METER_TYPE)
	{
		ShowLineChar_HZ(8,1,&MenuHZ15_3[0],5);//热分配表法
	}
	ShowMenuTop();
	ShowMenuBottom(25);
	ReverseShowLine(line);
}

/****************************************************************************************************
**	函 数  名 称: FreshSetReadMeterTime
**	函 数  功 能: 刷新抄表时间设置页面
**	输 入  参 数: uint8 timenum
**                uint8 *pIndex
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshSetUserData(uint8 *pIndex,uint8 *puser,uint8 *pData)//刷新抄表时间设置页面
{
	LOG_assert_param( pIndex == NULL );
	LOG_assert_param( puser == NULL );

	CleanScreen(16,0,128,54);
	ShowMenuTop();
	ShowMenuBottom(1);
	ShowLineChar_HZ(1,10,&chose_user_display[0],6);
	ShowLineChar_HZ(2,9,&MenuHZ_user[0],6);
	ShowLineChar_HZ(3,9,&MenuHZ_TimeNum[0],9);
	ShowIniDate(4,9,&pData[0]);//显示日期
	ShowPicture(80,22,16,32,&MenuPic_OK[0]);//OK 4行
		

	ShowChar(2, 14, *puser++);
	ShowChar(2, 17, *puser++);
	ShowChar(2, 20, *puser);
	
	ShowChar(3, 14, *pIndex++);
	ShowChar(3, 17, *pIndex);

}
/****************************************************************************************************
**	函 数  名 称: get_sd_display_data
**	函 数  功 能: 从sd卡选择要显示的数据
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void get_sd_display_data(void)
{
	uint8 Res;
	uint8 buf[200];
	uint8 asiibuf[10];
	uint16  TimeNode;
	uint8 *pbuf;
	uint16 max_usr_num;
	SDSave_Data *pSaveData;
	char  Datapath[]="/2014/02/25/1500";
	char  NodePath[]="/2014/02/25/timenode";
	//此处需添加根据抄表点查找抄表时间点函数

	NodePath[3]=str_check_data.time_path[3];
	NodePath[4]=str_check_data.time_path[4];
	NodePath[6]=str_check_data.time_path[6];
	NodePath[7]=str_check_data.time_path[7];
	NodePath[9]=str_check_data.time_path[9];
	NodePath[10]=str_check_data.time_path[10];
	//读取该时间点下用户数目
	//Res = SDReadData((const char*)NodePath, (uint8 *)&max_usr_num, 2,(str_check_data.read_num-1)*2+6);
	Res = SDReadData((const char*)NodePath, (uint8 *)&max_usr_num, 2,6);
	if(Res!=NO_ERR)
	{
		;
	}
	if(str_check_data.user_id>(max_usr_num-1))
	{
		str_check_data.user_id = 1;
	}
	else if(str_check_data.user_id==0)
	{
		str_check_data.user_id = max_usr_num-1;
	}	
	//读取该时间点的路径						 
	Res = SDReadData((const char*)NodePath, (uint8 *)&TimeNode, 2,(str_check_data.read_num-1)*2+8);
	if(Res!=NO_ERR)
	{
		;
	}
	memcpy(Datapath, str_check_data.time_path, 17);
	HexToAscii_1(TimeNode,asiibuf);
	Datapath[12] = HexToAscii((TimeNode>>12)%16); 
	Datapath[13] = HexToAscii((TimeNode>>8)%16); 
	Datapath[14] = HexToAscii((TimeNode>>4)%16); 
	Datapath[15] = HexToAscii(TimeNode%16); 
	pbuf = buf;
	if(str_check_data.user_id <=1)
	{
		str_check_data.user_id = 1;
	}
	/*第0块存储热表数据，第1块以后存储时间通断面积法数据*/
	Res = SDReadData((const char*)Datapath, pbuf, sizeof(SDSave_Data),(str_check_data.user_id)*128); 
	if(Res!=NO_ERR)
	{
		;
	}
	pSaveData = (SDSave_Data*)pbuf;
	strDisplayData.user_id = pSaveData->user_id;
	strDisplayData.address = pSaveData->address;
	strDisplayData.area = pSaveData->area;
	strDisplayData.open_time = pSaveData->open_time;
	strDisplayData.room_temperature = pSaveData->room_temperature;
	strDisplayData.forward_temperature = pSaveData->forward_temperature;
	strDisplayData.return_temperature = pSaveData->return_temperature;
	strDisplayData.proportion_energy = pSaveData->proportion_energy;
	strDisplayData.total_energy = pSaveData->total_energy;

}

/****************************************************************************************************
**	函 数  名 称: Chose_time_data
**	函 数  功 能: 选择时间
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Chose_time_data(void)
{
	uint8 status;
	uint8 line1,line2,line3;
	uint8 column1,column2,column3;
	uint8 i;
	uint8 ASCIIData1[2];
	uint8 User_Num[3];
	static uint8 TimeNum = 0x01;
	static uint16 UserNum=1;
	
	uint8 ASCIIData[12];
	uint8 datebuf[6];
	
	SelectInfo TabMenu[] = {{2,14,9},{3,14,6},{4,9,34},{5,22,11}};//行，列，长度
	uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	bool OK_Flag = FALSE;
	bool Tab_Flag = TRUE;
	
	//默认显示第一抄表时间点
	ASCIIData1[0] = HexToAscii((TimeNum)>>4);
	ASCIIData1[1] = HexToAscii((TimeNum)&0x0f);

	User_Num[0]= HexToAscii(UserNum/100);
	User_Num[1]= HexToAscii((UserNum - (UserNum/100)*100)/10);
	User_Num[2]= HexToAscii(UserNum%10);

	
	//读当前时间
	ReadDateTime(&datebuf[0]);
	
	datebuf[4] = datebuf[4]&0x1F;//去掉星期
	for(i=0;i<6;i++)
	{
		ASCIIData[2*i] = HexToAscii(datebuf[5-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(datebuf[5-i]&0x0f);
	}
	str_check_data.user_id = UserNum;
	str_check_data.read_num = TimeNum;
	str_check_data.time_path[3]=ASCIIData[0];
	str_check_data.time_path[4]=ASCIIData[1];
	str_check_data.time_path[6]=ASCIIData[2];
	str_check_data.time_path[7]=ASCIIData[3];
	str_check_data.time_path[9]=ASCIIData[4];
	str_check_data.time_path[10]=ASCIIData[5];		
	FreshSetUserData(ASCIIData1,User_Num,ASCIIData);
	
	i = 0;
	line1 = TabMenu[0].startline;
	line2 = TabMenu[1].startline;//3
	line3 = TabMenu[2].startline;//3
	
	column1 = TabMenu[0].startcolumn;//14
	column2 = TabMenu[1].startcolumn;//16
	column3 = TabMenu[2].startcolumn;//16
	
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	while(OK_Flag == FALSE)
	{
		FreshSetUserData(ASCIIData1,User_Num,ASCIIData);
		
		if(Tab_Flag == TRUE)
		{
			ReverseShowLine_len(TabMenu[TabNum].startline,TabMenu[TabNum].startcolumn,TabMenu[TabNum].columnnum);
			status = GetKeyStatus();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
					if(TabNum == 0)
					{
						TabNum = MaxTabNum;
					}
					else
					{
						TabNum = TabNum - 1;
					}
					break;
				case KEY_DOWN://向下键
					if(TabNum == MaxTabNum)
					{
						TabNum = 0;
					}
					else
					{
						TabNum = TabNum + 1;
					}
					break;			
				case KEY_LEFT://向左键	
					break;
				case KEY_RIGHT://向右键	
					break;
				case KEY_ENTER://回车键
					if(TabNum == MaxTabNum)
					{
						OK_Flag = TRUE;
					}
					else
					{
						Tab_Flag = (!Tab_Flag);
					}
					break;
				case KEY_EXIT://返回键
					KeyFuncIndex = 30;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					return;			
					
				default:	
					break;
			}		
		}
		else
		{
			if(TabNum == 0)
			{
				ReverseShowChar(line1,column1);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(User_Num[i] == 0x39)
						{
							User_Num[i] = 0x30;
						}
						else
						{
							User_Num[i] = User_Num[i] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(User_Num[i] == 0x30)
						{
							User_Num[i] = 0x39;
						}
						else
						{
							User_Num[i] = User_Num[i] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line1,column1,User_Num[i]);
						if(i > 0)
						{
							column1 = column1 - 3;
							i = i - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line1,column1,User_Num[i]);
						if(i < 2)
						{
							column1 = column1 + 3;
							i = i + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						column1 = TabMenu[0].startcolumn;//0
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 32;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					
					default:	
						break;
				}

				UserNum = (User_Num[0]-0x30)*100+ (User_Num[1]-0x30)*10 + (User_Num[2]-0x30);
				str_check_data.user_id=UserNum;
			}
			else if(TabNum == 1)
			{
				ReverseShowChar(line2,column2);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(ASCIIData1[i] == 0x39)
						{
							ASCIIData1[i] = 0x30;
						}
						else
						{
							ASCIIData1[i] = ASCIIData1[i] + 1;
						}
						break;
					case KEY_DOWN://向下键
						if(ASCIIData1[i] == 0x30)
						{
							ASCIIData1[i] = 0x39;
						}
						else
						{
							ASCIIData1[i] = ASCIIData1[i] - 1;
						}	
						break;			
					case KEY_LEFT://向左键
						ShowChar(line2,column2,ASCIIData1[i]);
						if(i > 0)
						{
							column2 = column2 - 3;
							i = i - 1;
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line2,column2,ASCIIData1[i]);
						if(i < 1)
						{
							column2 = column2 + 3;
							i = i + 1;
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						column2 = TabMenu[1].startcolumn;//0					
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 32;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					
					default:	
						break;
				}

				TimeNum = (ASCIIData1[0]-0x30)*10+ (ASCIIData1[1]-0x30);
				str_check_data.read_num = TimeNum;
			}
			else if(TabNum == 2)
			{
				ReverseShowChar(line3,column3);
				status = GetKeyStatus();//有待修改为得到键盘的状态函数
				switch(status)
				{
					case KEY_UP://向上键
						if(i == 2)//判断月份最大只能到1
						{
							if(ASCIIData[i] == 0x31)
							{
								ASCIIData[i] = 0x30;
							}
							else
							{
								ASCIIData[i] = ASCIIData[i] + 1;
							}
						}
						else if(i == 3)
						{
							if(ASCIIData[2] == 0x31)
							{
								if(ASCIIData[i] == 0x32)
								{
									ASCIIData[i] = 0x30;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] + 1;
								}
							}
							else
							{
								if(ASCIIData[i] == 0x39)
								{
									ASCIIData[i] = 0x30;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] + 1;
								}
							}
						}
						else if(i == 4 )
						{
							if(ASCIIData[i] == 0x33)
							{
								ASCIIData[i] = 0x30;
							}
							else
							{
								ASCIIData[i] = ASCIIData[i] + 1;
							}
						}
						else if(i == 5)
						{
							if(ASCIIData[4] == 0x33)
							{
								if(ASCIIData[i] == 0x31)
								{
									ASCIIData[i] = 0x30;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] + 1;
								}
							}
							else
							{
								if(ASCIIData[i] == 0x39)
								{
									ASCIIData[i] = 0x30;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] + 1;
								}
							}
						}
						else
						{
							if(ASCIIData[i] == 0x39)
							{
								ASCIIData[i] = 0x30;
							}
							else
							{
								ASCIIData[i] = ASCIIData[i] + 1;
							}
						}
						break;
					case KEY_DOWN://向下键
						if(i == 2)//判断月份最大只能到1
						{
							if(ASCIIData[i] == 0x30)
							{
								ASCIIData[i] = 0x31;
							}
							else
							{
								ASCIIData[i] = ASCIIData[i] - 1;
							}
						}
						else if(i == 3)
						{
							if(ASCIIData[2] == 0x31)
							{
								if(ASCIIData[i] == 0x30)
								{
									ASCIIData[i] = 0x32;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] - 1;
								}
							}
							else
							{
								if(ASCIIData[i] == 0x30)
								{
									ASCIIData[i] = 0x39;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] - 1;
								}
							}
						}
						else if(i == 4 )
						{
							if(ASCIIData[i] == 0x30)
							{
								ASCIIData[i] = 0x33;
							}
							else
							{
								ASCIIData[i] = ASCIIData[i] - 1;
							}
						}
						else if(i == 5)
						{
							if(ASCIIData[4] == 0x33)
							{
								if(ASCIIData[i] == 0x30)
								{
									ASCIIData[i] = 0x31;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] - 1;
								}
							}
							else
							{
								if(ASCIIData[i] == 0x30)
								{
									ASCIIData[i] = 0x39;
								}
								else
								{
									ASCIIData[i] = ASCIIData[i] - 1;
								}
							}
						}
						else
						{
							if(ASCIIData[i] == 0x30)
							{
								ASCIIData[i] = 0x39;
							}
							else
							{
								ASCIIData[i] = ASCIIData[i] - 1;
							}
						}
						break;			
					case KEY_LEFT://向左键
						ShowChar(line3,column3,ASCIIData[i]);
						if(i > 0)
						{
							if(i%2)
							{
								column3 = column3 - 3;
							}
							else
							{
								column3 = column3 - 8;
							}
							i = i - 1;
							if((ASCIIData[2]==0x31)&&(ASCIIData[3]>0x32))
							{
								ASCIIData[3] = 0x30;
							}
							if((ASCIIData[4]==0x33)&&(ASCIIData[5]>0x31))
							{	
								ASCIIData[5] = 0x30;
							}
						}
						break;
					case KEY_RIGHT://向右键
						ShowChar(line1,column3,ASCIIData[i]);
						if(i < 5)
						{
							if(i%2)
							{
								column3= column3 + 8;
							}
							else
							{
								column3 = column3 + 3;
							}
							i = i + 1;
							if((ASCIIData[2]==0x31)&&(ASCIIData[3]>0x32))
							{
								ASCIIData[3] = 0x30;
							}
							if((ASCIIData[4]==0x33)&&(ASCIIData[5]>0x31))
							{	
								ASCIIData[5] = 0x30;
							}
						}
						break;
					case KEY_ENTER://回车键
						Tab_Flag = (!Tab_Flag);
						i = 0;
						column3 = TabMenu[0].startcolumn;//0	
						TabNum = TabNum + 1;//确定时自动转入下一个要设置的位置
						break;
					case KEY_EXIT://返回键
						KeyFuncIndex = 32;
						(*KeyTab[KeyFuncIndex].CurrentOperate)();
						return;			
					
					default:	
						break;
				}
				//此处需要加入时间转换函数，去查找文件
				str_check_data.time_path[3]=ASCIIData[0];
				str_check_data.time_path[4]=ASCIIData[1];
				str_check_data.time_path[6]=ASCIIData[2];
				str_check_data.time_path[7]=ASCIIData[3];
				str_check_data.time_path[9]=ASCIIData[4];
				str_check_data.time_path[10]=ASCIIData[5];			
			}
		}//end if(Tab_Flag == TRUE)
	}//end while	

    ShowMenuBottom(8);
	get_sd_display_data();
	KeyFuncIndex = 34;
	(*KeyTab[KeyFuncIndex].CurrentOperate)();
	return;

}
uint8  HexToAscii_1(uint16 Hex_bcd_code,uint8* AscBuf)//16位的HEX或转化为ASCII码，然后反方向存放在AscBuf中，长度为num
{
	uint8  num = 0;
	//uint8  TempBuf;
	uint16 b1,b2;
	b2 = Hex_bcd_code;
	if(b2==0)//Hex_bcd_code为0的情况
	{
		AscBuf[num] = 0x30;
		num = 1;
	}
	while(b2)
	{
		b1 = b2%16;
		AscBuf[num] = HexToAscii(b1);
		b2 = b2/16;
		num++;
	}
	return num;
}

void  DeciToAscii(uint16 code,uint8* AscBuf)//10进制的HEX或转化为ASCII码，然后反方向存放在AscBuf中，长度为num
{
	AscBuf[0] = HexToAscii(code/100);
	AscBuf[1] = HexToAscii((code - (code/100)*100)/10);
	AscBuf[2] = HexToAscii(code%10);
}

uint8	Hex_BcdToAscii_32(uint32 Hex_bcd_code,uint8* AscBuf)//16位的HEX或者BCD码转化为ASCII码，然后反方向存放在AscBuf中，长度为num
{
	uint8  num = 0;
	//uint8  TempBuf;
	uint32 b1,b2;
	b2 = Hex_bcd_code;
	if(b2==0)//Hex_bcd_code为0的情况
	{
		AscBuf[num] = 0x30;
		num = 1;
	}
	while(b2)
	{
		b1 = b2%16;
		AscBuf[num] = HexToAscii(b1);
		b2 = b2/16;
		num++;
	}
	return num;
}
/****************************************************************************************************
**	函 数  名 称: FreshDisplayData
**	函 数  功 能: 刷新显示数据
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void FreshDisplayData(void)
{
	uint8 ASCIIData[20];
	uint8 i,num;

	display_white();
	ShowMenuTop();
	ShowMenuBottom(25);

	memset(ASCIIData,'0', 20);
	ShowLineChar_HZ(1,0,&MenuHZ_user_num[0],4); //用户编号
	DeciToAscii(strDisplayData.user_id,ASCIIData);
	for(i=0;i<3;i++)
	{	
		ShowChar(1, 19+i*3, ASCIIData[i]);
	}
	
	ShowLineChar_HZ(1,28,&MenuHZ_user_area[0],2);//用户面积
	//DeciToAscii(strDisplayData.area,ASCIIData);
	num = Hex_BcdToAscii(strDisplayData.area,ASCIIData);
	for(i=0;i<num;i++)
	{	
		ShowChar(1, 38+i*3, ASCIIData[num-1-i]);
	}
	ShowHZ(1,38+i*3,0x0185);//㎡
	
	ShowLineChar_HZ(2,0,&MenuHZ_device_addr[0],4);//设备地址
	HexToAscii_1(strDisplayData.address, (uint8 *)ASCIIData);
	for(i=0;i<4;i++)
	{	
		ShowChar(2, 30+i*3, ASCIIData[3-i]);
	}
	
	ShowLineChar_HZ(3,0,&MenuHZ_open_time[0],4);//开阀时间
	num = Hex_BcdToAscii_32(strDisplayData.open_time,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(3, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	
	ShowChar(3, 22+i*3, 0x2E);
	ShowChar(3, 22+(i+1)*3, ASCIIData[1]);
	ShowChar(3, 22+(i+2)*3, ASCIIData[0]);
	ShowChar(3, 22+(i+3)*3, 'h');
	ShowLineChar_HZ(4,0,&MenuHZ_indoor_temp[0],4);//室内温度
	memset(ASCIIData,'0',sizeof(ASCIIData));
	num = Hex_BcdToAscii_32(strDisplayData.room_temperature,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(4, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	ShowChar(4, 22+2*3, 0x2E);
	ShowChar(4, 22+3*3, ASCIIData[1]);
	ShowChar(4, 22+4*3, ASCIIData[0]);
	ShowHZ(4,38,0x0184);//℃
	ShowLineChar_HZ(5,0,&MenuHZ_water_temp_in[0],4);//进水温度
	/*yangfei added for*/
	memset(ASCIIData,'0',sizeof(ASCIIData));
	/**/
	num = Hex_BcdToAscii_32(strDisplayData.forward_temperature,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(5, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	ShowChar(5, 22+2*3, 0x2E);
	ShowChar(5, 22+3*3, ASCIIData[1]);
	ShowChar(5, 22+4*3, ASCIIData[0]);
	ShowHZ(5,38,0x0184);//℃

	ShowLineChar_HZ(6,0,&MenuHZ_water_temp_out[0],4);//回水温度
	memset(ASCIIData,'0',sizeof(ASCIIData));
	num = Hex_BcdToAscii_32(strDisplayData.return_temperature,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(6, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	ShowChar(6, 22+2*3, 0x2E);
	ShowChar(6, 22+3*3, ASCIIData[1]);
	ShowChar(6, 22+4*3, ASCIIData[0]);
	ShowHZ(6,38,0x0184);//℃

	memset(ASCIIData,'0',sizeof(ASCIIData));
	ShowLineChar_HZ(7,0,&MenuHZ_proportion_energy[0],4);//分摊热量 
	num =Hex_BcdToAscii_32(strDisplayData.proportion_energy,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(7, 20+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(7, 20+i*3, ASCIIData[num-1-i]);
		}
	}
	
	ShowChar(7, 20+i*3, 0x2E);
	ShowChar(7, 20+(i+1)*3-2, ASCIIData[1]);
	ShowChar(7, 20+(i+2)*3-2, ASCIIData[0]);
	ShowChar(7, 20+(i+3)*3-2, 'k');
	ShowChar(7, 20+(i+4)*3-2, 'w');
	ShowChar(7, 20+(i+5)*3-2, 'h');
	ShowLineChar_HZ(8,0,&MenuHZ_total_energy[0],3);//总热量
	num =Hex_BcdToAscii_32(strDisplayData.total_energy,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(8, 15+i*3, 0x30);
		i++;
	}
	for(i=0;i<num-2;i++)
	{	
		ShowChar(8, 15+i*3, ASCIIData[num-1-i]);
	}
	ShowChar(8, 15+i*3, 0x2E);
	ShowChar(8, 15+(i+1)*3, ASCIIData[1]);
	ShowChar(8, 15+(i+2)*3, ASCIIData[0]);
	ShowChar(8, 15+(i+3)*3, 'k');
	ShowChar(8, 15+(i+4)*3, 'w');
	ShowChar(8, 15+(i+5)*3, 'h');
	
}


/****************************************************************************************************
**	函 数  名 称: Display_Data_Menu
**	函 数  功 能: 数据展示界面
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Display_Data_Menu(void)
{
	uint8 status;
	bool OK_Flag = FALSE;
	
	while(OK_Flag == FALSE)
	{
		FreshDisplayData();
		status = GetKeyStatuschaobiao();//得到键盘的状态
		switch(status)
		{
			case KEY_UP://向上键
			case KEY_RIGHT://向右键	
			{
				str_check_data.user_id++;
				get_sd_display_data();
				break;
			}
			case KEY_DOWN://向下键
			case KEY_LEFT://向左键
			{
				str_check_data.user_id--;
				get_sd_display_data();
				break;
			}			
			case KEY_ENTER://回车键
			{
				break;
			}
			case KEY_EXIT://返回键
			{
				KeyFuncIndex = 32;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				OK_Flag = TRUE;
				break;
			}		
			default:	
				break;
		}
	}	
}


/****************************************************************************************************
**	函 数  名 称: Chose_metric_method
**	函 数  功 能: 选择计量方法
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Chose_metric_method(void)
{
	uint8 status;
	uint8 now_line =2;//显示当前光标反显的行
	bool OK_Flag = FALSE;
	uint8 DeviceType;
	while(OK_Flag == FALSE)
	{
		FreshMethodshow(now_line);
		status = GetKeyStatuschaobiao();//得到键盘的状态
		switch(status)
		{
			case KEY_UP://向上键
			{
				if(now_line>2)
				{
					now_line--;
				}
				else
				{
					now_line = 4;
				}
				break;
			}
			case KEY_DOWN://向下键
			{
				if(now_line<4)
				{
					now_line++;
				}
				else
				{
					now_line = 2;
				}
				break;
			}			
			case KEY_LEFT://向左键	
			case KEY_RIGHT://向右键	
			{
				break;
			}
			case KEY_ENTER://回车键
			{
				OK_Flag = TRUE;
				if(now_line ==2)
				{
					DeviceType = HEAT_METER_TYPE;
				}
				else if(now_line ==3)/*时间通断面积法*/
				{
					DeviceType = TIME_ON_OFF_AREA_TYPE;
				}
				else if(now_line ==4)/*电子式热量表*/
				{
					DeviceType = HEATCOST_METER_TYPE;
				}
				else
				{
					DeviceType = HEAT_METER_TYPE;
				}
				/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
				SaveDeviceType(DeviceType);
				/*end:yangfei added 20140226 */
				now_line= 8;
				FreshMethodshow(now_line);
				break;
				
			}
			case KEY_EXIT://返回键
			{
				KeyFuncIndex = 31;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				OK_Flag = TRUE;
				break;
			}		
			default:	
				break;
		}
	}
	
}
/****************************************************************************************************
**	函 数  名 称: FactroytestSDCardData
**	函 数  功 能: 刷新出厂测试显示数据
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
	typedef struct{
			uint16 WriteFlag;					//该区域是否被写过数据
			uint16 MeterStoreNums;				//共存储多少块表档案信息
		}MeterFilePara1;

uint8  FactroytestSDCardData(void)//muxiaoqing test

{
//	uint8 ASCIIData[20];
//	uint8 i,num;
	uint8 Err 	= 0;
//	char  NodePath[] = "/2012/12/24/test.txt";;
	//uint16 testNum			= 16;							//test
	
	MeterFilePara1	para;
	
	TermParaSaveType TermPara;
	if(!autotestmode)
		{
        	display_white();
        	ShowMenuTop();
        	ShowMenuBottom(25);
        	ShowLineChar_HZ(2,2,&MenuHZTesting[0],6);//
        //	memset(ASCIIData,'0', 20);
        	ShowLineChar_HZ(1,0,&Sdtest[0],3); //sd 卡
        	
			OSTimeDly(OS_TICKS_PER_SEC);
		}
//	DeciToAscii(strDisplayData.user_id,ASCIIData);
	manualtestmode = TRUE;

	para.WriteFlag = 0xAA55;
	para.MeterStoreNums = 0x0016;

	Err = SDSaveData("/TEST_FILE_ADDR", &para, sizeof(MeterFilePara1),0); 
	if(Err!=NO_ERR)	
	  {
	   // debug("%s %d read TIME_NODE_ADDR err=%d!\r\n",__FUNCTION__,__LINE__,Err);
		
		//len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
	//	len = sprintf((char*)&sdtestBuff[0], "%s", sdtestwriterr);
	//	ShowLineChar(3, 7, &sdtestBuff[0], len);
		return Err=1;
	  }
	else
		{
			   // debug("%s %d read TIME_NODE_ADDR err=%d!\r\n",__FUNCTION__,__LINE__,Err);
				
				//len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
		//		len = sprintf((char*)&sdtestBuff[0], "%s", sdtestwritOK);
		//		ShowLineChar(3, 7, &sdtestBuff[0], len);
        	//memset(sdtestBuff,0,sizeof(sdtestBuff));
			
			para.WriteFlag = 0x00;
			para.MeterStoreNums = 0x00;
        	//testNum = 0;
			MakeFile("/test");
			do{
				OSMutexPend (FlashMutex,0,&Err);
				Err = SDReadData("/test", &TermPara, sizeof(TermParaSaveType), 0);
				OSMutexPost (FlashMutex);
				if(Err!=NO_ERR) 
				  {
					debug("%s %d read test err=%d!\r\n",__FUNCTION__,__LINE__,Err);
				  }
			}while(Err!=NO_ERR);
			/*end:yangfei modified 2013-4-3 */
			OSMutexPend (FlashMutex,0,&Err);
			Err = SDReadData("/TEST_FILE_ADDR", &para, sizeof(MeterFilePara1), 0);
			OSMutexPost (FlashMutex);
        	
			
        	if(Err!=NO_ERR)
        	{
        		//len = sprintf((char*)&sdtestBuff[0], "%s", sdtestreaderr);
        		//ShowLineChar(5, 7, &sdtestBuff[0], len);
        		return Err=2;
        	}
			
			else if(para.WriteFlag == 0xAA55)
			
			{
        		//len = sprintf((char*)&sdtestBuff[0], "%s", sdtestOK);
        		//ShowLineChar(7, 7, &sdtestBuff[0], len);
        		return 0;

			}
			else
				{
				return Err =3;
				}
		}
#if 0
	for(i=0;i<3;i++)
	{	
		ShowChar(1, 19+i*3, ASCIIData[i]);
	}
	
	ShowLineChar_HZ(1,28,&MenuHZ_user_area[0],2);//用户面积
	//DeciToAscii(strDisplayData.area,ASCIIData);
	num = Hex_BcdToAscii(strDisplayData.area,ASCIIData);
	for(i=0;i<num;i++)
	{	
		ShowChar(1, 38+i*3, ASCIIData[num-1-i]);
	}
	ShowHZ(1,38+i*3,0x0185);//㎡
	
	ShowLineChar_HZ(2,0,&MenuHZ_device_addr[0],4);//设备地址
	HexToAscii_1(strDisplayData.address, (uint8 *)ASCIIData);
	for(i=0;i<4;i++)
	{	
		ShowChar(2, 30+i*3, ASCIIData[3-i]);
	}
	
	ShowLineChar_HZ(3,0,&MenuHZ_open_time[0],4);//开阀时间
	num = Hex_BcdToAscii_32(strDisplayData.open_time,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(3, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	
	ShowChar(3, 22+i*3, 0x2E);
	ShowChar(3, 22+(i+1)*3, ASCIIData[1]);
	ShowChar(3, 22+(i+2)*3, ASCIIData[0]);
	ShowChar(3, 22+(i+3)*3, 'h');
	ShowLineChar_HZ(4,0,&MenuHZ_indoor_temp[0],4);//室内温度
	memset(ASCIIData,'0',sizeof(ASCIIData));
	num = Hex_BcdToAscii_32(strDisplayData.room_temperature,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(4, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	ShowChar(4, 22+2*3, 0x2E);
	ShowChar(4, 22+3*3, ASCIIData[1]);
	ShowChar(4, 22+4*3, ASCIIData[0]);
	ShowHZ(4,38,0x0184);//℃
	ShowLineChar_HZ(5,0,&MenuHZ_water_temp_in[0],4);//进水温度
	/*yangfei added for*/
	memset(ASCIIData,'0',sizeof(ASCIIData));
	/**/
	num = Hex_BcdToAscii_32(strDisplayData.forward_temperature,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(5, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	ShowChar(5, 22+2*3, 0x2E);
	ShowChar(5, 22+3*3, ASCIIData[1]);
	ShowChar(5, 22+4*3, ASCIIData[0]);
	ShowHZ(5,38,0x0184);//℃

	ShowLineChar_HZ(6,0,&MenuHZ_water_temp_out[0],4);//回水温度
	memset(ASCIIData,'0',sizeof(ASCIIData));
	num = Hex_BcdToAscii_32(strDisplayData.return_temperature,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(3, 22+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(6, 22+i*3, ASCIIData[num-1-i]);
		}
	}
	ShowChar(6, 22+2*3, 0x2E);
	ShowChar(6, 22+3*3, ASCIIData[1]);
	ShowChar(6, 22+4*3, ASCIIData[0]);
	ShowHZ(6,38,0x0184);//℃

	memset(ASCIIData,'0',sizeof(ASCIIData));
	ShowLineChar_HZ(7,0,&MenuHZ_proportion_energy[0],4);//分摊热量 
	num =Hex_BcdToAscii_32(strDisplayData.proportion_energy,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(7, 20+i*3, 0x30);
		i++;
	}
	else
	{
		for(i=0;i<num-2;i++)
		{	
			ShowChar(7, 20+i*3, ASCIIData[num-1-i]);
		}
	}
	
	ShowChar(7, 20+i*3, 0x2E);
	ShowChar(7, 20+(i+1)*3-2, ASCIIData[1]);
	ShowChar(7, 20+(i+2)*3-2, ASCIIData[0]);
	ShowChar(7, 20+(i+3)*3-2, 'k');
	ShowChar(7, 20+(i+4)*3-2, 'w');
	ShowChar(7, 20+(i+5)*3-2, 'h');
	ShowLineChar_HZ(8,0,&MenuHZ_total_energy[0],3);//总热量
	num =Hex_BcdToAscii_32(strDisplayData.total_energy,ASCIIData);
	if(num<=2)//判断是否只有小数
	{
		i=0;
		ShowChar(8, 15+i*3, 0x30);
		i++;
	}
	for(i=0;i<num-2;i++)
	{	
		ShowChar(8, 15+i*3, ASCIIData[num-1-i]);
	}
	ShowChar(8, 15+i*3, 0x2E);
	ShowChar(8, 15+(i+1)*3, ASCIIData[1]);
	ShowChar(8, 15+(i+2)*3, ASCIIData[0]);
	ShowChar(8, 15+(i+3)*3, 'k');
	ShowChar(8, 15+(i+4)*3, 'w');
	ShowChar(8, 15+(i+5)*3, 'h');
	#endif
	
	manualtestmode = FALSE;
}

/****************************************************************************************************
**	函 数  名 称:factory_test_method_sd
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void factory_test_method_sd(void)
{
	uint8 status;
	uint8 now_line =2;//显示当前光标反显的行
	bool OK_Flag = FALSE;
	//uint8 DeviceType;
	uint8 err;	
	uint8 len;
    char  sdtestwriterr[] = "SD write erro ";	
    char  sdtestwritOK[] = "SD write OK ";
    char  sdtestreaderr[] = "SD read erro ";	
    char  sdtestOK[] = "SD test OK ";	
	uint8 sdtestBuff[15] = {0};
	err = FactroytestSDCardData();
	while(OK_Flag == FALSE)
	{
	    
		display_white();
		ShowMenuTop();
		ShowMenuBottom(25);
		//FreshMethodshow(now_line);
		
		if(err == 1) 
		  {
		   // debug("%s %d read TIME_NODE_ADDR err=%d!\r\n",__FUNCTION__,__LINE__,Err);
			
			//len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
			len = sprintf((char*)&sdtestBuff[0], "%s", sdtestwriterr);
			ShowLineChar(3, 7, &sdtestBuff[0], len);
		  }
		else if(err == 2)
		  {
        	 len = sprintf((char*)&sdtestBuff[0], "%s", sdtestreaderr);
        	 ShowLineChar(3, 7, &sdtestBuff[0], len);
          }
		else if (err ==0)
		  {
        	 len = sprintf((char*)&sdtestBuff[0], "%s", sdtestOK);
        	 ShowLineChar(3, 7, &sdtestBuff[0], len);

			}	
		status = GetKeyStatuschaobiao();//得到键盘的状态
		switch(status)
		{
			case KEY_UP://向上键
			{
				if(now_line>2)
				{
					now_line--;
				}
				else
				{
					now_line = 4;
				}
				break;
			}
			case KEY_DOWN://向下键
			{
				if(now_line<4)
				{
					now_line++;
				}
				else
				{
					now_line = 2;
				}
				break;
			}			
			case KEY_LEFT://向左键	
			case KEY_RIGHT://向右键	
			{
				break;
			}
			case KEY_ENTER://回车键
			{
				#if 0
				OK_Flag = TRUE;
				if(now_line ==2)
				{
					DeviceType = HEAT_METER_TYPE;
				}
				else if(now_line ==3)/*时间通断面积法*/
				{
					DeviceType = TIME_ON_OFF_AREA_TYPE;
				}
				else if(now_line ==4)/*电子式热量表*/
				{
					DeviceType = HEATCOST_METER_TYPE;
				}
				else
				{
					DeviceType = HEAT_METER_TYPE;
				}
				/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
				SaveDeviceType(DeviceType);
				/*end:yangfei added 20140226 */
				now_line= 8;
				FreshMethodshow(now_line);
				#endif
				break;
				
			}
			case KEY_EXIT://返回键
			{
				KeyFuncIndex = 38;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				OK_Flag = TRUE;
				break;
			}		
			default:	
				break;
		}
	}
	
}
/****************************************************************************************************
**	函 数  名 称: Set_SystemTime
**	函 数  功 能: 终端时间设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Factory_test_Set_SystemTime(void)  //终端时间设置

{
	uint8 err;
	uint8 line1,line2;
	uint8 column1,column2;
	uint8 i,j;
	uint8 datebuf[6];
	uint8 ASCIIData[12];
	uint8 HexData[6];
	
	SelectInfo TabMenu[] = {{2,0,33},{3,0,33},{4,22,11}};//行，列，长度
	//uint8 TabNum = 0;
	uint8 MaxTabNum = 0;
	//bool OK_Flag = FALSE;
	//bool Tab_Flag = TRUE;
	
	if(!autotestmode)
		{
        	display_white();
        	ShowMenuTop();
        	ShowMenuBottom(25);
        	ShowLineChar_HZ(2,2,&MenuHZTesting[0],6);//
		}
		
	//读当前时间
	//UserReadDateTime(&datebuf[0]);//秒分时日月年
	ReadDateTime(&datebuf[0]);
	/*begin:yangfei added 2013-06-04 for 时间有效性判断 */
	#if 0
	for(i=0;i<6;i++)
		{
	debug("%d ",datebuf[i]);

		}
	#endif
	if(datebuf[0]>60)
		{
		datebuf[0]=1;
		datebuf[1]=1;
		datebuf[2]=1;
		datebuf[3]=1;
		datebuf[4]=1;
		datebuf[5]=9;
		}
	/*end:yangfei added 2013-06-04 for 时间有效性判断 */
	datebuf[4] = datebuf[4]&0x1F;//去掉星期
	for(i=0;i<6;i++)
	{
		ASCIIData[2*i] = HexToAscii(datebuf[5-i]>>4);
		ASCIIData[2*i+1] = HexToAscii(datebuf[5-i]&0x0f);
	}
	i = 0;
	j = 6;
	line1 = TabMenu[0].startline;//2
	line2 = TabMenu[1].startline;//3
	column1 = TabMenu[0].startcolumn;//0
	column2 = TabMenu[1].startcolumn;//0
	
	MaxTabNum = (sizeof(TabMenu)/sizeof(SelectInfo)) - 1;//最大下标
	//end while	
	
	//==========================================
	//此处需要加入设置时间函数，然后根据返回值的结果判断是否设置成功
	for(i=0;i<6;i++)//将ASC码转换为BCD，并按照秒---年的顺序存放
	{
		HexData[5-i] = ((ASCIIData[2*i]-0x30)<<4) + (ASCIIData[2*i+1]-0x30);
	}
	HexData[4] = HexData[4]|0x20;
	err = TimeCheck(&HexData[0]);
	if(err != 0)//不合要求
	{
		if(err == 1)///日期不合要求
		{
			ShowMenuBottom(7);
		}
		else if(err == 2)//时间不合法
		{
			ShowMenuBottom(8);
		}
//		OSTimeDly(OS_TICKS_PER_SEC*2);
//		KeyFuncIndex = 37;
//		(*KeyTab[KeyFuncIndex].CurrentOperate)();
//		return;
	}
	
	err = UserSetDateTime(&HexData[0]);
	ShowMenuBottom(2);//需要根据时间设置函数的返回值，在底部状态栏显示设置是否成功
//	OSTimeDly(OS_TICKS_PER_SEC*2);
	return err;
}

/****************************************************************************************************
**	函 数  名 称:factory_test_method_time
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void factory_test_method_time(void)
{
	uint8 status;
	uint8 now_line =2;//显示当前光标反显的行
	bool OK_Flag = FALSE;
	//uint8 DeviceType;	
	uint8 Err 	= 0;	
	uint8 len;
    char  timetesterr[] = "time test erro ";	
    char  timetestOK[] = "time test OK ";	
	uint8 timetestBuff[15] = {0};
	
	Err = Factory_test_Set_SystemTime();
	while(OK_Flag == FALSE)
	{
	
	display_white();
	ShowMenuTop();
	ShowMenuBottom(25);
		if(Err!=NO_ERR) 
		  {
		   // debug("%s %d read TIME_NODE_ADDR err=%d!\r\n",__FUNCTION__,__LINE__,Err);
			
			//len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
			len = sprintf((char*)&timetestBuff[0], "%s", timetesterr);
			ShowLineChar(3, 7, &timetestBuff[0], len);
			
		  }
		else
			{
				   // debug("%s %d read TIME_NODE_ADDR err=%d!\r\n",__FUNCTION__,__LINE__,Err);
					
					//len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
					len = sprintf((char*)&timetestBuff[0], "%s", timetestOK);
					ShowLineChar(3, 7, &timetestBuff[0], len);
					
			}
		
		status = GetKeyStatuschaobiao();//得到键盘的状态
		switch(status)
		{
			case KEY_UP://向上键
			{
				if(now_line>2)
				{
					now_line--;
				}
				else
				{
					now_line = 4;
				}
				break;
			}
			case KEY_DOWN://向下键
			{
				if(now_line<4)
				{
					now_line++;
				}
				else
				{
					now_line = 2;
				}
				break;
			}			
			case KEY_LEFT://向左键	
			case KEY_RIGHT://向右键	
			{
				break;
			}
			case KEY_ENTER://回车键
			{
				#if 0
				OK_Flag = TRUE;
				if(now_line ==2)
				{
					DeviceType = HEAT_METER_TYPE;
				}
				else if(now_line ==3)/*时间通断面积法*/
				{
					DeviceType = TIME_ON_OFF_AREA_TYPE;
				}
				else if(now_line ==4)/*电子式热量表*/
				{
					DeviceType = HEATCOST_METER_TYPE;
				}
				else
				{
					DeviceType = HEAT_METER_TYPE;
				}
				/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
				SaveDeviceType(DeviceType);
				/*end:yangfei added 20140226 */
				now_line= 8;
				FreshMethodshow(now_line);
				#endif
				break;

			}
			case KEY_EXIT://返回键
			{
				KeyFuncIndex = 39;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				OK_Flag = TRUE;
				break;
			}		
			default:	
				break;
		}
	}
	
}
/****************************************************************************************************
**	函 数  名 称:factory_test_method_GPRS
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void factory_test_method_GPRS(void)
{

	uint8 status;
	uint8 now_line =2;//显示当前光标反显的行
	bool OK_Flag = FALSE;
	//uint8 DeviceType;
	
//	uint8 tmpmid;
	uint8 err;
	
	char Ats_AT[]="AT\r";
	char Ata_OK[]="\r\nOK\r\n";
	uint8 len;
    char  GPRStesterr[] = "GPRS test erro ";	
    char  GPRStestOK[] = "GPRS test OK ";	
	uint8 GPRStestBuff[15] = {0};
	
	if(!autotestmode)
    {
         display_white();
     	ShowMenuTop();
     	ShowMenuBottom(25);
     	
     	ShowLineChar_HZ(2,2,&MenuHZTesting[0],6);//
	}
	err = CMD_AT_RP(Ats_AT,Ata_OK,NULL,OS_TICKS_PER_SEC,1,TRUE);
	while(OK_Flag == FALSE)
	{
		//FreshMethodshow(now_line);
		//FactroytestSDCardData();
		
		
		display_white();
		ShowMenuTop();
		ShowMenuBottom(25);

	
		if(err!=NO_ERR) 
		{
			//len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
			len = sprintf((char*)&GPRStestBuff[0], "%s", GPRStesterr);
			ShowLineChar(3, 7, &GPRStestBuff[0], len);
		}		
		else
			{
				   // debug("%s %d read TIME_NODE_ADDR err=%d!\r\n",__FUNCTION__,__LINE__,Err);
					
					//len = sprintf((char*)&ASCIIData[0],"%d",lTimingState.TimingMeterNums);
					len = sprintf((char*)&GPRStestBuff[0], "%s", GPRStestOK);
					ShowLineChar(3, 7, &GPRStestBuff[0], len);
					
			}
		
		status = GetKeyStatuschaobiao();//得到键盘的状态
		switch(status)
		{
			case KEY_UP://向上键
			{
				if(now_line>2)
				{
					now_line--;
				}
				else
				{
					now_line = 4;
				}
				break;
			}
			case KEY_DOWN://向下键
			{
				if(now_line<4)
				{
					now_line++;
				}
				else
				{
					now_line = 2;
				}
				break;
			}			
			case KEY_LEFT://向左键	
			case KEY_RIGHT://向右键	
			{
				break;
			}
			case KEY_ENTER://回车键
			{
				#if 0
				OK_Flag = TRUE;
				if(now_line ==2)
				{
					DeviceType = HEAT_METER_TYPE;
				}
				else if(now_line ==3)/*时间通断面积法*/
				{
					DeviceType = TIME_ON_OFF_AREA_TYPE;
				}
				else if(now_line ==4)/*电子式热量表*/
				{
					DeviceType = HEATCOST_METER_TYPE;
				}
				else
				{
					DeviceType = HEAT_METER_TYPE;
				}
				/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
				SaveDeviceType(DeviceType);
				/*end:yangfei added 20140226 */
				now_line= 8;
				FreshMethodshow(now_line);
				#endif
				break;
				
			}
			case KEY_EXIT://返回键
			{
				KeyFuncIndex = 40;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				OK_Flag = TRUE;
				break;
			}		
			default:	
				break;
		}
	}
	
}
uint8 IfconfirmByte(uint8 data,char* confirm,uint8 p)
{
	if(data==(uint8)(confirm[p])){
		return 0;//匹配首选回应成功
	}
	else{
		return 1;//匹配失败
	}
	
}
        uint16 QueuetestNData(void *Buf)//倪 可重入
{
    uint16 temp;
//	OS_CPU_SR cpu_sr;
    
    temp = 0;                                                   /* 队列无效返回0 */
    if (Buf != NULL)
    {
      //  OS_ENTER_CRITICAL();
        temp = ((DataQueue *)Buf)->NData;
       // OS_EXIT_CRITICAL();
    }
    return temp;
}

uint8 PorttestGetHead(uint8 device,char* confirm,uint16 OutTime,uint8 Find, uint8 upORdown)

{
	uint8 err =0;
	uint8 headlen;
	char  TmpBuf[64];//调试观察用
	uint8 test_buff[512];
  #if 1		
	headlen=strlen(confirm);
	if(headlen==0||headlen>64){//headlen 必须大于0 ，即HeadStr必须非空;HeadStr最多允许HEAD_MAX_LEN个字符
		return 0xff;
	}
	memset(test_buff, 0x00, 512);	
	memset(TmpBuf, 0x00, 64);
	#if 1
	if(upORdown)
		{
        	switch(device)
        	{
        		case UP_COMMU_DEV_ZIGBEE:						//针对于Zigbee发送数据，每次100个字节，每次发送间隔333MS
        				{
        			//		QueueFlush((void*)UART4RecQueue); //清空接收队列
        		//			QueueNData((void*)UART4RecQueue); //清空接收队列
        		while((IRQ4testBuf[strlen((char*)IRQ4testBuf)-1])!=0x0a)//等中断
        			{
        			
					OSTimeDly(OS_TICKS_PER_SEC/4);
                     testPortreadNum++;
					 if(testPortreadNum==2)	
					 	break;
				    }
				testPortreadNum = 0;
				    if(strlen((char*)IRQ4testBuf)==0)
						err = 1;
					     //   err = strcmp(confirm, (char*)IRQ4testBuf);
        					
        					break;
        				}
				
					case UP_COMMU_DEV_485:
        				{
					#if 0		
							//count = QueueNData(UART5RecQueue);
							count = 32;
							
							for(i=0; i<count; i++)
							{
								err=UpGetch(UP_COMMU_DEV_485,buf,OS_TICKS_PER_SEC/5);
							   if(!err)
									IRQ5testBuf[i] =  *buf;
								else
								{
									break;
								}
							}
					#endif	
					
					while((IRQ5testBuf[strlen((char*)IRQ5testBuf)-1])!=0x0a)//等中断
						{
						
						OSTimeDly(OS_TICKS_PER_SEC/4);
						 testPortreadNum++;
						 if(testPortreadNum==2) 
							break;
						}
					testPortreadNum = 0;
							//	err = strcmp(confirm, (char*)IRQ5testBuf);
							
							if(strlen((char*)IRQ5testBuf)==0)
								err = 1;
			//		if(err == NO_ERR)		
        			//		QueueFlush((void*)UART4RecQueue); //清空接收队列
        	//				QueueNData((void*)UART5RecQueue); //清空接收队列
        	
        					
        					break;
        				}
					case UP_COMMU_DEV_232:
        				{
        			//		QueueFlush((void*)UART4RecQueue); //清空接收队列
        			//		QueueNData((void*)USART1RecQueue); //清空接收队列
        			
					while((IRQ1testBuf[strlen((char*)IRQ1testBuf)-1])!=0x0a)//等中断
						{
						OSTimeDly(OS_TICKS_PER_SEC/4);
						 testPortreadNum++;
						 if(testPortreadNum==2) 
							break;
						}
					testPortreadNum = 0;
        				//	err = strcmp(confirm, (char*)IRQ1testBuf);
						if(strlen((char*)IRQ1testBuf)==0)
							err = 1;
        					
        					break;
        				}
					case UP_COMMU_DEV_GPRS:
        				{
        			//		QueueFlush((void*)UART4RecQueue); //清空接收队列
        			//		QueueNData((void*)USART3RecQueue_At); //清空接收队列
        			
					while((IRQ3testBuf[strlen((char*)IRQ3testBuf)-1])!=0x0a)//等中断
						{
						
						OSTimeDly(OS_TICKS_PER_SEC/4);
						 testPortreadNum++;
						 if(testPortreadNum==2) 
							break;
						}
					testPortreadNum = 0;
        				//	err = strcmp(confirm, (char*)IRQ3testBuf);
						if(strlen((char*)IRQ3testBuf)==0)
							err = 1;
        					
        					break;
        				}					
        	}
		}
	else
		{
		
		switch(device)
			{
			case DOWN_COMM_DEV_MBUS:						//针对于Zigbee发送数据，每次100个字节，每次发送间隔333MS
					{
				//		QueueFlush((void*)UART4RecQueue); //清空接收队列
				//		QueueNData((void*)USART2RecQueue); //清空接收队列
				
				while((IRQ1testBuf[strlen((char*)IRQ2testBuf)-1])!=0x0a)//等中断
        			{
        			
					OSTimeDly(OS_TICKS_PER_SEC/4);
                     testPortreadNum++;
					 if(testPortreadNum==2)	
					 	break;
				    }
				testPortreadNum = 0;
        				//err = strcmp((char*)confirm, (char*)IRQ2testBuf);
						
						if(strlen((char*)IRQ2testBuf)==0)
							err = 1;
						break;
					}

		    }
		}

	
//	err = strcmp(gDebugRcv, data_buf);
	#else
	if(upORdown)
	err=UpGetch(device,&data,OutTime);
	else
	err=DuGetch(device,&data,OutTime);		
	if(err){//超时
	//	gtmpnum++;//为了在此处能设置断点
		return err;	
	}
  /*begin:yangfei added 2013-02-26 for test */
    test_buff[i++] = data;
	/*end:yangfei added 2013-02-26 for test */
	n=500;///查找情况下最多接收的字符数
	flag=0;
	p=0;
	while(n>1){
		n--;
		//if(data!=(uint8)(HeadStr[p])){
		if(IfconfirmByte(data,confirm,p)>=2){
	 		if(Find==TRUE){
	 			if(p>0){//正在匹配HeadStr的第1个字节以后的字节（已匹配p个字节 ，匹配第p+1个字节出错）
	 				p=0;     //
	 				continue;//重新开始匹配HeadStr的第1个字节
	 			}
	 		}
	 		else{
	 			flag=0xfe;//表示失败，但还是要接收完headlen个字符，所以并不返回
	 			TmpBuf[p]=data;//调试观察用
	 			TmpBuf[p+1]='\0';//便于调试观察
	 			p++;
	 			
	 		}
	 	}
	 	else{
	 		TmpBuf[p]=data;//调试观察用
	 		TmpBuf[p+1]='\0';//便于调试观察
	 		p++;
			
	 	}
					
		if(p==headlen){//匹配完成
		//	gtmpnum++;//为了在此处能设置断点
			break;
		}
	//	err=UpGetch(device,&data,OutTime);//OS_TICKS_PER_SEC/5
	
	if(upORdown)
	err=UpGetch(device,&data,OutTime);
	else
	err=DuGetch(device,&data,OutTime);		
		if(err){//
			return 0xfe;	//接收中间字符时超时  //ncq080819 这里也可能是正在匹配第1个字符！所以上面超时时间应怎么给还不好办，现在还不合理
		}
         /*begin:yangfei added 2013-02-26 for test */
         test_buff[i++] = data;
         if(i>=100)
          {
          //debug("\r\n%s\r\n",test_buff);
          #if  0
          OSMutexPend (FlashMutex,0,&Err);
		  SDSaveData("test.txt",test_buff, 512, 512*count);
          OSMutexPost (FlashMutex);
          #endif
          i = 0;
          count++;
          }
         /*end:yangfei added 2013-02-26 for test */
	}
	if(n==1){//查找次数超限制，应返回失败
		flag=0xfd;
	}
	#endif
	#endif
	//return flag;
	
	return  err;
}
void Test_SuspendGprsRecTask(void)
{
	uint8 err;
	//在挂起GPRS接收任务前获取它可能正在占用的信号量GprsXmzSem,此信号量上行发送任务也可能申请,
	//如果不做此处理,可能导致上行发送任务死掉
	OSSemPend(GprsXmzSem, 0, &err);
	OSTaskSuspend(PRIO_TASK_GPRS_IPD);//挂起GPRS接收任务
	OSSemPost(GprsXmzSem);
}
uint8  Randtest8(void)
{
	uint8 Randtest8;
//	uint8 Time[6];

//	uint32 seed,randu32;
	CPU_SR	cpu_sr;
	
	OS_ENTER_CRITICAL();
	//seed=*((uint32*)Time);//秒分时日作随机种子
 //   srand(time(NULL));
    
   // Randtest8=1+(int)(10.0*rand()/(RAND_MAX+1.0));
   
  Randtest8= 1+(rand()%50);
   
   
	OS_EXIT_CRITICAL();
	
	//Randtest8=randu32%1000000;
	return Randtest8;
	
}

uint8 CMD_Port_test_Check(uint8 device,char *str,char *confirm,uint8 upORdown)// 1 ->up 0->down
{
	uint8 err =0;
	//UGprsWriteStr(str);
    char  Debugtesting[] = "测试中...";	
    uint8  Debugtestingbuf[24]={0} ;	
	//uint8 status;
	uint8 len;
//	uint8 i =0;
//	uint8 PreSmybol[30]={0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
	uint8 PreSmybol[512];

    len = sprintf((char*)&Debugtestingbuf[0], "%s", Debugtesting);
//	ShowLineChar(3, 7, &Debugtestingbuf[0], len);
	if(!autotestmode)
		{
        	ShowLineChar_HZ(2,2,&MenuHZTesting[0],6);//
		}
	memset(PreSmybol, Randtest8(), 512);
	//memset(PreSmybol, 0xaa, 512);
	#if 0
	for(i = 0; i<512; i ++)
		{
          PreSmybol[i] = Randtest8();
	    }
	#endif
	
#if 1	
		OSTaskSuspend(PRIO_TASK_UART_SHELL);	
		OSTaskSuspend(PRIO_FLASH_MUTEX_PIP);	
		OSTaskSuspend(PRIO_TASK_MANAGE_DEBUG_INFO); 
		OSTaskSuspend(PRIO_TASK_GPRS_IPD);	
		//Test_SuspendGprsRecTask();
		OSTaskSuspend(PRIO_TASK_UP_SEND);	
		OSTaskSuspend(PRIO_TASK_UP_ANAL_FR);	
		OSTaskSuspend(PRIO_TASK_UP_REC_GPRS);	
		OSTaskSuspend(PRIO_TASK_UP_REC_ZIGBEE); 
		OSTaskSuspend(PRIO_TASK_UP_REC_RS485);	
		OSTaskSuspend(PRIO_TASK_UP_REC_RS232);	
		OSTaskSuspend(PRIO_TASK_GPRS_MANA); 
		OSTaskSuspend(PRIO_TASK_CLOCK); 
		OSTaskSuspend(PRIO_TASK_READ_ALL_CUR);	
		OSTaskSuspend(PRIO_TASK_LOG);	
#endif	
	if(upORdown)
		{
	#if 1	
	UpDevSend(device,  (uint8*)str,	strlen(str)); 
	#else
        	//for(i=30;i>0;i--)
        		{
                	UpDevSend(device,  PreSmybol,	512); 
	//				OSTimeDly(OS_TICKS_PER_SEC*2);
        		}
	#endif		
		}
	else
	DuSend(device,  (uint8*)str,	strlen(str));	
	
	//forbidsending1 = 0;
//	OSTimeDly(OS_TICKS_PER_SEC);
//	OSTimeDly(OS_TICKS_PER_SEC/2);
#if 0
	if(upORdown)
		{
        	switch(device)
        	{
        		case UP_COMMU_DEV_ZIGBEE:						//针对于Zigbee发送数据，每次100个字节，每次发送间隔333MS
        				{
        					QueueFlush((void*)UART4RecQueue); //清空接收队列
        					QueueNData((void*)UART4RecQueue); //清空接收队列
        					
        					break;
        				}
				
					case UP_COMMU_DEV_485:
        				{
        					QueueFlush((void*)UART5RecQueue); //清空接收队列
        					QueueNData((void*)UART5RecQueue); //清空接收队列
        					
        					break;
        				}
					case UP_COMMU_DEV_232:
        				{
        					QueueFlush((void*)USART1RecQueue); //清空接收队列
        					QueueNData((void*)USART1RecQueue); //清空接收队列
        					
        					break;
        				}
					case UP_COMMU_DEV_GPRS:
        				{
        					QueueFlush((void*)USART3RecQueue_At); //清空接收队列
        					QueueNData((void*)USART3RecQueue_At); //清空接收队列
        					
        					break;
        				}					
        	}
		}
	else
		{
		
		switch(device)
			{
			case DOWN_COMM_DEV_MBUS:						//针对于Zigbee发送数据，每次100个字节，每次发送间隔333MS
					{
						QueueFlush((void*)USART2RecQueue); //清空接收队列
						QueueNData((void*)USART2RecQueue); //清空接收队列
						
						break;
					}

		    }
		}
#endif	
   // testmode = TRUE;

	err=PorttestGetHead(device,confirm,OS_TICKS_PER_SEC,TRUE,upORdown);
#if 0	
	OSTaskResume(PRIO_TASK_UART_SHELL);	
	OSTaskResume(PRIO_FLASH_MUTEX_PIP);	
	OSTaskResume(PRIO_TASK_MANAGE_DEBUG_INFO);	
	OSTaskResume(PRIO_TASK_GPRS_IPD);	
	OSTaskResume(PRIO_TASK_UP_SEND);	
	OSTaskResume(PRIO_TASK_UP_ANAL_FR);	
	OSTaskResume(PRIO_TASK_UP_REC_GPRS);	
	OSTaskResume(PRIO_TASK_UP_REC_ZIGBEE);	
	OSTaskResume(PRIO_TASK_UP_REC_RS485);	
	OSTaskResume(PRIO_TASK_UP_REC_RS232);	
	OSTaskResume(PRIO_TASK_GPRS_MANA);	
	OSTaskResume(PRIO_TASK_CLOCK);	
	OSTaskResume(PRIO_TASK_READ_ALL_CUR);	
	OSTaskResume(PRIO_TASK_LOG);
#endif	
#if 1
	if((err!=NO_ERR)&&(autotestRetryNum<2)) 
	{
      autotestRetryNum ++;
	  if((upORdown)&&(device==UP_COMMU_DEV_232))
	  	{
	  	 forbidsending1 =0;
	  	}
	  err = CMD_Port_test_Check(device,str,confirm,upORdown);
	}
	#if 0
	if((autotestRetryNum ==2)&& err!=0)
		{
		 if(autotestmode)
		 	{
     		 err= 0;
     		 autotestdisp3 = TRUE;
		 	}
		}
	#endif
	return err;
#endif	

}

/****************************************************************************************************
**	函 数  名 称:factory_test_method_Debug_RS232_0
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/

void factory_test_method_Debug_RS232_0(void)
{

	uint8 status;
	uint8 now_line =2;//显示当前光标反显的行
	bool OK_Flag = FALSE;
//	uint8 DeviceType;
	
//	uint8 tmpmid;
	uint8 err;
	
	uint8 len;
    char  Debugtestforport0Command[] = "Debug Test Command";	
    char  DebugtestforportOK[] = "Debug Test OK\n";	
	
    char  DebugtestforportErro[] = "Debug test Erro ";	
	uint8 DebugtestforportBuff[24] = {0};
   	   IRQ1testBuf_Counter = 0x00;
	  //ueueFlush((void*)UART5RecQueue); //清空接收队列
	   memset(IRQ1testBuf, 0x00, 32);
   display_white();
   ShowMenuTop();
   ShowMenuBottom(25);
   autotestRetryNum = 0;
  // UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
  err = CMD_Port_test_Check(UP_COMMU_DEV_232,Debugtestforport0Command,DebugtestforportOK,1);
   
  // ShowLineChar(3, 7, &GPRStestBuff[0], len);
	while(OK_Flag == FALSE)
	{
		//FreshMethodshow(now_line);
		//FactroytestSDCardData();
		display_white();
		ShowMenuTop();
		ShowMenuBottom(25);
		
//		UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
		 if(err!=NO_ERR) 
		 {		 
	    	 len = sprintf((char*)&DebugtestforportBuff[0], "%s", DebugtestforportErro);
			 ShowLineChar(3, 7, &DebugtestforportBuff[0], len);
		 }		 
		 else
		 {			 
        	 len = sprintf((char*)&DebugtestforportBuff[0], "%s", DebugtestforportOK);
    		 ShowLineChar(3, 7, &DebugtestforportBuff[0], len);
				 
		 }
		

	
		
		status = GetKeyStatuschaobiao();//得到键盘的状态
		switch(status)
		{
			case KEY_UP://向上键
			{
				if(now_line>2)
				{
					now_line--;
				}
				else
				{
					now_line = 4;
				}
				break;
			}
			case KEY_DOWN://向下键
			{
				if(now_line<4)
				{
					now_line++;
				}
				else
				{
					now_line = 2;
				}
				break;
			}			
			case KEY_LEFT://向左键	
			case KEY_RIGHT://向右键	
			{
				break;
			}
			case KEY_ENTER://回车键
			{
				#if 0
				OK_Flag = TRUE;
				if(now_line ==2)
				{
					DeviceType = HEAT_METER_TYPE;
				}
				else if(now_line ==3)/*时间通断面积法*/
				{
					DeviceType = TIME_ON_OFF_AREA_TYPE;
				}
				else if(now_line ==4)/*电子式热量表*/
				{
					DeviceType = HEATCOST_METER_TYPE;
				}
				else
				{
					DeviceType = HEAT_METER_TYPE;
				}
				/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
				SaveDeviceType(DeviceType);
				/*end:yangfei added 20140226 */
				now_line= 8;
				FreshMethodshow(now_line);
				#endif
				break;
				
			}
			case KEY_EXIT://返回键
			{
				KeyFuncIndex = 41;
				(*KeyTab[KeyFuncIndex].CurrentOperate)();
				OK_Flag = TRUE;
				break;
			}		
			default:	
				break;
		}
	}
	
}
/****************************************************************************************************
**	函 数  名 称:factory_test_method_ChaoBiao_RS232_1
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/

void factory_test_method_ChaoBiao_RS232_1(void)
	{
	
		uint8 status;
		uint8 now_line =2;//显示当前光标反显的行
		bool OK_Flag = FALSE;
	//	uint8 DeviceType;

	//	uint8 tmpmid;
		uint8 err;
		
		uint8 len;
		char  MBUStestforport0Command[] = "MBUS Test Command";	
		char  MBUStestforportOK[] = "MBUS Test OK\n";	
		
		char  MBUStestforportErro[] = "MBUS Test Erro ";	
		uint8 MBUStestforportBuff[24] = {0};
	   	   IRQ2testBuf_Counter = 0x00;
	  //ueueFlush((void*)UART5RecQueue); //清空接收队列
	   memset(IRQ2testBuf, 0x00, 32);
	   display_white();
	   ShowMenuTop();
	   ShowMenuBottom(25);
	   len = sprintf((char*)&MBUStestforportBuff[0], "%s", MBUStestforport0Command);
	   
	//   DuSend(DOWN_COMM_DEV_MBUS,(uint8 *)MBUStestforport0Command,len);
	
	autotestRetryNum = 0;
	err = CMD_Port_test_Check(DOWN_COMM_DEV_MBUS,MBUStestforport0Command,MBUStestforportOK,0);
	  // UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
//	  err = CMD_Port_test_Check(UP_COMMU_DEV_232,MBUStestforport0Command,MBUStestforportOK);
	   
	  // ShowLineChar(3, 7, &GPRStestBuff[0], len);
		while(OK_Flag == FALSE)
		{
			//FreshMethodshow(now_line);
			//FactroytestSDCardData();
			display_white();
			ShowMenuTop();
			ShowMenuBottom(25);
			
	//		UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
			 if(err!=NO_ERR) 
			 {		 
				 len = sprintf((char*)&MBUStestforportBuff[0], "%s", MBUStestforportErro);
				 ShowLineChar(3, 7, &MBUStestforportBuff[0], len);
			 }		 
			 else
			 {			 
				 len = sprintf((char*)&MBUStestforportBuff[0], "%s", MBUStestforportOK);
				 ShowLineChar(3, 7, &MBUStestforportBuff[0], len);
					 
			 }
			
	
		
			
			status = GetKeyStatuschaobiao();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
				{
					if(now_line>2)
					{
						now_line--;
					}
					else
					{
						now_line = 4;
					}
					break;
				}
				case KEY_DOWN://向下键
				{
					if(now_line<4)
					{
						now_line++;
					}
					else
					{
						now_line = 2;
					}
					break;
				}			
				case KEY_LEFT://向左键	
				case KEY_RIGHT://向右键 
				{
					break;
				}
				case KEY_ENTER://回车键
				{
				#if 0
					OK_Flag = TRUE;
					if(now_line ==2)
					{
						DeviceType = HEAT_METER_TYPE;
					}
					else if(now_line ==3)/*时间通断面积法*/
					{
						DeviceType = TIME_ON_OFF_AREA_TYPE;
					}
					else if(now_line ==4)/*电子式热量表*/
					{
						DeviceType = HEATCOST_METER_TYPE;
					}
					else
					{
						DeviceType = HEAT_METER_TYPE;
					}
					/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
					SaveDeviceType(DeviceType);
					/*end:yangfei added 20140226 */
					now_line= 8;
					FreshMethodshow(now_line);
				#endif
					break;
					
				}
				case KEY_EXIT://返回键
				{
					KeyFuncIndex = 42;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					OK_Flag = TRUE;
					break;
				}		
				default:	
					break;
			}
		}
		
	}

/****************************************************************************************************
**	函 数  名 称:factory_test_method_GPRS_TTL_2
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/

void factory_test_method_GPRS_TTL_2(void)
	{
	
		uint8 status;
		uint8 now_line =2;//显示当前光标反显的行
		bool OK_Flag = FALSE;
	//	uint8 DeviceType;
		
	//	uint8 tmpmid;
		uint8 err;
		
		uint8 len;
		char  GPRStestforport0Command[] = "GPRS Test Command";	
		char  GPRStestforportOK[] = "GPRS Test OK\n";	
		
		char  GPRStestforportErro[] = "GPRS Test Erro ";	
		uint8 GPRStestforportBuff[15] = {0};
	   	   IRQ3testBuf_Counter = 0x00;
	  //ueueFlush((void*)UART5RecQueue); //清空接收队列
	   memset(IRQ3testBuf, 0x00, 32);
	   display_white();
	   ShowMenuTop();
	   ShowMenuBottom(25);
	   
	   autotestRetryNum = 0;
	  // UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
	  err = CMD_Port_test_Check(UP_COMMU_DEV_GPRS,GPRStestforport0Command,GPRStestforportOK,1);
	   
	  // ShowLineChar(3, 7, &GPRStestBuff[0], len);
		while(OK_Flag == FALSE)
		{
			//FreshMethodshow(now_line);
			//FactroytestSDCardData();
			display_white();
			ShowMenuTop();
			ShowMenuBottom(25);
			
	//		UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
			 if(err!=NO_ERR) 
			 {		 
				 len = sprintf((char*)&GPRStestforportBuff[0], "%s", GPRStestforportErro);
				 ShowLineChar(3, 7, &GPRStestforportBuff[0], len);
			 }		 
			 else
			 {			 
				 len = sprintf((char*)&GPRStestforportBuff[0], "%s", GPRStestforportOK);
				 ShowLineChar(3, 7, &GPRStestforportBuff[0], len);
					 
			 }
			
	
		
			
			status = GetKeyStatuschaobiao();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
				{
					if(now_line>2)
					{
						now_line--;
					}
					else
					{
						now_line = 4;
					}
					break;
				}
				case KEY_DOWN://向下键
				{
					if(now_line<4)
					{
						now_line++;
					}
					else
					{
						now_line = 2;
					}
					break;
				}			
				case KEY_LEFT://向左键	
				case KEY_RIGHT://向右键 
				{
					break;
				}
				case KEY_ENTER://回车键
				{
				#if 0
					OK_Flag = TRUE;
					if(now_line ==2)
					{
						DeviceType = HEAT_METER_TYPE;
					}
					else if(now_line ==3)/*时间通断面积法*/
					{
						DeviceType = TIME_ON_OFF_AREA_TYPE;
					}
					else if(now_line ==4)/*电子式热量表*/
					{
						DeviceType = HEATCOST_METER_TYPE;
					}
					else
					{
						DeviceType = HEAT_METER_TYPE;
					}
					/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
					SaveDeviceType(DeviceType);
					/*end:yangfei added 20140226 */
					now_line= 8;
					FreshMethodshow(now_line);
				#endif
					break;
					
				}
				case KEY_EXIT://返回键
				{
					KeyFuncIndex = 43;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					OK_Flag = TRUE;
					break;
				}		
				default:	
					break;
			}
		}
		
	}

/****************************************************************************************************
**	函 数  名 称:factory_test_method_Commu_RS485_3
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/

void factory_test_method_Commu_Zigbee_3(void)
	{
	
		uint8 status;
		uint8 now_line =2;//显示当前光标反显的行
		bool OK_Flag = FALSE;
	//	uint8 DeviceType;
		
	//	uint8 tmpmid;
		uint8 err;
		
		uint8 len;
		char  Zigbeetestforport0Command[] = "Zigbee Test Command";	
		char  ZigbeetestforportOK[] = "Zigbee Test OK\n";	
		
		char  ZigbeetestforportErro[] = "Zigbee Test Erro ";	
		uint8 ZigbeetestforportBuff[15] = {0};
	   	   IRQ4testBuf_Counter = 0x00;
	  //ueueFlush((void*)UART5RecQueue); //清空接收队列
	   memset(IRQ4testBuf, 0x00, 32);
	   display_white();
	   ShowMenuTop();
	   ShowMenuBottom(25);
	   
	   autotestRetryNum = 0;
	  // UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
	  err = CMD_Port_test_Check(UP_COMMU_DEV_ZIGBEE,Zigbeetestforport0Command,ZigbeetestforportOK,1);
	   
	  // ShowLineChar(3, 7, &GPRStestBuff[0], len);
		while(OK_Flag == FALSE)
		{
			//FreshMethodshow(now_line);
			//FactroytestSDCardData();
			display_white();
			ShowMenuTop();
			ShowMenuBottom(25);
			
	//		UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
			 if(err!=NO_ERR) 
			 {		 
				 len = sprintf((char*)&ZigbeetestforportBuff[0], "%s", ZigbeetestforportErro);
				 ShowLineChar(3, 7, &ZigbeetestforportBuff[0], len);
			 }		 
			 else
			 {			 
				 len = sprintf((char*)&ZigbeetestforportBuff[0], "%s", ZigbeetestforportOK);
				 ShowLineChar(3, 7, &ZigbeetestforportBuff[0], len);
					 
			 }
			
	
		
			
			status = GetKeyStatuschaobiao();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
				{
					if(now_line>2)
					{
						now_line--;
					}
					else
					{
						now_line = 4;
					}
					break;
				}
				case KEY_DOWN://向下键
				{
					if(now_line<4)
					{
						now_line++;
					}
					else
					{
						now_line = 2;
					}
					break;
				}			
				case KEY_LEFT://向左键	
				case KEY_RIGHT://向右键 
				{
					break;
				}
				case KEY_ENTER://回车键
				{
				#if 0
					OK_Flag = TRUE;
					if(now_line ==2)
					{
						DeviceType = HEAT_METER_TYPE;
					}
					else if(now_line ==3)/*时间通断面积法*/
					{
						DeviceType = TIME_ON_OFF_AREA_TYPE;
					}
					else if(now_line ==4)/*电子式热量表*/
					{
						DeviceType = HEATCOST_METER_TYPE;
					}
					else
					{
						DeviceType = HEAT_METER_TYPE;
					}
					/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
					SaveDeviceType(DeviceType);
					/*end:yangfei added 20140226 */
					now_line= 8;
					FreshMethodshow(now_line);
				#endif
					break;
					
				}
				case KEY_EXIT://返回键
				{
					KeyFuncIndex = 44;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					OK_Flag = TRUE;
					break;
				}		
				default:	
					break;
			}
		}
		
	}

/****************************************************************************************************
**	函 数  名 称:factory_test_method_Commu_RS485_4
**	函 数  功 能:
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/

void factory_test_method_Commu_RS485_4(void)
	{
	
		uint8 status;
		uint8 now_line =2;//显示当前光标反显的行
		bool OK_Flag = FALSE;
	//	uint8 DeviceType;
		
	//	uint8 tmpmid;
		uint8 err;
		
		uint8 len;
		char  RS485testforport0Command[] = "RS485 Test Command";	
		char  RS485testforportOK[] = "RS485 Test OK\n";	
		
		char  RS485testforportErro[] = "RS485 Test Erro ";	
		uint8 RS485testforportBuff[15] = {0};
	   display_white();
	   ShowMenuTop();
	   ShowMenuBottom(25);
	   IRQ5testBuf_Counter = 0x00;
	  //ueueFlush((void*)UART5RecQueue); //清空接收队列
	   memset(IRQ5testBuf, 0x00, 32);
	//  QueueFlush((void*)UART5RecQueue);
	   
	autotestRetryNum = 0;
	  // UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
	  err = CMD_Port_test_Check(UP_COMMU_DEV_485,RS485testforport0Command,RS485testforportOK,1);
	 //  testmode= FALSE;
	  // ShowLineChar(3, 7, &GPRStestBuff[0], len);
		while(OK_Flag == FALSE)
		{
			//FreshMethodshow(now_line);
			//FactroytestSDCardData();
			display_white();
			ShowMenuTop();
			ShowMenuBottom(25);
			
	//		UpDevSend(UP_COMMU_DEV_232,DebugtestforportBuff,len);
			 if(err!=NO_ERR) 
			 {		 
				 len = sprintf((char*)&RS485testforportBuff[0], "%s", RS485testforportErro);
				 ShowLineChar(3, 7, &RS485testforportBuff[0], len);
			 }		 
			 else
			 {			 
				 len = sprintf((char*)&RS485testforportBuff[0], "%s", RS485testforportOK);
				 ShowLineChar(3, 7, &RS485testforportBuff[0], len);
					 
			 }
			
	
		
			
			status = GetKeyStatuschaobiao();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
				{
					if(now_line>2)
					{
						now_line--;
					}
					else
					{
						now_line = 4;
					}
					break;
				}
				case KEY_DOWN://向下键
				{
					if(now_line<4)
					{
						now_line++;
					}
					else
					{
						now_line = 2;
					}
					break;
				}			
				case KEY_LEFT://向左键	
				case KEY_RIGHT://向右键 
				{
					break;
				}
				case KEY_ENTER://回车键
				{
				#if 0
					OK_Flag = TRUE;
					if(now_line ==2)
					{
						DeviceType = HEAT_METER_TYPE;
					}
					else if(now_line ==3)/*时间通断面积法*/
					{
						DeviceType = TIME_ON_OFF_AREA_TYPE;
					}
					else if(now_line ==4)/*电子式热量表*/
					{
						DeviceType = HEATCOST_METER_TYPE;
					}
					else
					{
						DeviceType = HEAT_METER_TYPE;
					}
					/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
					SaveDeviceType(DeviceType);
					/*end:yangfei added 20140226 */
					now_line= 8;
					FreshMethodshow(now_line);
				#endif
					break;
					
				}
				case KEY_EXIT://返回键
				{
					KeyFuncIndex = 45;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					OK_Flag = TRUE;
					break;
				}		
				default:	
					break;
			}
		}
		
	}

uint8 factory_test_map(uint8 index)
{
	char Ats_AT[]="AT\r";
	char Ata_OK[]="\r\nOK\r\n";

    uint8 err=0;
	
	 char  sdtestwriterr[] = "SD write erro ";	 
	 char  timetesterr[] = "time test erro ";	 
	 char  timetestOK[] = "time test OK ";	 
	 char  sdtestreaderr[] = "SD read erro ";	 
	 char  sdtestOK[] = "SD test OK ";	 
	 char  GPRStesterr[] = "GPRS test erro ";	 
	 char  GPRStestOK[] = "GPRS test OK ";	 
	 char  Debugtestforport0Command[] = "Debug Test Command";	 
	 char  DebugtestforportOK[] = "Debug Test OK\n"; 
	 
	 char  DebugtestforportErro[] = "Debug test Erro ";  
	 
	 char	 MBUStestforport0Command[] = "MBUS Test Command";  
	 char	 MBUStestforportOK[] = "MBUS Test OK\n"; 
	 
	 char	 MBUStestforportErro[] = "MBUS Test Erro ";    
	 uint8 MBUStestforportBuff[24] = {0};
	 char  GPRStestforport0Command[] = "GPRS Test Command";  
	 char  GPRStestforportOK[] = "GPRS Test OK\n";	 
	 
	 char  GPRStestforportErro[] = "GPRS Test Erro ";	 
	 
	char Zigbeetestforport0Command[] = "Zigbee Test Command";  
	char ZigbeetestforportOK[] = "Zigbee Test OK\n"; 
	
	char ZigbeetestforportErro[] = "Zigbee Test Erro ";    
	
	  char	RS485testforport0Command[] = "RS485 Test Command";	  
	  char	RS485testforportOK[] = "RS485 Test OK\n";	  
	switch(index)
	{
		case 0:
		{
			err=FactroytestSDCardData();  
			break;
		}
		case 1:
		{
			
			err=Factory_test_Set_SystemTime();
			break;
		}			
		case 2:
		{
			err=CMD_AT_RP(Ats_AT,Ata_OK,NULL,OS_TICKS_PER_SEC,1,TRUE);
			break;
		}
		case 3:
		{
			
			err=CMD_Port_test_Check(UP_COMMU_DEV_232,Debugtestforport0Command,DebugtestforportOK,1);
			break;
		}
		case 4:
		{
			
			err =CMD_Port_test_Check(DOWN_COMM_DEV_MBUS,MBUStestforport0Command,MBUStestforportOK,0);
			break;
			
		}
		case 5:
		{
			err =CMD_Port_test_Check(UP_COMMU_DEV_GPRS,GPRStestforport0Command,GPRStestforportOK,1);
			break;
		}
		case 6:
		{
			
			err =CMD_Port_test_Check(UP_COMMU_DEV_ZIGBEE,Zigbeetestforport0Command,ZigbeetestforportOK,1);
		}
		case 7:
		{
			
			err = CMD_Port_test_Check(UP_COMMU_DEV_485,RS485testforport0Command,RS485testforportOK,1);
		}
		default:	
			break;
	}
    return err;
}
void factory_test_method_auto(void)
{ 
//   uint8 err; 
//   char Ats_AT[]="AT\r";
//   char Ata_OK[]="\r\nOK\r\n";
   autotestmode = TRUE;   
   uint8 len;
   uint8 testresult[8] ={0};
   
   uint8 status;   
   uint8 now_line =2;//显示当前光标反显的行
   bool OK_Flag = FALSE;
   uint8 testresultBuff[24] = {0};
    char  sdtestwriterr[] = "SD write erro ";	
    char  timetesterr[] = "time test erro ";	
    char  timetestOK[] = "time test OK ";	
    char  sdtestreaderr[] = "SD read erro ";	
    char  sdtestOK[] = "SD test OK ";	
    char  GPRStesterr[] = "GPRS test erro ";	
    char  GPRStestOK[] = "GPRS test OK ";	
    char  Debugtestforport0Command[] = "Debug Test Command";	
    char  DebugtestforportOK[] = "Debug Test OK\n";	
	
    char  DebugtestforportErro[] = "Debug test Erro ";	
	
    char	MBUStestforport0Command[] = "MBUS Test Command";  
    char	MBUStestforportOK[] = "MBUS Test OK\n"; 
    
    char	MBUStestforportErro[] = "MBUS Test Erro ";	  
    uint8 MBUStestforportBuff[24] = {0};
	char  GPRStestforport0Command[] = "GPRS Test Command";	
	char  GPRStestforportOK[] = "GPRS Test OK\n";	
	
	char  GPRStestforportErro[] = "GPRS Test Erro ";	
	
   char	Zigbeetestforport0Command[] = "Zigbee Test Command";  
   char	ZigbeetestforportOK[] = "Zigbee Test OK\n"; 
   
   char	ZigbeetestforportErro[] = "Zigbee Test Erro ";	  
   
	 char  RS485testforport0Command[] = "RS485 Test Command";	 
	 char  RS485testforportOK[] = "RS485 Test OK\n";	 
	 
	 char  RS485testforportErro[] = "RS485 Test Erro ";  
	//const char resultOKArr[8][24] = {sdtestOK[0],timetestOK[0],GPRStestOK[0],DebugtestforportOK[0],MBUStestforportOK[0],GPRStestforportOK[0],ZigbeetestforportOK[0],RS485testforportOK[0]};
	//const char resultERRArr[8][24] = {sdtestreaderr[0],timetesterr[0],GPRStesterr[0],DebugtestforportErro[0],MBUStestforportErro[0],GPRStestforportErro[0],ZigbeetestforportErro[0],RS485testforportErro[0]};
	const char resultOKArr[8][24] = {{"SD test OK "},{"time test OK "},{"GPRS test OK "},{"Debug Test OK"},{"MBUS Test OK"},{"GPRS Test OK"},{"Zigbee Test OK"},{"RS485 Test OK"}};
	const char resultOKArr3[8][24] = {{"SD test OK. "},{"time test OK. "},{"GPRS test OK. "},{"Debug Test OK."},{"MBUS Test OK."},{"GPRS Test OK."},{"Zigbee Test OK."},{"RS485 Test OK."}};
	const char resultERRArr[8][24] = {{"SD read erro "},{"time test erro"},{"GPRS test erro "},{"Debug test Erro "},{"MBUS Test Erro "},{"GPRS Test Erro "},{"Zigbee Test Erro "},{"RS485 Test Erro "}};

//char resultOKArr[][8]={"a","a","a","a","a","a","a","a"};
//	char resultERRArr[][8]={"b","b","b","b","b","b","b","b"};
	 uint8 RS485testforportBuff[15] = {0};
	if(autotestIsfromExitKey== 1)//防止自动测试返回键退出后再按上下左右键重复运行函数
	return;
	autotestIsfromExitKey= 0;
    autotestFirstIn ++;
	IRQ5testBuf_Counter = 0x00;
   //ueueFlush((void*)UART5RecQueue); //清空接收队列
	memset(IRQ5testBuf, 0x00, 32);
 	 IRQ4testBuf_Counter = 0x00;
 //ueueFlush((void*)UART5RecQueue); //清空接收队列
  memset(IRQ4testBuf, 0x00, 32);
   	   IRQ3testBuf_Counter = 0x00;
  //ueueFlush((void*)UART5RecQueue); //清空接收队列
   memset(IRQ3testBuf, 0x00, 32);
	  
		 IRQ2testBuf_Counter = 0x00;
	//ueueFlush((void*)UART5RecQueue); //清空接收队列
	 memset(IRQ2testBuf, 0x00, 32);
   	   IRQ1testBuf_Counter = 0x00;
	  //ueueFlush((void*)UART5RecQueue); //清空接收队列
	   memset(IRQ1testBuf, 0x00, 32);
	  display_white();
	  ShowMenuTop();
	  ShowMenuBottom(25);
	  
   ShowLineChar_HZ(2,2,&MenuHZTestingPleaseWaite[0],9);//
   
#if 1	
	   OSTaskSuspend(PRIO_TASK_UART_SHELL);    
	   OSTaskSuspend(PRIO_FLASH_MUTEX_PIP);    
	   OSTaskSuspend(PRIO_TASK_MANAGE_DEBUG_INFO); 
	   OSTaskSuspend(PRIO_TASK_GPRS_IPD);  
	   //Test_SuspendGprsRecTask();
	   OSTaskSuspend(PRIO_TASK_UP_SEND);   
	   OSTaskSuspend(PRIO_TASK_UP_ANAL_FR);    
	   OSTaskSuspend(PRIO_TASK_UP_REC_GPRS);   
	   OSTaskSuspend(PRIO_TASK_UP_REC_ZIGBEE); 
	   OSTaskSuspend(PRIO_TASK_UP_REC_RS485);  
	   OSTaskSuspend(PRIO_TASK_UP_REC_RS232);  
	   OSTaskSuspend(PRIO_TASK_GPRS_MANA); 
	   OSTaskSuspend(PRIO_TASK_CLOCK); 
	   OSTaskSuspend(PRIO_TASK_READ_ALL_CUR);  
	   OSTaskSuspend(PRIO_TASK_LOG);   
#endif	
   autotestRetryNum = 0;
      
	  OSTimeDly(OS_TICKS_PER_SEC/2);
	  
	  display_white();
	  ShowMenuTop();
	  ShowMenuBottom(25);
       for (int i =0; i<8 ;i++)
       {
         testresult[i] =factory_test_map(i);
         
		 if(testresult[i])
			   {
			   
			   len = sprintf((char*)&testresultBuff[0], "%s", (char*)&resultERRArr[i][0]);
			   ShowLineChar(i+1, 2, &testresultBuff[0], len);
			   
			   }
			 else
			   {
				if(autotestdisp3)
					{					
					len = sprintf((char*)&testresultBuff[0], "%s", (char*)&resultOKArr3[i]);
					ShowLineChar(i+1, 2, &testresultBuff[0], len);
					autotestdisp3 =FALSE;
					}
				else
					{
        				len = sprintf((char*)&testresultBuff[0], "%s", (char*)&resultOKArr[i]);
        				ShowLineChar(i+1, 2, &testresultBuff[0], len);
					}
			   }
				 status = GetKeyStatuschaobiao();//得到键盘的状态
				 switch(status)
				 {
					 case KEY_UP://向上键
					 {
						 break;
					 }
					 case KEY_DOWN://向下键
					 {
						 break;
					 }			 
					 case KEY_LEFT://向左键  
					 case KEY_RIGHT://向右键 
					 {
						 break;
					 }
					 case KEY_ENTER://回车键
					 {
						 break;
						 
					 }
					 case KEY_EXIT://返回键
					 {
					 	#if 0
						 display_white();
						 ShowMenuTop();
						 ShowMenuBottom(25);
						 KeyFuncIndex = 37;
						 (*KeyTab[KeyFuncIndex].CurrentOperate)();
						 #endif
						// OK_Flag = TRUE;
						autotestIsfromExitKey = 1;
						 autotestmode = FALSE;
						 return;
					 }		 
					 default:	 
						 break;
				 }


	   }
#if 0	   
   testresult[0] = FactroytestSDCardData();  
   testresult[1] =0;//Factory_test_Set_SystemTime();
   testresult[2] =CMD_AT_RP(Ats_AT,Ata_OK,NULL,OS_TICKS_PER_SEC,1,TRUE);;
   testresult[3] =CMD_Port_test_Check(UP_COMMU_DEV_232,Debugtestforport0Command,DebugtestforportOK,1);
   testresult[4] =CMD_Port_test_Check(DOWN_COMM_DEV_MBUS,MBUStestforport0Command,MBUStestforportOK,0);
   testresult[5] =CMD_Port_test_Check(UP_COMMU_DEV_GPRS,GPRStestforport0Command,GPRStestforportOK,1);
   testresult[6] =CMD_Port_test_Check(UP_COMMU_DEV_ZIGBEE,Zigbeetestforport0Command,ZigbeetestforportOK,1);
   testresult[7] = CMD_Port_test_Check(UP_COMMU_DEV_485,RS485testforport0Command,RS485testforportOK,1);
   display_white();
   ShowMenuTop();
   ShowMenuBottom(25);

   for(int i =0; i<8; i++)
   	{
      if(testresult[i])
      	{
      	
		len = sprintf((char*)&testresultBuff[0], "%s", (char*)&resultERRArr[i][0]);
		ShowLineChar(i+1, 2, &testresultBuff[0], len);
		
		OSTimeDly(OS_TICKS_PER_SEC/2);
      	}
	  else
	  	{
         
		 len = sprintf((char*)&testresultBuff[0], "%s", (char*)&resultOKArr[i]);
		 ShowLineChar(i+1, 2, &testresultBuff[0], len);		 
		 OSTimeDly(OS_TICKS_PER_SEC/2);
	    }
	  
    }
  #endif 
   autotestmode = FALSE;   
  
  forbidsending1 = 0;
  while(OK_Flag == FALSE)
   {
			
	
		
			
			status = GetKeyStatuschaobiao();//得到键盘的状态
			switch(status)
			{
				case KEY_UP://向上键
				{
					if(now_line>2)
					{
						now_line--;
					}
					else
					{
						now_line = 4;
					}
					break;
				}
				case KEY_DOWN://向下键
				{
					if(now_line<4)
					{
						now_line++;
					}
					else
					{
						now_line = 2;
					}
					break;
				}			
				case KEY_LEFT://向左键	
				case KEY_RIGHT://向右键 
				{
					break;
				}
				case KEY_ENTER://回车键
				{
				#if 0
					OK_Flag = TRUE;
					if(now_line ==2)
					{
						DeviceType = HEAT_METER_TYPE;
					}
					else if(now_line ==3)/*时间通断面积法*/
					{
						DeviceType = TIME_ON_OFF_AREA_TYPE;
					}
					else if(now_line ==4)/*电子式热量表*/
					{
						DeviceType = HEATCOST_METER_TYPE;
					}
					else
					{
						DeviceType = HEAT_METER_TYPE;
					}
					/*begin:yangfei added 20140226 for 增加sd卡保存数据*/
					SaveDeviceType(DeviceType);
					/*end:yangfei added 20140226 */
					now_line= 8;
					FreshMethodshow(now_line);
				#endif
					break;
					
				}
				case KEY_EXIT://返回键
				{
					KeyFuncIndex = 37;
					(*KeyTab[KeyFuncIndex].CurrentOperate)();
					OK_Flag = TRUE;
					autotestmode = FALSE;
					
					forbidsending1 = 0;
					break;
				}		
				default:	
					break;
			}
		}
   	
}

/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/

