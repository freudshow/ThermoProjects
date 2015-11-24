/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: meter.h
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年9月10日 
**  描        述: 热计量表协议解析
**	修 改  记 录:   	
*****************************************************************************************************/
#ifndef				_METER_H
#define				_METER_H

#include "StoreManage.h"

#define  MaxRec_num      			5
#define METER_ADDR_LEN				7				//热计量表地址长度
#define	DELU_FRAME_START			0x68

extern OS_EVENT *UpAskMeterSem;
extern OS_EVENT *METERChangeChannelSem;

extern uint8 gCurrent_Channel;

extern uint16 gu16ValveIDRecord[METER_NUM_MAX];


#pragma pack(1)

typedef struct {
    uint8 	Channel;  				//下行MBUS通信的 通道
    uint32  Bps;     				//下行通信波特率
    uint16 	OutTime; 				//等待超时时间  0表示不指定，按底层的缺省超时时间
    uint8 	RpTimes; 				//失败重试次数，0表示不重试
	} DownCommType;

typedef struct{
	uint8   PreSmybolNum;			/*前导字符个数*/
	uint8   MeterType;				/*仪表类型*/
	uint8 	MeterAddr[7];			/*热计量表地址*/
	uint8 	ControlCode;			/*控制码*/
	uint8 	Length;					/*数据域长度*/
	uint16	DataIdentifier;			/*数据标识符*/
	uint8 	SER;					/*序列号*/
	/*begin:yangfei added 2013-09-18 for 真兰热表数据太多*/
	#if 0
	uint8   DataBuf[METER_FRAME_LEN_MAX];
    #else
    uint8   DataBuf[METER_FRAME_LEN_MAX*2];
    #endif
	}DELU_Protocol;


typedef struct
{
	uint32 ValveOpenTime;				//阀开时间，BCD码，最后2位为小数位，单位小时。
	uint16 WaterInTemp;					//进水温度，BCD码，2位小数，单位℃。
	uint16 WaterOutTemp;				//回水温度，BCD码，2位小数，单位℃。
	uint16 RoomTemp;					//房间内温度，BCD码，2位小数，单位℃。
	uint8  RoomSetTemp;					//房间设定温度。
	uint8  RoomCompensateTemp;			//室内补偿温度。
	uint8  Bit_OnOffState		:1;		//位域1,开关机状态，1-开机，0-关机。
	uint8  Bit_PanelLockFlag	:1;		//位域2，面板锁定状态，0-未锁定，1-锁定。
	uint8  Bit_ValveState		:1;		//位域3，阀门状态，0-关阀，1-开阀。
	uint8  Bit_LockOpenFlag		:1;		//位域4，负载强制开标志，0-正常状态，1-强制开。
	uint8  Bit_LockCloseFlag	:1;		//位域5，负载强制关标志，0-正常状态，1-强制关。
	uint8  Bit_WirelessState	:1;		//位域6，无线故障标志，0-无线正常，1-无线故障。
	uint8  Bit_RechargeFlag		:1;		//位域7，充值标志，0-未充值，1-充值。
	uint8  Bit_ArrearageFlag	:1;		//位域8，欠费标志，0-未欠费，1-欠费。
	
}Valve_Format;  //共占13字节。

//通断时间面积法上位机下发分摊值格式。
typedef struct  
{
	uint16	MeterID;//计量点号。
	uint32	ApportionValueThis;  //本阀控本次分摊值,BCD码，2位小数，单位kWh。
	uint8   ThisUnit;  //本次分摊热量单位。
	uint32	ApportionValueSum;  //本阀控分摊总值,BCD码，2位小数，单位kWh。。
	uint8   SumUint;	//总分摊热量单位。
}HeatValue_Format;



/*begin:yangfei added 2013-03-18 for meter data format standardized*/
typedef struct{
	uint32   DailyHeat;		                /*结算日热量*/	
    uint8     DailyHeatUnit;
	uint32   CurrentHeat;				   /*当前热量*/
    uint8     CurrentHeatUnit;
	uint32 	 HeatPower;			      /*热功率*/
    uint8     HeatPowerUnit;
	uint32 	 Flow;			                 /*流量流速*/
    uint8 	 FlowUnit;
	uint32 	AccumulateFlow;			/*累积流量*/
    uint8 	AccumulateFlowUnit;	
	uint8	WaterInTemp[3];		        /*供水温度*/	
	uint8 	WaterOutTemp[3];				/*回水温度*/
    uint8 	AccumulateWorkTime[3];	/*累计工作时间*/
	uint8	RealTime[7];		                /*实时时间*/	
	uint16 	ST;					                    /*状态ST*/  
	}CJ188_Format;

// 震宇时间无年中的20
typedef struct{
   uint32	DailyHeat;					   /*结算日热量*/  
   uint8	 DailyHeatUnit;
   uint32	CurrentHeat;				  /*当前热量*/
   uint8	 CurrentHeatUnit;
   uint32	HeatPower;				 /*热功率*/
   uint8	 HeatPowerUnit;
   uint32	Flow;							/*流量流速*/
   uint8	FlowUnit;
   uint32  AccumulateFlow;		   /*累积流量*/
   uint8   AccumulateFlowUnit; 
   uint8   WaterInTemp[3];			   /*供水温度*/    
   uint8   WaterOutTemp[3]; 			   /*回水温度*/
   uint8   AccumulateWorkTime[3];  /*累计工作时间*/
   uint8   RealTime[7]; 					   /*实时时间*/    
   uint16  ST;									   /*状态ST*/  
   }ZHENYU2_Format;

//begin: added by zjjin.
typedef struct{
	uint32	MeterID;
	uint8	Constant08;
	uint32  CurrentCool;		     /*当前冷量*/	
    uint8   CurrentCoolUnit;
	uint32  CurrentHeat;			/*当前热量*/
    uint8   CurrentHeatUnit;
	uint32 	 Flow;			         /*流量流速*/
    uint8 	 FlowUnit;
	uint32 	AccumulateFlow;			/*累积流量*/
    uint8 	AccumulateFlowUnit;	
	uint8	WaterInTemp[3];		        /*供水温度*/	
	uint8 	WaterOutTemp[3];			/*回水温度*/
    uint8 	AccumulateWorkTime[3];	/*累计工作时间*/
	uint8	RealTime[7];		                /*实时时间*/	
	uint16 	ST;					                    /*状态ST*/  
	}YITONGDA_Format;

//end: added by zjjin.

// 林晓彬添加结构体，用于将积成热量表的结算日热量，由IE754格式转换为BCD码
typedef union
{
    u8   gp22_u8[4];
    float gp22_float;
}To_Float;

typedef struct{
    
	//float   DailyHeat;		/*结算日热量积成热表的结算日热量是IEEE754协议*/
	To_Float  DailyHeat;		/*结算日热量积成热表的结算日热量是IEEE754协议*/
	uint8     DailyHeatUnit;	 //单位 
	uint32   CurrentHeat;				/*当前热量*/
	uint8     CurrentHeatUnit;   
	uint32 	 HeatPower;			/*热功率*/
	uint8     HeatPowerUnit;
   	uint32 	 Flow;			/*流量*/
	uint8 	 FlowUnit;   
	uint32 	AccumulateFlow;					/*累积流量*/
	uint8 	AccumulateFlowUnit;	
	uint8	WaterInTemp[3];		/*供水温度*/	
	uint8 	WaterOutTemp[3];					        /*回水温度*/
    uint8 	AccumulateWorkTime[3];					/*累计工作时间*/
	uint8	RealTime[7];		/*实时时间积成的时间是 年 月 日 时分秒，与CJ188恰好相反*/	
	uint8 	ST;					        /*状态ST*/  
	}JICHENG_Format;


typedef struct{
     
  	
	uint32   BiaoHao;        //表号
	uint8     BiaoShi;		 //标识

	uint32   CurrentHeat;				   /*当前热量*/
	uint8     CurrentHeatUnit; 
 	
	uint32   DailyHeat;		                /*结算日热量 冷量*/
	uint8     DailyHeatUnit;	
    

    //uint8     HeatPowerUnit;
	//uint32 	 HeatPower;			      /*热功率*/

    
	uint32 	 Flow;			                 /*流量流速*/
	uint8 	 FlowUnit;

    	
	uint32 	AccumulateFlow;			/*累积流量*/
	uint8 	AccumulateFlowUnit;

	uint8	WaterInTemp[3];		        /*供水温度*/	
	uint8 	WaterOutTemp[3];				/*回水温度*/
    uint8 	AccumulateWorkTime[3];	/*累计工作时间*/
	uint8	RealTime[7];		                /*实时时间*/	
	uint16 	ST;	
	}JingWeiFE_Format;


typedef struct{

 	uint8     DailyHeatUnit;
	uint32   DailyHeat;		                /*结算日热量*/	
    
	uint8     CurrentHeatUnit;
	uint32   CurrentHeat;				   /*当前热量*/

    uint8     HeatPowerUnit;
	uint32 	 HeatPower;			      /*热功率*/

    uint8 	 FlowUnit;
	uint32 	 Flow;			                 /*流量流速*/

    uint8 	AccumulateFlowUnit;	
	uint32 	AccumulateFlow;			/*累积流量*/

	uint8	WaterInTemp[3];		        /*供水温度*/	
	uint8 	WaterOutTemp[3];				/*回水温度*/
    uint8 	AccumulateWorkTime[3];	/*累计工作时间*/
	uint8	RealTime[7];		                /*实时时间*/	
	uint16 	ST;	
	}JingWeiXiping_Format;






typedef struct{
    uint8     Nouse[9];
    uint8     Vertion;
    uint8     Medium;
    uint8     AccessCnt;
    uint8     Status;
    uint16     Signature;
    uint8     Fabrication_ID[6];
    uint8     Firmware_Version[5];
    uint8     FlowTemperatureUnit[2];
    uint16   FlowTemperature;
    uint8     ReturnTemperatureUnit[2];
    uint16   ReturnTemperature;
    uint8     DiffTemperatureUnit[2];
    uint16   DiffTemperature;

    uint8     Averaging_time[3];

    uint8     FlowRateUnit[2];
    uint32     FlowRate;

    uint8     MaxFlowRateUnit[2];
    uint32   MaxFlowRate;

    uint8     MaxLatestFlowRateUnit[2];
    uint32     MaxLatestFlowRate;

    uint8     MaxPrevFlowRate[7];

    uint8     PowerUnit[2];
    uint32   Power;

    uint8     MaxPower[6];
    uint8     MaxLatestPower[6];
    uint8     MaxPrevPower[7];

    uint8     VolumeUnit[2];
    uint32   Volume;

    uint8     VolumeLatest[6];
    uint8     VolumePrev[7];

    uint8     Futurekeydate1[5];
    uint8     Futurekeydate2[6];

    uint8     Operating_Time_err[6];
    
    uint8     Operating_Time_Unit[2];
    uint32     Operating_Time;

    uint8     DateTime[6];

    uint8     CurrentHeatUnit[2];
    uint32   CurrentHeat;				   /*当前热量*/

    uint8     Currentcooling[7];
    
	
	}ZENNER_Format;
typedef struct{
    uint8     Nouse[12];
    uint8     Status;
    uint16   Signature;
    uint8     Updating_time[3];
    uint8     Averaging_time[3];
  
    uint8     CurrentHeatUnit[2];                  /*06h/0Eh VIF: Heat quantity (kWh, MJ)*/
    uint32   CurrentHeat;				   /*当前热量*/

    uint8     VolumeUnit[2];                   /*  m3*1/100  */
    uint32   Volume;

    uint8     PowerUnit[2];                     /*  Heat power (kW/10)   */
    uint8   Power[3];

    uint8     FlowRateUnit[2];          /*Flowrate (l/h)*/
    uint8     FlowRate[3];

    uint8     FlowTemperatureUnit[2];   /*Flow temperature (°C)*/
    uint16   FlowTemperature;
	
    uint8     ReturnTemperatureUnit[2]; /*Return temperature (°C)*/
    uint16   ReturnTemperature;
	}LANDISGYR_Format;

typedef struct{
	uint32   DailyHeat;		/*结算日热量*/	
    uint8     DailyHeatUnit;
	uint32   CurrentHeat;				/*当前热量*/
    uint8     CurrentHeatUnit;
	uint32 	 HeatPower;			/*热功率*/
    uint8     HeatPowerUnit;
	uint32 	 Flow;			/*流量*/
    uint8 	 FlowUnit;
	uint32 	AccumulateFlow;					/*累积流量*/
    uint8 	AccumulateFlowUnit;	
	uint8	WaterInTemp[3];		/*供水温度*/	
	uint8 	WaterOutTemp[3];					        /*回水温度*/
    uint8 	AccumulateWorkTime[3];					/*累计工作时间*/
	uint8	RealTime[7];		/*实时时间*/	
	uint16 	ST;					        /*状态ST*/  
	}Danfoss_Format;
typedef struct{
    uint8     DailyHeatUnit;
	uint32   DailyHeat;		/*结算日热量*/	
    uint8     CurrentHeatUnit;
	uint32   CurrentHeat;				/*当前热量*/
    uint8     HeatPowerUnit;
	uint32 	 HeatPower;			/*热功率*/
    uint8 	 FlowUnit;
	uint32 	 Flow;			/*流量*/
    uint8 	AccumulateFlowUnit;	
	uint32 	AccumulateFlow;					/*累积流量*/
    
	uint8	WaterInTemp[3];		/*供水温度*/	
	uint8 	WaterOutTemp[3];					        /*回水温度*/
    uint8 	AccumulateWorkTime[3];					/*累计工作时间*/
	uint8	RealTime[7];		/*实时时间*/	
	uint16 	ST;					        /*状态ST*/  
	}LICHUANG_Format;
typedef struct{
    uint8     DailyHeatUnit;
	uint32   DailyHeat;		/*结算日热量*/	
    uint8     CurrentHeatUnit;
	uint32   CurrentHeat;				/*当前热量*/
    uint8     HeatPowerUnit;
	uint32 	 HeatPower;			/*热功率*/
    uint8 	 FlowUnit;
	uint32 	 Flow;			/*流量*/
    uint8 	AccumulateFlowUnit;	
	uint32 	AccumulateFlow;					/*累积流量*/
    
	uint8	WaterInTemp[3];		/*供水温度*/	
	uint8 	WaterOutTemp[3];					        /*回水温度*/
    uint8 	AccumulateWorkTime[3];					/*累计工作时间*/
	uint8	RealTime[7];		/*实时时间*/	
	uint16 	ST;					        /*状态ST*/  
	}WANHUA_Format;
typedef struct{
	uint32   DailyHeat;		/*冷量*/	
    uint8     DailyHeatUnit;
	uint32   CurrentHeat;				/*热量*/
    uint8     CurrentHeatUnit;
    uint8     CurrentHeatDecimal[2];/*热量小数*/
    uint8     AccumulateFlowDecimal[3];/*累积流量小数*/
	uint32 	 Flow;			/*流量*/
    uint8 	 FlowUnit;
	uint32 	AccumulateFlow;					/*累积流量*/
    uint8 	AccumulateFlowUnit;	
	uint8	WaterInTemp[3];		/*供水温度*/	
	uint8 	WaterOutTemp[3];					        /*回水温度*/
    uint8 	AccumulateWorkTime[3];					/*累计工作时间*/
	uint8	RealTime[7];		        /*实时时间*/	
	uint16 	ST;					        /*状态ST*/  
	}LEYE485_Format;

typedef struct{
    uint8 Nouse[15];
    uint16   CurrentEnergyUnit;
	uint32   CurrentEnerg;		/*累积热量*/	
    uint8     CurrenTariff1[7];
	uint8     CurrentTariff2[7];			/*当前热量*/
    uint16   CurrentVolumeUnit;
	uint32 	 CurrentVolume;			/*累积流量*/
    uint16 	 CurrentPowerUnit;
	uint32 	 CurrentPower;			/*热功率*/
    uint16 	 CurrentFlowRateUnit;	
	uint8 	 CurrentFlowRate[3];					/*流量*/

    uint16  ForwardTemperatureUnit;
	uint8	ForwardTemperature[2];		/*供水温度*/	
	uint16  ReturnTemperatureUnit;
	uint8	ReturnTemperature[2];		/*回水温度*/	
  
	uint8	RealTime[9];		/*实时时间*/	
	}HYDROMETER_Format;



#pragma pack()
extern uint8 Get_Current_Channel(void);
extern void Reset_Current_Channel(void);

extern uint8 DelayTime_485;           //  485通讯抄取下行表计的延时时间     十进制
extern void METER_CreateFrame(DELU_Protocol *pData, uint8 *pSendFrame, uint8 *plenFrame);
extern uint8 METER_ReceiveFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen);
extern uint8 METER_DELU_AnalDataFrame(DELU_Protocol *pProtoclData, uint8 *pRecFrame);
extern uint8 METER_MeterCommunicate(DELU_Protocol *pData, uint8 *pResBuf, uint8 *pDataLenBack);
extern uint8 METER_MeterCommunicate_Direct(uint8 *pData, uint8 InLen, uint8 *pResBuf, uint8 *pOutLen);
extern uint8 Communicate_ForAll(uint8 *pData);
extern uint8 METER_ReadMeterDataTiming(uint16 MeterSn, uint8 *EleBuf);
extern uint8 VALVE_ReadMeterDataTiming(uint16 MeterSn, uint8 *EleBuf);
extern uint8 METER_ReadMeterDataCur(DELU_Protocol *pReadMeter, uint16 MeterSn);
extern void DisableAllMBusChannel(void);
extern uint8 METER_ChangeChannel(uint8 Channel);
extern uint8 METER_DataItem(DELU_Protocol *pProtocoalInfo);
extern void YINLINVALVE_CreateFrame(DELU_Protocol *pData, uint8 *pSendFrame, uint8 *plenFrame);
extern uint8 METER_MeterDataForValve(DELU_Protocol *pProtocoalInfo, uint8 ProtocolVer);
extern CJ188_Format METER_Data_To_CJ188Format(uint8 ProtocolVer,uint8* DataBuf,uint8 len,uint8* err);
extern uint8 To_Little_endian(uint8* Databuff,uint8 size);
extern uint8 Yilin_Calc_Parity(uint8 *pStart, uint8 ucLen);
extern uint8 ReadData_Communicate(uint8 *DataFrame, uint8 *DataLen);
extern uint8 Yilin_Valve_ReceiveFrame(uint8 *buf, uint8 *datalen);

#endif

