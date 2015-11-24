
#ifndef _READMETERDATA_H
#define _READMETERDATA_H

/******************************************************************/
#define TIMING_ALL		1
#define MSG_18			2
#define MSG_1C			3
#define MSG_1D			4
#define ISSUE_HEATVALUE  5  //通断时间面积法中，向用户发送上位机发来的热量分摊值。
/******************************************************************/



#define FORALL	0x0A	//群发
#define FORONE	0x0B	//单发

extern void* CmdMsgQ[10];               	//定义消息指针数组, 最多存储10个消息
extern OS_EVENT *CMD_Q;


#pragma pack(1)

typedef struct{
	uint8  MultiFlag;
	uint8  SendPackIndex;
	uint16 StartMeterSn;
	uint16 SendMeterNums;
	
	}MultiFrameMsg_0E;
	
typedef struct{
	uint8  MultiFlag;
	uint32 SendPackIndex;
	uint32 StartReadAddr;
	uint32 SendByteNums;
	uint32 EndReadAddr;
	
	}MultiFrameMsg_1A;
	
typedef struct{
	uint8  MultiFlag;
	uint32 SendPackIndex;
	uint32 StartReadAddr;
	uint32 SendByteNums;
	uint32 EndReadAddr;
	
	}MultiFrameMsg_1E;
#if 0
typedef struct{
	uint8  MultiFlag;
	uint8  SendPackIndex;
	uint16 StartMeterSn;
	uint16 SendMeterNums;
	
	}MultiFrameMsg_22;
#else
typedef struct{
	uint8  MultiFlag;
	uint8  SendPackIndex;
	uint8  time[6];/*存储时间*/
	uint8  device_type;/*设备类型*/
	uint8 SendMeterNums;
	
	uint16 StartMeterSn;
	}MultiFrameMsg_22;

typedef struct
	{
	uint8  MultiFlag;
	uint8  SendPackIndex;
	uint8  time[6];/*存储时间*/
	uint8  device_type;/*设备类型*/
	uint8 SendMeterNums;
	
	uint16 StartMeterSn;
	}MultiFrameMsg_92;


#endif
typedef struct{
	uint8  MultiFlag;
	uint8  SendPackIndex;
	uint8  time[6];/*存储时间*/
	uint8  device_type;/*设备类型*/
	uint8 SendMeterNums;
	
	uint16 StartMeterSn;
	}MultiFrameMsg_40;
/*begin:yangfei added 2014-02-24 for support 时间通断面积法热分配表法*/
typedef struct 
{
	uint8 have_next_flag; /*后续包标志,0表示无后续包；1表示有后续包;0x10表示异常回应*/
	uint8 packet_serial;/*包序号*/
	uint8 time[6];/*存储时间*/
	uint8 device_type;/*设备类型*/
	uint8 userdata_number;/*用户数据个数*/
}HistoryDataHead;

typedef struct heatcost_meter /*热分配表用户数据9B*/
{
	uint16 address;/*仪表地址*/
	uint8  length;/*仪表数据长度*/
	uint8 currut_cost[3];/*当前消耗值*/
	uint8 front_temperature;/*(BCD码)前面板温度,单位:摄氏度*/
	uint8 back_temperature;/*(BCD码)后面板温度,单位:摄氏度*/
	uint8 error_code;/*(HEX码)错误告警代码*/
}HeatCost;

typedef struct time_on_off_area/*时间通断面积法用户数据*/
{
	uint8 user_id;/*用户编号*/
	uint16 address;/*设备地址*/
	uint16 area;/*用户面积*/
	uint8  data_valid;/*数据是否有效  有效为0xA5，无效为0x00*/
	uint32 total_energy;/*(BCD码)用户的能量累计值,后2位为小数位。单位:kwh*/
	uint32 open_time;/*(BCD码)开阀时间，后2位为小数位。单位:h*/
	uint32 proportion_energy;/*(BCD码)当前时间段内的能量分摊值*/
	uint8  open_percent;/*当前时间段内的阀门开启比例(HEX码 0-100) */
	uint16 forward_temperature;/*(BCD码)进水温度，后2位为小数位。单位:摄氏度*/
	uint16 return_temperature;/*(BCD码)回水温度，后2位为小数位。单位:摄氏度*/
	uint16 room_temperature;/*(BCD码)房间温度，后2位为小数位。单位:摄氏度*/
	uint8  state;/*bit7 代表无线故障，1故障 0正常 
                              bit6 代表欠费状态 1欠费 0正常  
                             bit5 代表充值状态 1充值 0正常  
                             bit4 代表开关机状态，1开机，0关机; 
                             bit3 代表锁定 0不锁，1锁定;
                             bit2 代表阀门堵转故障，1故障，0正常; 
                             bit1 代表NTC故障        1故障，0正常;
                             bit0 代表阀门状态，1阀开，0阀关*/	
}TimeOnOffArea;
/*end:yangfei added 2014-02-24 for support 时间通断面积法热分配表法*/

typedef struct {  //读集中器参数结构体。
	uint8 u8IP[4];
	uint16 u16Port;
	uint8 u8Address[6];//
	uint8 u8HostAddress[6];// 主站地址
	uint16 u16SoftVer;/*例如：0x0232代表2.32*/
	uint16 u16HardwareVer; /*例如：0x0232代表2.32*/
	//uint8  u8Reserved[3];  /*保留四字节*/

	uint8 Method;// 超声波热计量表20H;电子式热分配表A0H;时间通断面积法B0H;
	uint8 DataSource;// （0X0A 上位机，0X0B本地抄表）
	uint16 Period;// 分摊周期（分钟）
	uint8 u8LogReportTime; //0x00 :打开 ，0x01:不打开
	uint8 u8LogOpenType; // 0X00: 打开 ，0X01 ：不打开
	uint8 u8LogCompressType; // 0X00:压缩，0X01：不压缩
 
}StruJZQInit;


typedef struct
{

	uint8 u8backpackFlag;
	uint8 u8NumPack;
	uint8 u8DeviceType;
	uint8 u8UserNum;	
}StruZLTB;



typedef struct{
	uint8 Method;// 超声波热计量表20H;电子式热分配表A0H;时间通断面积法B0H;
	uint8 DataSource;// （0X0A 上位机，0X0B本地抄表）
	uint16 Period;// 分摊周期（分钟）
	uint8 u8LogReportTime; //0x00 :打开 ，0x01:不打开
	uint8 u8LogOpenType; // 0X00: 打开 ，0X01 ：不打开
	uint8 u8LogCompressType; // 0X00:压缩，0X01：不压缩
	uint8 u8IPAddr0;	//0是低位IP  3是高位IP 16进制下发
	uint8 u8IPAddr1;
	uint8 u8IPAddr2;
	uint8 u8IPAddr3;
	uint8 u8HostPortHigh;  // 主站端口号高8位
	uint8 u8HostPortLow;   // 主站端口号 低8位


         //以备后续添加
}StruCSXF;   //参数下发结构体





typedef struct
{
	uint8 ForAllOrOne;
	uint8 Device_type;
	uint8 Channel;
	uint8 ProtocolVer;
	uint8 ControlCode;
	uint8 Lenth;
}FrameMsg_42;	
#pragma pack()

extern uint8 test_proMsg_04(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_06( _ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_0E(_ProtocolType &_ProData, uint8 *ControlSave);
extern uint8 DEAL_ProcessMsg_18(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_18_ForOne(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_18_ForAll(_ProtocolType &_ProData);
extern void DEAL_MSG18_ForAll(uint8 *pData);
extern uint8 DEAL_ProcessMsg_1A(_ProtocolType &_ProData, uint8 *ControlSave);
extern uint8 DEAL_ProcessMsg_1C(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_1C_ForOne(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_1C_ForAll(_ProtocolType &_ProData);
extern void DEAL_MSG1C_ForAll(uint8 *pData);
extern uint8 DEAL_ProcessMsg_1E(_ProtocolType &_ProData, uint8 *ControlSave);
extern uint8 DEAL_ProcessMsg_22(_ProtocolType &_ProData, uint8 *ControlSave);
extern uint8 DEAL_ProcessMsg_3C(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_3E(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_40(_ProtocolType &_ProData, uint8 *ControlSave);
extern uint8 DEAL_ProcessMsg_42(_ProtocolType &_ProData);





#endif
