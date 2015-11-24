#ifndef        	 _LCD_MENU_H
#define		 _LCD_MENU_H

#define		KEY_UP		0x01
#define 	KEY_DOWN	0x02
#define		KEY_LEFT	0x03
#define		KEY_RIGHT	0x04
#define		KEY_ENTER	0x05
#define		KEY_EXIT	0x06
#define 	NO_KEY		0xff

//#ifndef IN_LCD_MENU
void LCD_Initial(void);//液晶初始化，对函数lcd_initial()中的对比度设置进行补充
void RegisterMenu(void);//注册界面菜单
//void IniPara(void);//初始化参数  暂时放在这个位置

//1级
void MainMenu(void);//液晶主菜单							==0

void Action_LcdMenu11(void); // "1.参数设置与查看"			==1
void Action_LcdMenu12(void); // "2.抄表状态查询				==2
void Action_LcdMenu13(void); // "3.集中器管理与维护"		==3
void Action_LcdMenu14(void);
void Action_LcdMenu15(void);
void Action_LcdMenu16(void);//muxiaoqing test


//2级
void Action_LcdMenu11_21(void); //"1.通信通道设置"			==4
void Action_LcdMenu11_22(void); //"2.终端时间设置"			==5
void Action_LcdMenu11_23(void); //"3.终端编号设置"			==6
void Action_LcdMenu11_24(void); //"4.抄表时间点设置"		==7
void Action_LcdMenu11_25(void);	//"5.GPRS参数设置"		    ==28
void Set_IPAndPort(void);	  	       //"IP及端口号设置"			==29
void Action_LcdMenu11_26(void);
void Set_ReplenishPara(void);


//"1.当前抄表时间节点抄表状态统计显示"
void Action_LcdMenu12_21(void);			//					==8
void Action_LcdMenu12_22(void); 			// "1.查看当前时间节点抄表状态"
void Action_LcdMenu12_23(void);
void Action_LcdMenu12_24(void);

		//此处预留8-12给显示抄表状态用

void Action_LcdMenu13_21(void);	//"1.重启终端"				==13
void Action_LcdMenu13_22(void); //"2.终端版本信息"			==14
void Action_LcdMenu13_23(void); //"3.液晶对比度设置"		==15

//muxiaoqing
#if 0
void Action_LcdMenu16_21(void);		//"SD 卡数据写入和读出对比测试"
void Action_LcdMenu16_22(void);		//"时钟芯片的写入和读出对比测试"
void Action_LcdMenu16_23(void);		//"GPRS 指令测试"
void Action_LcdMenu16_24(void);		//"调试串口命令测试"
void Action_LcdMenu16_25(void);		//"抄表串口1命令测试"
void Action_LcdMenu16_26(void);		//"GPRS通讯命令测试"
void Action_LcdMenu16_27(void);		//"RS485 通讯1命令测试"
void Action_LcdMenu16_28(void);		//"RS485 通讯2命令测试"
#else
void Action_LcdMenu16_21(void);		//"手动测试"
void Action_LcdMenu16_22(void);		//"自动测试"

#endif

//3级
void ZigbeeSet(void);				//设置zigbee主站地址	==16
void Set_SystemTime(void);			//设置终端时间函数		==17
void Set_SystemAddr(void);			//终端编号设置函数		==18
void Set_ReadMeterTime(void);		//抄表时间点设置函数	==19





void ReadMeterStateShow(void);		//抄表状态信息详情函数	==20
void ReadMeterNow(void);//立即抄表 ==21
void ShowMeterFail(void);
void ShowValveFail(void);
void InquiryMeterFailInfo(void);
void InquiryValveFailInfo(void);
void SlectReadMeterChannel(void);
//muxiaoqing test menu manual
void Action_LcdMenu16_21_31(void); 	//"SD 卡数据写入和读出对比测试"
void Action_LcdMenu16_21_32(void); 	//"时钟芯片的写入和读出对比测试"
void Action_LcdMenu16_21_33(void); 	//"GPRS 指令测试"
void Action_LcdMenu16_21_34(void); 	//"调试串口命令测试"
void Action_LcdMenu16_21_35(void); 	//"抄表串口1命令测试"
void Action_LcdMenu16_21_36(void); 	//"GPRS通讯命令测试"
void Action_LcdMenu16_21_37(void); 	//"RS485 通讯1命令测试"
void Action_LcdMenu16_21_38(void); 	//"RS485 通讯2命令测试"

#ifdef DEMO_APP
void ReadMeter_Demo(void);//
void ReadMeterStateShow_Demo(void);		//抄表状态信息详情函数	==20
void Set_MeterParam_1(void);		//
void Set_MeterParam_2(void);		//
void Set_MeterParam_3(void);		//

#endif

		//此处预留20-24给显示抄表状态用

void ResetSystem(void);  			//重启终端，确认或者否认==25
void VerInfoShow(void);				//显示 终端版本信息		==26
void LCDContrastSet(void);			//显示 液晶对比度		==27

void KeyCheck(void);
void MenuTest(void);
void Meter_RotateDisplay(void);
void RotateDisplay(void);

void Chose_metric_method(void);
void Chose_time_data(void);
void Display_Data_Menu(void);
uint8  HexToAscii_1(uint16 Hex_bcd_code,uint8* AscBuf);
void factory_test_method_sd(void);//muxiaoqing test
void factory_test_method_time(void);
void factory_test_method_GPRS(void);
void factory_test_method_Debug_RS232_0(void);
void factory_test_method_ChaoBiao_RS232_1(void);
void factory_test_method_GPRS_TTL_2(void);
void factory_test_method_Commu_Zigbee_3(void);
void factory_test_method_Commu_RS485_4(void);
void factory_test_method_auto(void);
void SelectAPNSet(void);
void SelectIPSet(void);
void APNSet(void);





//本文件自用函数
uint8 ConverHexToASCIP_15(uint8* pTemp1,uint8* pTemp2);
uint8 Hex_BcdToAscii_5(uint16 Hex_bcd_code,uint8* AscBuf);

typedef	struct	SelectInfoStruct
{
	uint8 startline;
	uint8 startcolumn;
	uint8 columnnum;
}SelectInfo;

#pragma pack(1)

typedef struct DisplayDataStruct
{
	uint8   user_id;/*用户编号*/
	uint16	address;  /*设备地址*/
	uint16	area;/*用户面积*/
	uint32  open_time;/*(BCD码)开阀时间，后2位为小数位。单位:h*/
	uint16  room_temperature;/*(BCD码)房间温度，后2位为小数位。单位:摄氏度*/
	uint16  forward_temperature;/*(BCD码)进水温度，后2位为小数位。单位:摄氏度*/
	uint16  return_temperature;/*(BCD码)回水温度，后2位为小数位。单位:摄氏度*/
	uint32  proportion_energy;/*(BCD码)当前时间段内的能量分摊值*/
	uint32  total_energy;/*(BCD码)用户的能量累计值,后2位为小数位。单位:kwh*/
}DisplayData;


typedef struct SDSaveDataStruct
{	
	uint8   data_len;
	uint8   user_id;/*用户编号*/
	uint16  address;  /*设备地址*/
	uint16  area;/*用户面积*/
	uint8   data_valid;/*数据是否有效  有效为0xA5，无效为0x00*/
	uint32  total_energy;/*(BCD码)用户的能量累计值,后2位为小数位。单位:kwh*/
	uint32  open_time;/*(BCD码)开阀时间，后2位为小数位。单位:h*/
	uint32  proportion_energy;/*(BCD码)当前时间段内的能量分摊值*/
	uint8   open_percent;/*当前时间段内的阀门开启比例(HEX码 0-100) */
	uint16  forward_temperature;/*(BCD码)进水温度，后2位为小数位。单位:摄氏度*/
	uint16  return_temperature;/*(BCD码)回水温度，后2位为小数位。单位:摄氏度*/
	uint16  room_temperature;/*(BCD码)房间温度，后2位为小数位。单位:摄氏度*/
	uint8   state;/*bit7 代表无线故障，1故障 0正常 
				      bit6 代表欠费状态 1欠费 0正常  
				     bit5 代表充值状态 1充值 0正常  
				     bit4 代表开关机状态，1开机，0关机; 
				     bit3 代表锁定 0不锁，1锁定;
				     bit2 代表阀门堵转故障，1故障，0正常; 
				     bit1 代表NTC故障        1故障，0正常;
				     bit0 代表阀门状态，1阀开，0阀关*/	
}SDSave_Data;

typedef struct CheckDataStruct
{
	uint8 user_id;//用户id
	uint8 read_num;//抄表点
	uint8 time_path[17];
}checkData;

#pragma pack()

typedef struct	KeyStateStruct
{
	uint8 KeyStateIndex;//当前索引
	uint8 KeyDnState;//按向下键时转向的索引
	uint8 KeyUpState;//按向上键时转向的索引
	uint8 KeyLeftState;//按向左键时转向的索引
	uint8 KeyRightState;//按向右键时转向的索引
	uint8 KeyCrState;//按确认键时转向的索引
	uint8 KeyBackState;//按取消键时转向的索引
	void  (*CurrentOperate)(void);//当前索引执行的动作
}KeyState;

//void *pMainMenu;
//pMainMenu  = &MainMenu;
#define	KEYBORD_MENE_SIZE	60 //muxiaoqing 50->60

extern KeyState KeyTab[KEYBORD_MENE_SIZE];
//#endif

#endif
