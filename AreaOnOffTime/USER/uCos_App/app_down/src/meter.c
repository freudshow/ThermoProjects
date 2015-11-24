/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: Meter.c
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年9月10日 
**  描        述: 热计量表协议解析
**	修 改  记 录:   	
*****************************************************************************************************/

/********************************************** include *********************************************/
#include <includes.h>
#include "app_down.h"
#include "Valve.h"

#include "ModBus.h" //测试modbus---林晓彬添加
/********************************************** define *********************************************/
#define   BIT2_LOW_VOLTAGE    0x04
#define   BIT3_RESERVE    0x08    /*reserve*/
#define   BIT4_RESERVE    0x10    /*reserve*/
#define   BIT5_TEMP_SENSOR    0x20    /*温度传感器故障*/
#define   BIT6_FLOW_SENSOR    0x40   /*流量传感器故障*/
#define   BIT7_N_TEMP_DIFF    0x80    /*negative temperature difference*/
#ifdef DEMO_APP
extern CJ188_Format Demo_CJ188_ch1;
extern CJ188_Format Demo_CJ188_ch2;
extern CJ188_Format Demo_CJ188_ch3;
extern uint8 Show_Data_Status;
extern uint8 Data_complete;

#endif

#define  BIT0   0X01
#define  BIT1   0X02
#define  BIT2   0X04

/*CJ188 标准*/
#define  WH   0X02
#define  KWH   0X05
#define  MWH   0X08
#define  MWH100   0X0A
#define  J   0X01
#define  KJ   0X0B
#define  MJ   0X0E
#define  GJ   0X11
#define  GJ100   0X13
#define  W   0X14
#define  KW   0X17
#define  MW   0X1A
#define  L  0X29
#define  M3   0X2C
#define  L_H  0X32
#define  M3_H   0X35


#if 0
typedef struct  
{
	uint8 Extension:1;
	uint8 LSB:1;
	uint8 Function:2;
    uint8 BCD:1;     /**/
	uint8 Data:3;
}ST_DIF;
typedef struct  
{
	uint8 Extension:1;
	uint8 Unit:1;
	uint8 Tariff:2;
	uint8 StorageNumber:4;
}ST_DIFE;
typedef struct  
{
	uint8 Extension:1;
	uint8 Data:7;
}ST_VIF;
typedef struct  
{
	uint8 Extension:1;
	uint8 value:7;
}ST_VIFE;
#else
typedef struct  
{
  uint8 Data:3;
  uint8 BCD:1;     /**/
  uint8 Function:2;
  uint8 LSB:1;
  uint8 Extension:1;
}ST_DIF;
typedef struct  
{
  uint8 StorageNumber:4;
  uint8 Tariff:2;
  uint8 Unit:1;
  uint8 Extension:1;
}ST_DIFE;
typedef struct  
{
	uint8 Data:7;
  	uint8 Extension:1;
}ST_VIF;
typedef struct  
{
	uint8 value:7;
    uint8 Extension:1;
}ST_VIFE;
#endif
/********************************************** global *********************************************/
OS_EVENT *UpAskMeterSem;
OS_EVENT *METERChangeChannelSem;


/********************************************** static *********************************************/
static uint8 gMETER_FrameSer = 0x00;																

extern char gPrintData[1024];

extern uint8 gDownCommDev485;


uint8 gCurrent_Channel = 0;  //当前MBUS处于第几通道。0表示通道全关。



uint8   HYDROMETER_TO_CJ188(CJ188_Format  *MeterData,uint8* DataBuf,uint8 len);
uint8  ENLEMAN_TO_CJ188(CJ188_Format  *MeterData,uint8* DataBuf,uint8 len);


extern  void  rtu_read_coil_status ( ModBusMRData *PLCCMD,unsigned char n); //林晓彬添加
//
uint8 METER_MeterCommunicate_Direct_PLC(uint8 *pData, uint8 InLen, uint8 *pResBuf, uint8 *pOutLen);  //林晓彬添加
/****************************************************************************************************
**	函 数  名 称: METER_CreateFrame
**	函 数  功 能: 集中器对热计量表数据组帧
**	输 入  参 数: DELU_Protocol *pReadMeter -- 读取热计量表的数据结构
**	输 出  参 数: uint8  *_sendBuf -- 组建出的数据帧; 
**                uint16 *plenFrame -- 组建出的数据帧长度
**  返   回   值: 无
**	备		  注: 本组帧函数，根据热计量表数据帧结构体内信息进行组帧
*****************************************************************************************************/
void METER_CreateFrame(DELU_Protocol *pData, uint8 *pSendFrame, uint8 *plenFrame)
{
	uint8 templen	= 0x00;
	uint8 cs		= 0x00;
	uint8 *pTemp, *pTemp1;
    uint16 lenFrame = 0x00;
    uint16 CheckLen = 0x00;
    
    LOG_assert_param(pData == NULL);
    LOG_assert_param(pSendFrame == NULL);
    LOG_assert_param(plenFrame == NULL);
	
	pTemp = pSendFrame;
	
	memset(pTemp, 0xFE, pData->PreSmybolNum);		//填入前导字符0xFE, 个数视情况而定
	pTemp += pData->PreSmybolNum;
	lenFrame += pData->PreSmybolNum;
	
	pTemp1 = pTemp;									//预留计算校验位
	*pTemp++ = DELU_FRAME_START;					//组建 帧头 0x68
	*pTemp++ = pData->MeterType;					//仪表类型
	memcpy(pTemp, pData->MeterAddr, 7);				//组建 地址域
	pTemp += 7;
	lenFrame += 9;
	CheckLen += 9;
	
	*pTemp++ = pData->ControlCode;					//组建 控制码
	*pTemp++ = pData->Length;						//组建 数据域长度
	lenFrame += 2;
	CheckLen += 2;
	
	*pTemp++ = pData->DataIdentifier;
	*pTemp++ = (pData->DataIdentifier) >> 8;		//组建 数据标识符
	*pTemp++ = gMETER_FrameSer++;
	lenFrame += 3;
	CheckLen += 3;
	
	templen = pData->Length-3;
	memcpy(pTemp, pData->DataBuf, templen);
	pTemp += templen;
	lenFrame += templen;
	CheckLen += templen;
	
	cs = PUBLIC_CountCS(pTemp1, CheckLen);			//计算 校验字节
	*pTemp++ = cs;									//组建 校验位
	*pTemp = 0x16;									//组建 结束位
	lenFrame += 2;									//组建 数据帧长度
	
	*plenFrame = lenFrame;							//返回数据帧长度
}

uint8 Yilin_Calc_Parity(uint8 *pStart, uint8 ucLen)
{
	uint32 i = 0;
	uint32 value = 0;
	for (i = 0;i < ucLen;i++)
	{
		value += pStart[i];
	}
	value = value & 0xff;
	value = value ^ 0xA5;
	return value;
}


/****************************************************************************************************
**	函 数  名 称: YINLINVALVE_CreateFrame
**	函 数  功 能: 集中器对热计量表数据组帧
**	输 入  参 数: DELU_Protocol *pReadMeter -- 读取热计量表的数据结构
**	输 出  参 数: uint8  *_sendBuf -- 组建出的数据帧; 
**                uint16 *plenFrame -- 组建出的数据帧长度
**  返   回   值: 无
**	备		  注: 本组帧函数，根据热计量表数据帧结构体内信息进行组帧
*****************************************************************************************************/
void YINLINVALVE_CreateFrame(DELU_Protocol *pData, uint8 *pSendFrame, uint8 *plenFrame)
{
	uint8 i=0;
    //UINT8 ucTempLoop = 0;
	uint8 *pTemp = pSendFrame;
	uint8 *pCheck = pTemp;

	*pTemp++ = pData->ControlCode;				// 命令类型

	for(i = 0; i < 2; i++)					// 仪表地址 做逆序
	{
		*pTemp++ = pData->MeterAddr[i];
	}
       for(i = 0;i < 4;i++)
       {
          *pTemp++ = pData->DataBuf[i];
       }

	*pTemp++ = Yilin_Calc_Parity(pCheck, 7);
	
     *plenFrame = 8;
	
							//返回数据帧长度
}



/****************************************************************************************************
**	函 数  名 称: METER_ReceiveFrame
**	函 数  功 能: 集中器中的计量模块接收上位机下发的对其操作的命令
**	输 入  参 数: uint8 dev -- 下行通道设备; 
**                uint16 Out_Time -- 接收命令超时等待时间
**	输 出  参 数: uint8 *buf -- 输出数据指针; 
**                uint8 *datalen -- 输出接收到数据帧长度
**  返   回   值: 0 -- 接收到有效帧;		1 -- 数据帧头不正确; 		2 -- 接收到的数据帧不是本电能表地址
**				  4 -- 数据帧长度不够		5 -- 数据帧校验不正确		6 -- 数据帧尾不正确
**	备		  注: 此处输出参数最好用联合体或宏定义，错误的返回值
*****************************************************************************************************/

uint8 METER_ReceiveFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen)
{
	uint8 data 	= 0x00;
	uint8 len	= 0x00;
	uint8 Cs	= 0x00;
	uint32 i,j;
	uint8  rtu_number_long;

	LOG_assert_param(dev != 0x00);
	LOG_assert_param(buf == NULL);
	LOG_assert_param(datalen == NULL);
	
	*datalen = 0;
	 	//林晓彬在这里添加改动，原来只能够读取热量表，这里需要再根据表dev增加一个判断过程，如果是485则采用标准的modbus，否则使用原来的热量表回读过程	
   if(dev == DOWN_COMM_DEV_485_PLC){
	 		//这里为PLC新建一个接收	 ，这个属于测试，将发送给485的年数据的数据接收过来
		rtu_number_long=0;//计算接收的数据总量 ---标志量
        for(i=0;i<9;i++){	  //能够正确的读回485的数据
            DuGetch(DOWN_COMM_DEV_485,&buf[i],OS_TICKS_PER_SEC*2);			//	OS_TICKS_PER_SEC*2  //将485的数据读回来
            rtu_number_long++;
           }
		*datalen = rtu_number_long;
   }
   else	{	
	i = 30;
	while(i--)														//找帧头
	{
		FeedTaskDog();   
		if(DuGetch(dev, &data, Out_Time))  			
			return 1;
		if(data==0x68)	break; 	     
	}
	Cs 		= data;
	*buf++ 	= data;
	
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC*3))  		
		return 2;		//表类型
	Cs 		+= data;
	*buf++	 = data;
	
    for(i=0; i<7; i++)												//地址
    {
		if(DuGetch(dev, &data, OS_TICKS_PER_SEC*3))  	
			return 3;
		Cs 		+= data;
		*buf++	 = data;
	}
	
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC*3))  		
		return 4;		//控制码
	Cs 		+= data;
	*buf++	 = data;
	
	if(DuGetch(dev, &len, OS_TICKS_PER_SEC*3))  		
		return 5;		//数据长度
	if(len > METER_FRAME_LEN_MAX)             		
		return 10;
	Cs 		+= len;
	*buf++	 = len;

	for(j=0; j<len; j++)											//数据域
	{
	    if(DuGetch(dev, &data, OS_TICKS_PER_SEC*3))  	
			return 6;
		*buf++  = data;
		Cs     += data;
    }  
     
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC*3))  		
		return 7;		//校验字节
	if(data != Cs)         							
		return 11;   
	*buf++	= data; 
	
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC*3))  		
		return 8;		//结束符
	if(data != 0x16)         						
		return 12;
	   
  	*buf++		= data; 
  	*datalen 	= len + 16;
  	  }
    return NO_ERR;
}

  

/****************************************************************************************************
**	函 数  名 称: METER_DELU_AnalDataFrame
**	函 数  功 能: 集解析读取来的热计量数据
**	输 入  参 数: Duint8 *pRecFrame -- 输入的待解帧的数据
**	输 出  参 数: DELU_Protocol *pProtoclData -- 解帧后的数据结构
**  返   回   值: NO_ERR
**	备		  注: 
*****************************************************************************************************/

uint8 METER_DELU_AnalDataFrame(DELU_Protocol *pProtoclData, uint8 *pRecFrame)
{
	uint8 *pTemp 		= pRecFrame;
	
	LOG_assert_param(pProtoclData == NULL);
	LOG_assert_param(pRecFrame == NULL);
	if(pProtoclData->MeterType == 0x20)
	{
		if(0x68 != *pTemp++)	return 1;	
		
		pProtoclData->MeterType = *pTemp++;										//仪表类型
		memcpy(pProtoclData->MeterAddr, pTemp, 7);								//仪表地址
		pTemp += 7;
		
		pProtoclData->ControlCode	= *pTemp++;									//控制码
		pProtoclData->Length		= *pTemp++;									//长度
		pProtoclData->DataIdentifier= (*pTemp++) | (*pTemp++ << 8);
		pProtoclData->SER			= *pTemp++;
		memcpy(pProtoclData->DataBuf, pTemp, (pProtoclData->Length-3));			//数据域数据
		pTemp += pProtoclData->Length-3;
	}
	else if(pProtoclData->MeterType == 0xB0)
	{
		pProtoclData->ControlCode	= *pTemp++;									//仪表类型
		memcpy(pProtoclData->MeterAddr, pTemp, 2);								//仪表地址
		pTemp += 2;
		memcpy(pProtoclData->DataBuf, pTemp, 8);			//数据域数据
	}
	else if(pProtoclData->MeterType == 0x41){ //济南普赛通讯阀控器。
		if(0x68 != *pTemp++)	return 1;	
		
		pProtoclData->MeterType = *pTemp++;										//仪表类型
		memcpy(pProtoclData->MeterAddr, pTemp, 7);								//仪表地址
		pTemp += 7;
		
		pProtoclData->ControlCode	= *pTemp++;									//控制码
		pProtoclData->Length		= *pTemp++;									//长度
		pProtoclData->DataIdentifier= (*pTemp++) | (*pTemp++ << 8);
		pProtoclData->SER			= *pTemp++;
		memcpy(pProtoclData->DataBuf, pTemp, (pProtoclData->Length-3));			//数据域数据
		pTemp += pProtoclData->Length-3;

	}

	return NO_ERR;
}

/****************************************************************************************************
**	函 数  名 称: METER_MeterCommunicate
**	函 数  功 能: 集中器与热计量表通信
**	输 入  参 数: DELU_Protocol *pData -- 需下发的德鲁协议数据帧
**	输 出  参 数: uint8 *pResBuf -- 接收到的数据帧回应; 
**                uint8 *pDataLenBack -- 数据帧长度
**  返   回   值: NO_ERR, ERR_1 -- 没有接收到数据
**	备		  注: 
*****************************************************************************************************/

uint8 METER_MeterCommunicate(DELU_Protocol *pData, uint8 *pResBuf, uint8 *pDataLenBack)
{
	  uint8 err;
      uint8 *cp = NULL;
      uint8 DataFrame[METER_FRAME_LEN_MAX];
      uint8 DataLen = 0x00;
      
      LOG_assert_param(pData == NULL);
	  LOG_assert_param(pResBuf == NULL);
	  LOG_assert_param(pDataLenBack == NULL);
      if(pData->MeterType == 0xB0)
     	  	YINLINVALVE_CreateFrame(pData, DataFrame, &DataLen);
      else
           	METER_CreateFrame(pData, DataFrame, &DataLen);
     	  
      LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate> CreateFrame successful");
      debug_debug(gDebugModule[TASKDOWN_MODULE],"INFO: <METER_MeterCommunicate> CreateFrame successful!");	
           
      do{
     		FeedTaskDog();
     		OSSemPend(UpAskMeterSem, 5*OS_TICKS_PER_SEC, &err);                         //申请MBUS的通道
      }while(err!=OS_ERR_NONE);
	  
	 LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate> Start Read HeatMeter!");
     debug_debug(gDebugModule[TASKDOWN_MODULE],"INFO: <METER_MeterCommunicate> Start Read HeatMeter!");	

		   /*begin:yangfei added 2013-02-21 */
     	if(gDebugModule[TASKDOWN_MODULE] >= KERN_DEBUG)
     		{
     			PUBLIC_HexStreamToString(DataFrame, DataLen, gPrintData);
     			debug_debug(gDebugModule[TASKDOWN_MODULE],"Meter send data:%s\r\n",gPrintData);	
     		}
     	/*end:yangfei added 2013-02-21*/
        	  cp=(uint8*)Uart0SendThenReceive_PostPend(DataFrame, &DataLen);
         
        	  OSSemPost(UpAskMeterSem);
       	  if(cp[0]) {							//出错
       	    	debug_debug(gDebugModule[TASKDOWN_MODULE], "WARNING: <METER_MeterCommunicate> Recive Data Time OUT!\r\n");
     		    return 1;
     	    } 
           else{
                 /*begin:yangfei added 2013-02-21 */
         	    if(gDebugModule[TASKDOWN_MODULE] >= KERN_DEBUG)
         		{
         			PUBLIC_HexStreamToString(DataFrame, DataLen, gPrintData);
         			debug_debug(gDebugModule[TASKDOWN_MODULE],"Meter recive data:%s\r\n",gPrintData);	
         		}
         	    /*end:yangfei added 2013-02-21*/
             }
           LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate> Read HeatMeter Data successful");
           
           memcpy(pResBuf, DataFrame, DataLen);
           *pDataLenBack = DataLen;
           return NO_ERR;
}

/****************************************************************************************************
**	函 数  名 称: METER_MeterCommunicate_Direct
**	函 数  功 能: 集中器与热计量表通信, 透明转发专用
**	输 入  参 数: uint8 *pData -- 需转发的数据帧; 
**                uint8 InLen -- 需转发的数据帧长度
**	输 出  参 数: uint8 *pResBuf -- 接收到的数据帧回应; 
**                uint8 *pOutLen -- 数据帧长度
**  返   回   值: NO_ERR, ERR_1 -- 没有接收到数据
**	备		  注: 
*****************************************************************************************************/
uint8 METER_MeterCommunicate_Direct(uint8 *pData, uint8 InLen, uint8 *pResBuf, uint8 *pOutLen)
{
	  uint8 i=0;
      uint8 *cp = NULL;
      uint8 DataFrame[METER_FRAME_LEN_MAX];
      uint8 DataLen = 0x00;
      uint8 Err = 0x00;
      
      LOG_assert_param(pData == NULL);
	  LOG_assert_param(pResBuf == NULL);
	  LOG_assert_param(pOutLen == NULL);
      DataLen = InLen;
      memcpy(DataFrame, pData, InLen);
      LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate_Direct> Send Datas Direct, The Frame Datas:");
      
      for(i=1; i<7; i++)
      {
      	
      	METER_ChangeChannel(i);
      	
      	do{
			FeedTaskDog();
			OSSemPend(UpAskMeterSem, 5*OS_TICKS_PER_SEC, &Err);                         //申请MBUS的通道
		}while(Err!=OS_ERR_NONE);
      	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate_Direct> Start Read HeatMeter, Channel is NO.%d", i);
   	  	cp=(uint8*)Uart0SendThenReceive_PostPend(DataFrame, &DataLen);
   	  	OSSemPost(UpAskMeterSem);
  	  	if(cp[0])
  	    	{							//出错
  	    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_MeterCommunicate_Direct> Recive Data Time OUT, Channel is NO.%d", i);
		    	Err++;
	    	}
	    else
	    	{
	    		break;
	    	}
	   } 
      
      if(Err>=5)	
      	{
      		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_MeterCommunicate_Direct> ALL Of Channels Recive Data Time OUT");
      		return 1;			//6个通道 都试了  都没有接收到数据
      	}
      	
      LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate> Read HeatMeter Data successful! Channel is NO.%d", i);
      memcpy(pResBuf, DataFrame, DataLen);
      *pOutLen = DataLen;
      return NO_ERR;
}



/****************************************************************************************************
**	函 数  名 称:METER_MeterCommunicate_Direct_PLC	  -----林晓彬添加本函数
**	函 数  功 能: 集中器与热计量表通信, 透明转发专用
**	输 入  参 数: uint8 *pData -- 需转发的数据帧; 
**                uint8 InLen -- 需转发的数据帧长度
**	输 出  参 数: uint8 *pResBuf -- 接收到的数据帧回应; 
**                uint8 *pOutLen -- 数据帧长度
**  返   回   值: NO_ERR, ERR_1 -- 没有接收到数据
**	备		  注:林晓彬在透明传输指令中做出了修改，添加了rtu_read_coil_status，是485的modus协议处理，导致这个函数只能够对PLC（485做出反应）
*****************************************************************************************************/
uint8 METER_MeterCommunicate_Direct_PLC(uint8 *pData, uint8 InLen, uint8 *pResBuf, uint8 *pOutLen)
{
	  uint8 i=0;
      uint8 *cp = NULL;
      uint8 DataFrame[METER_FRAME_LEN_MAX];
	  //uint8 DataFrame_Modbus[METER_FRAME_LEN_MAX];//林晓彬 添加	，主要是将上位机下发的数据传送给下面的PLC
      uint8 DataLen = 0x08;
      uint8 Err = 0x00;
      
      LOG_assert_param(pData == NULL);
	  LOG_assert_param(pResBuf == NULL);
	  LOG_assert_param(pOutLen == NULL);
      
      memcpy(DataFrame, pData, InLen); //将上位机的数据转存
	  //这里为了不影响后续设备或者透明传输的功能，应该在这里做出判断，只有采用485传输的时候才能够使用，或者在上位机做出数据修改
	  //在底层改的话影响更大，还是放在顶层修改MODBUS协议
	  rtu_read_coil_status((ModBusMRData*)DataFrame,6);	//林晓彬 添加  上位机传下数据 0x01030000100002到pData->MeterAddr[6]={0x02，0x00，0x01, 0x00, 0x03, 0x01, 0x00};	位置需要调整

      LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate_Direct_plc> Send Datas Direct, The Frame Datas:");
     
      for(i=1; i<7; i++)
      {
      	
      	METER_ChangeChannel(i);
      	
      	do{
			FeedTaskDog();
			OSSemPend(UpAskMeterSem, 5*OS_TICKS_PER_SEC, &Err);                         //申请MBUS的通道
		}while(Err!=OS_ERR_NONE);
      	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate_Direct_PLC> Start Read plc, Channel is NO.%d", i);
   	  	 
	  
	  	//cp=(uint8*)Uart0SendThenReceive_PostPend(DataFrame_Modbus, &DataLen);	 //这里 的	 DataLen 属于输出参数必须正确反应从表读回来的数据的长度
   	  	
		
		cp=(uint8*)Uart0SendThenReceive_PostPend(DataFrame, &DataLen);	 //这里 的	 DataLen 属于输出参数必须正确反应从表读回来的数据的长度

		OSSemPost(UpAskMeterSem);
  	  	if(cp[0])
  	    	{							//出错
  	    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_MeterCommunicate_Direct_PLC> Recive Data Time OUT, Channel is NO.%d", i);
		    	Err++;
	    	}
	    else
	    	{
	    		break;
	    	}
	   } 
      
      if(Err>=5)	
      	{
      		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_MeterCommunicate_Direct> ALL Of Channels Recive Data Time OUT");
      		return 1;			//6个通道 都试了  都没有接收到数据
      	}
      	
      LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_MeterCommunicate> Read HeatMeter Data successful! Channel is NO.%d", i);
      memcpy(pResBuf, &cp[2], 7);
      *pOutLen = DataLen;  //表的有效数据的长度
      return NO_ERR;
}
/****************************************************************************************************
**	函 数  名 称: Communicate_ForAll
**	函 数  功 能: 集中器向热表或阀控器群发消息。
**	输 入  参 数: 
**         
**	输 出  参 数: 
**               
**  返   回   值: NO_ERR,固定返回无错误。
**	备		  注: 
*****************************************************************************************************/
uint8 Communicate_ForAll(uint8 *pData)
{
	  uint8 i = 0;
      uint8 *cp = NULL;
      uint8 lu8DataFrame[METER_FRAME_LEN_MAX];
      uint8 lu8DataLen = 0x00;
      uint8 Err = 0x00;
	  uint8 lu8RetryTimes = 0;
	  uint8 lu8DevType = 0;
	  uint8 lu8ProtocolType = 0;
	  uint16 lu16meternum = 0;
      
      LOG_assert_param(pData == NULL);

	  FeedTaskDog();

	  
	  lu8DevType = *(pData + 1);  //设备类型
	  lu8ProtocolType = *(pData + 3); //协议版本号

	  lu8DataLen = *(pData + 5);  //帧长度
	  memcpy(lu8DataFrame,pData+6,lu8DataLen);

	  if(lu8DevType == HEAT_METER_TYPE){  //热表
			(*METER_ComParaSetArray[gMETER_Table[lu8ProtocolType][0]])();
	  }
	  else if(lu8DevType == TIME_ON_OFF_AREA_TYPE){  //阀控器。
			(*METER_ComParaSetArray[gVALVE_Table[lu8ProtocolType][0]])();  //设置阀门对应串口参数。
	  }
	  else{
			//待需要时补充。
	  }

	  FeedTaskDog();

	  for(i=1; i<=6; i++){	//MBUS通道4个。
	  	lu16meternum = gPARA_MeterChannelNum[i-1];
	  	if(lu16meternum > 0){
	      	METER_ChangeChannel(i);
			FeedTaskDog();
	      	do{
				FeedTaskDog();
				OSSemPend(UpAskMeterSem, 5*OS_TICKS_PER_SEC, &Err);                         //申请MBUS的通道
			}while(Err!=OS_ERR_NONE);
			
			for(lu8RetryTimes= 0;lu8RetryTimes<5;lu8RetryTimes++)  //为保证群发成功率，群发4次(经测试不能少于4次).
			{
				FeedTaskDog();
	 			DuQueueFlush(DOWN_COMM_DEV_MBUS);   										//清空缓冲区
				DuSend(DOWN_COMM_DEV_MBUS,lu8DataFrame,lu8DataLen);
				OSTimeDly(2*OS_TICKS_PER_SEC);   //为保证成功率，间隔3秒钟。
			}
			
			OSSemPost(UpAskMeterSem);
	  	}
	 } 

//处理RS485通道。
	  lu16meternum = gPARA_MeterChannelNum[METER_CHANNEL_NUM-1];
	  if(lu16meternum > 0){

	  		METER_ChangeChannel(METER_CHANNEL_NUM);  //最后一个通道是RS485通道。
	  		FeedTaskDog();
	  		do{
		  		FeedTaskDog();
		  		OSSemPend(UpAskMeterSem, 5*OS_TICKS_PER_SEC, &Err);   //申请MBUS的通道
	  		}while(Err!=OS_ERR_NONE);
			
	  		for(lu8RetryTimes= 0;lu8RetryTimes<5;lu8RetryTimes++)  //为保证群发成功率，群发4次。
	  		{
	 			FeedTaskDog();
	 			DuQueueFlush(DOWN_COMM_DEV_485);   										//清空缓冲区
				DuSend(DOWN_COMM_DEV_485,lu8DataFrame,lu8DataLen);
				OSTimeDly(2*OS_TICKS_PER_SEC);
	  		}
			
	  		OSSemPost(UpAskMeterSem);
	  }

	  

	  
      return NO_ERR;  //固定返回NO_ERR。
}


/****************************************************************************************************
**	函 数  名 称: METER_ReadMeterDataTiming
**	函 数  功 能: 集中器读取下挂热计量表的数据及读取控制
**	输 入  参 数: uint16 MeterSn -- 表序号;
**	输 出  参 数: uint8 *EleBuf -- 接收到的数据帧回应;
**  返   回   值: NO_ERR, ERR_1 -- 没有接收到数据
**	备		  注: 填写下行读取数据的相关数据结构
**  SD卡中每个表的数据占用128字节的空间，其中第一个字节为数据长度(1字节) + 数据(N+11) + CS累加和校验(1字节)
**	其中数据格式为: 热计量表地址(7字节) + 热量数据长度(1字节) + 热量数据(N) + 温度数据(3字节)
**	若读取到得数据不正确，或没有读取到数据，则规定热量数据长度为0，无热量数据。
*****************************************************************************************************/
uint8 METER_ReadMeterDataTiming(uint16 MeterSn, uint8 *EleBuf)
{
	uint8 i				= 0x00;
	uint8 Err 		 	= 0x00;
	//uint8 RetryTimes 	= 0x03;
	uint8 DataLen		= 0x00;
	uint8 DataBuf[128]	= {0x00};
	uint8 ReadMeterSuccess = 0;
	uint8 *pTemp = DataBuf;
	MeterFileType	mf;
	DELU_Protocol	ProtocoalInfo;
	uint8 lu8DataIn[100] = {0};  //在阀控控制时用于传递阀控控制信息。
	uint8 DataFrame[METER_FRAME_LEN_MAX];
	uint8 DataLen_Vave = 0x00;
	uint8 lu8ReadTime[6] = {0};
	uint8 *p_ReadVavleTime;  //抄阀控器时间预留。
	CPU_SR		cpu_sr;

    CJ188_Format CJ188_Data;
	
	LOG_assert_param(EleBuf == NULL);
	LOG_assert_param(MeterSn > METER_NUM_MAX);

    Err = PARA_ReadMeterInfo(MeterSn, &mf);
    if(Err != NO_ERR){
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_ReadMeterDataTiming> Read Meter filed failed, MeterSn is %4XH, Return is %d"
    								,MeterSn, Err);
    }
	if(mf.ProtocolVer>PROTOCOL_MAX){
        mf.ProtocolVer = 0;
	}
	
	/*begin:yangfei added 2013-11-2 for 判断485 热表必须为7通道*/
	if(mf.ProtocolVer==4 && mf.ChannelIndex!=7){
		debug_err(gDebugModule[METER_DATA],"\r\n %s:485 meter channel!=7 \r\n",__FUNCTION__);
		goto ERROR;
	}
	/*end:yangfei added 2013-11-2 for 判断485 热表必须为7通道*/
	
	(*METER_ComParaSetArray[gMETER_Table[mf.ProtocolVer][0]])();  //按照协议，设置抄表串口参数。
	
    /*begin:yangfei added for support 485 meter*/
    gDownCommDev485 = gMETER_Table[mf.ProtocolVer][3];
    /*end:yangfei added for support 485 meter*/
	
	//填写相应的 热计量表通信协议 结构体
	ProtocoalInfo.PreSmybolNum  = gMETER_Table[mf.ProtocolVer][2];
	ProtocoalInfo.MeterType 	= 0x20;

	if(mf.ProtocolVer == 2)
	{							//若为丹佛斯表，则将表地址前5位为FFFFF
		for(i=0; i<7; i++){
			ProtocoalInfo.MeterAddr[i] = mf.MeterAddr[6-i];
		}
		ProtocoalInfo.MeterAddr[0] = 0xFF;
		ProtocoalInfo.MeterAddr[1] = 0xFF;
		ProtocoalInfo.MeterAddr[2] |= 0xF0;
	}
    /*begin:yangfei added for support wanhua big meter 2013-08-12*/
	else if(mf.ProtocolVer == 5){
		memcpy(ProtocoalInfo.MeterAddr, mf.MeterAddr, 7);
		ProtocoalInfo.MeterAddr[5] = 0x01;
	}
	/*end:yangfei added for support wanhua big meter 2013-08-12*/
	else{
		memcpy(ProtocoalInfo.MeterAddr, mf.MeterAddr, 7);
	}
	ProtocoalInfo.ControlCode 	= 0x01;
	ProtocoalInfo.Length		= 0x03;
	ProtocoalInfo.DataIdentifier= gMETER_Table[mf.ProtocolVer][1];
	memset(ProtocoalInfo.DataBuf, 0x00, METER_FRAME_LEN_MAX);
	
	pTemp++;	//为数据长度预留
	memcpy(pTemp,(uint8 *)&(mf.MeterID), 2);  //MeterID.
	pTemp += 2;
	DataLen = 2;
	*pTemp = mf.EquipmentType;  //设备类型。
	pTemp += 1;
	DataLen += 1;
	memcpy(pTemp, mf.MeterAddr, 7);
	pTemp += 7;
	DataLen += 7;
	*pTemp = mf.BulidID;  //楼号。
	pTemp += 1;
	DataLen += 1;
	*pTemp = mf.UnitID;  //单元号。
	pTemp += 1;
	DataLen += 1;
	memcpy(pTemp,(uint8 *)&(mf.RoomID), 2); //房间号
	pTemp += 2;
	DataLen += 2;

	ReadDateTime(lu8ReadTime);
	memcpy(pTemp,lu8ReadTime, 3); //加入抄表时间当前时间，只放时、分、秒。
	pTemp += 3;
	DataLen += 3;

	if(MeterNoBcdCheck(mf.MeterAddr) == TRUE){  //只有表地址不为空时才执行。
	
	 	/*begin:yangfei added 2013-08-05 for add HYDROMETER*/
   		 if(mf.ProtocolVer == HYDROMETER775_VER||mf.ProtocolVer == ZENNER_VER||mf.ProtocolVer == LANDISGYR_VER||mf.ProtocolVer == ENGELMANN_VER)   {
      			Err = HYDROMETER(&ProtocoalInfo,mf.ProtocolVer);
      	 }
    	 else{
     		 Err = METER_DataItem(&ProtocoalInfo);
      	 }
    	 /*end:yangfei added 2013-08-05 for add HYDROMETER*/
		if(Err == NO_ERR){
           	 /*begin:yangfei added 2013-03-18 for meter data format standardized 188*/
           	 if(mf.ProtocolVer == HYDROMETER775_VER||mf.ProtocolVer == ZENNER_VER||mf.ProtocolVer == LANDISGYR_VER||mf.ProtocolVer == ENGELMANN_VER)
           	 {
               	uint8 err=0;
               	*pTemp++ = sizeof(CJ188_Data) ;
	           	DataLen++;
              	//memcpy(pTemp, ProtocoalInfo.DataBuf, (ProtocoalInfo.Length-3));
               	CJ188_Data=METER_Data_To_CJ188Format(mf.ProtocolVer,ProtocoalInfo.DataBuf,ProtocoalInfo.Length-3,&err);
               	if(err ==0){
               		 memcpy(pTemp, &CJ188_Data, sizeof(CJ188_Data));
				     ProtocoalInfo.Length = sizeof(CJ188_Data) + 3;
                }
               	else{
               		 debug("%s %d METER_Data_To_CJ188Format err=%d\r\n",__FUNCTION__,__LINE__,err );
               		 memcpy(pTemp, ProtocoalInfo.DataBuf, (ProtocoalInfo.Length-3));/*上报原始数据*/
                }
             
              }
              else {
             		 uint8 err=0;
              		*pTemp++ = sizeof(CJ188_Data);
			  		DataLen++;
 			  		CJ188_Data=METER_Data_To_CJ188Format(mf.ProtocolVer,ProtocoalInfo.DataBuf,sizeof(CJ188_Data) ,&err);
              		memcpy(pTemp, &CJ188_Data,sizeof(CJ188_Data) );

              }
           		 /*end:yangfei added 2013-03-18 for meter data format standardized*/
 				pTemp += sizeof(CJ188_Data) ;
 				DataLen += sizeof(CJ188_Data) ;

 			
 				//OS_ENTER_CRITICAL();    //为优化LCD显示，将ReadCmplNums++放在本函数的最后。
				//gREAD_TimingState.ReadCmplNums++;
				//OS_EXIT_CRITICAL();
				ReadMeterSuccess = 1;
           		 debug_info(gDebugModule[TASKDOWN_MODULE],"%s %d Read HeatMeter data  ok",__FUNCTION__,__LINE__);
		}
		else{
ERROR:       debug("%s %d:read one meter fail\r\n",__FUNCTION__,__LINE__);
			*pTemp++ = 0x00;					//热计量表数据长度为0
			DataLen++;
			Err = NO_ERR;/*yangfei added 2013-11-09*/
		}

	}
	else{  //如果表地址为空，则数据长度设为1，数据为00，按照抄表成功处理，防止补抄。
		*pTemp++ = 1;
		DataLen++;
		*pTemp++ = 0;  //数据为0，与上位机约定，传到服务器解析为表地址为空。
		DataLen++;

		ReadMeterSuccess = 1;  //按照成功处理，防止重复补抄。

	}



 	DataBuf[0]	= DataLen;						//数据域长度
 	*pTemp = PUBLIC_CountCS(&DataBuf[1], DataLen);
 	
 	memcpy(EleBuf, DataBuf, DataLen+2);
	
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Timing Read Meter Complete!");
    debug_debug(gDebugModule[TASKDOWN_MODULE],"INFO: <METER_ReadMeterDataTiming> Timing Read Meter Complete!");
	
	return ReadMeterSuccess ? NO_ERR:ERR_1;      
}



/****************************************************************************************************
**	函 数  名 称: VALVE_ReadMeterDataTiming
**	函 数  功 能: 集中器读取下挂阀控器的数据及读取控制
**	输 入  参 数: uint16 MeterSn -- 表序号;
**	输 出  参 数: uint8 *EleBuf -- 接收到的数据帧回应;
**  返   回   值: NO_ERR, ERR_1 -- 没有接收到数据
**	备		  注: 填写下行读取数据的相关数据结构
**  					存储格式与热表存储格式相同。
*****************************************************************************************************/
uint16 gu16ValveIDRecord[METER_NUM_MAX] = {0};  //记录抄阀顺序计量点号，便于下发分摊热量查询存储位置。

uint8 VALVE_ReadMeterDataTiming(uint16 MeterSn, uint8 *EleBuf)
{
	uint8 i				= 0x00;
	uint8 Err 		 	= 0x00;
	uint8 DataLen		= 0x00;
	uint8 DataBuf[128]	= {0x00};
	uint8 ReadMeterSuccess = 0;
	uint8 *pTemp = DataBuf;
	MeterFileType	mf;
	uint8 lu8DataIn[10] = {0};  //在阀控控制时用于传递阀控控制信息。
	uint8 DataFrame[METER_FRAME_LEN_MAX];
	uint8 DataLen_Vave = 0x00;
	uint8 lu8ReadTime[6] = {0};
	Valve_Format ValveData;//阀门数据。
	uint16 lu16tmp = 0;
	uint8 lu8tmp = 0;
	int8 ls8tmp = 0;  //有符号8位变量。
	CPU_SR		cpu_sr;

	
	LOG_assert_param(EleBuf == NULL);
	LOG_assert_param(MeterSn > METER_NUM_MAX);

	gu8ReadValveFail = 0;

    Err = PARA_ReadMeterInfo(MeterSn, &mf);
    if(Err != NO_ERR){
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_ReadMeterDataTiming> Read Meter filed failed, MeterSn is %4XH, Return is %d"
    								,MeterSn, Err);
	}
	if(mf.ProtocolVer>PROTOCOL_MAX){
        mf.ProtocolVer = 0;
	}

	gu16ValveIDRecord[gu16ReadValveNum] = mf.MeterID;  //记录MeterID。
	
	pTemp++;	//为数据长度预留
	memcpy(pTemp,(uint8 *)&(mf.MeterID), 2);  //MeterID.
	pTemp += 2;
	DataLen = 2;
	*pTemp = mf.EquipmentType;  //设备类型。
	pTemp += 1;
	DataLen += 1;
	memcpy(pTemp, mf.ValveAddr, 7); //抄阀则放阀门的地址。
	pTemp += 7;
	DataLen += 7;
	*pTemp = mf.BulidID;  //楼号。
	pTemp += 1;
	DataLen += 1;
	*pTemp = mf.UnitID;  //单元号。
	pTemp += 1;
	DataLen += 1;
	memcpy(pTemp,(uint8 *)&(mf.RoomID), 2); //房间号
	pTemp += 2;
	DataLen += 2;

	ReadDateTime(lu8ReadTime);
	memcpy(pTemp,lu8ReadTime, 3); //加入抄阀时间当前时间，只放时、分、秒。
	pTemp += 3;
	DataLen += 3;

	if(MeterNoBcdCheck(mf.ValveAddr) == TRUE){  //只有表地址不为空时才执行。

		if(mf.ValveProtocal == VALVE_ADE){  //航天德鲁协议。
			Err = ValveContron(&mf,READ_VALVEDATA,lu8DataIn,DataFrame,&DataLen_Vave);
			if(Err == NO_ERR){
				lu16tmp = DataFrame[3];
				lu16tmp = lu16tmp * 100 / 60; //分钟转换为小时并乘以100.
				DataFrame[3] = HexToBcd((uint8)lu16tmp);  //转换成BCD码。
				memcpy((uint8 *)(&ValveData.ValveOpenTime),&DataFrame[3],4);	
				memcpy((uint8 *)(&ValveData.WaterInTemp),&DataFrame[14],2);	
				memcpy((uint8 *)(&ValveData.WaterOutTemp),&DataFrame[17],2);	
				memcpy((uint8 *)(&ValveData.RoomTemp),&DataFrame[20],2);	

				ValveData.RoomCompensateTemp = 0;  //航天德鲁阀控没有室内温度设定、补偿值传回，写为0.
				ValveData.RoomSetTemp = 0;

				//报警代码待定后加入
			}
			else{
				ReadMeterSuccess = 0;
				*pTemp++ = 0x00;					
				DataLen++;
				goto ReadOver;

			}

		}
		else{	//其他厂家阀控协议。		
			Err = ValveContron(&mf,READVALVE_OPENTIME,lu8DataIn,DataFrame,&DataLen_Vave);
			if(Err == NO_ERR){
				lu16tmp = DataFrame[3];
				lu16tmp = lu16tmp * 100 / 60; //分钟转换为小时并乘以100.
				DataFrame[3] = HexToBcd((uint8)lu16tmp);  //转换成BCD码。
				memcpy((uint8 *)(&ValveData.ValveOpenTime),&DataFrame[3],4);	
				
			}
			else{ //失败处理
				ReadMeterSuccess = 0;
				*pTemp++ = 0x00;					
				DataLen++;
				goto ReadOver;
			}

			Err = ValveContron(&mf,READVALVE_WATERTEMP,lu8DataIn,DataFrame,&DataLen_Vave);
			if(Err == NO_ERR){
				 ValveData.WaterInTemp = HexToBcd(DataFrame[5]);//进水温度整数位
				 ValveData.WaterInTemp = (ValveData.WaterInTemp<<8) + HexToBcd(DataFrame[6]);// 加上进水温度小数位。
				 ValveData.WaterOutTemp = HexToBcd(DataFrame[7]);//进水温度整数位
				 ValveData.WaterOutTemp = (ValveData.WaterOutTemp<<8) + HexToBcd(DataFrame[8]);// 加上进水温度小数位。
				
			}
			else{  //失败处理
				ReadMeterSuccess = 0;
				*pTemp++ = 0x00;					
				DataLen++;
				goto ReadOver;
			}

			Err = ValveContron(&mf,ReadVALVE_All,lu8DataIn,DataFrame,&DataLen_Vave);
			if(Err == NO_ERR){
				ls8tmp = (int8)DataFrame[4]; //房间温度需要加上有符号补偿温度.
				ls8tmp +=  DataFrame[8];
				lu8tmp = (uint8)ls8tmp;
				ValveData.RoomTemp = HexToBcd(lu8tmp);  //房间温度，加小数位。
				lu8tmp = DataFrame[7] & 0xf0;
				if(lu8tmp> 0x90)
					lu8tmp = 0x90;
				ValveData.RoomTemp = (ValveData.RoomTemp<<8) + lu8tmp;
				
				ValveData.RoomSetTemp = DataFrame[5] & 0x3f;  //房间设定温度,hex.
				ValveData.RoomCompensateTemp = DataFrame[4];  //房间补偿温度,hex。

				//下面是位标志赋值。
				if(DataFrame[3] & 0x01)
					ValveData.Bit_OnOffState = 1;
				else
					ValveData.Bit_OnOffState = 0;

				if(DataFrame[3] & 0x04)
					ValveData.Bit_PanelLockFlag	= 1;	
				else
					ValveData.Bit_PanelLockFlag	= 0;

				if(DataFrame[7] & 0x01)
					ValveData.Bit_ValveState = 1;
				else			
					ValveData.Bit_ValveState = 0;

				if(DataFrame[3] & 0x02)
					ValveData.Bit_LockOpenFlag = 1;	
				else
					ValveData.Bit_LockOpenFlag = 0;		

				if(DataFrame[3] & 0x80)
					ValveData.Bit_LockCloseFlag = 1;		
				else
					ValveData.Bit_LockCloseFlag = 0;		

				if(DataFrame[7] & 0x02)
					ValveData.Bit_WirelessState = 1;		
				else if((ValveData.RoomTemp < 0x0100) || (ValveData.RoomTemp > 0x9000)) //如果温度不在1-90℃，则认为无线异常。
					ValveData.Bit_WirelessState = 1;
				else
					ValveData.Bit_WirelessState = 0;

				if(DataFrame[5] & 0x40)
					ValveData.Bit_RechargeFlag = 1;	
				else
					ValveData.Bit_RechargeFlag = 0;	

				if(DataFrame[5] & 0x80)
					ValveData.Bit_ArrearageFlag = 1;		
				else
					ValveData.Bit_ArrearageFlag = 0;	

				
				
				
				
			}
			else{  //失败处理
				ReadMeterSuccess = 0;
				*pTemp++ = 0x00;					
				DataLen++;
				goto ReadOver;
			}

		}

		//把抄到的阀门数据加入。
		*pTemp++ = sizeof(ValveData);
		DataLen++;
		memcpy(pTemp,(uint8 *)&ValveData,sizeof(ValveData));
		pTemp += sizeof(ValveData);
		DataLen += sizeof(ValveData);
		
		ReadMeterSuccess = 1;
		

	}
	else{  //如果表地址为空，则数据长度设为1，数据为00，按照抄表成功处理，防止补抄。
		*pTemp++ = 1;
		DataLen++;
		*pTemp++ = 0;  //数据为0，与上位机约定，传到服务器解析为表地址为空。
		DataLen++;

		ReadMeterSuccess = 1;  //按照成功处理，防止重复补抄。

	}


ReadOver:

	if(1 != ReadMeterSuccess){  //如果抄表成功，则ReadCmplNums++。
		gu8ReadValveFail = 1;//标记抄阀失败。
	}



 	
 	DataBuf[0]	= DataLen;						//数据域长度
 	*pTemp = PUBLIC_CountCS(&DataBuf[1], DataLen);
 	
 	memcpy(EleBuf, DataBuf, DataLen+2);
	
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Timing Read Meter Complete!");
    debug_debug(gDebugModule[TASKDOWN_MODULE],"INFO: <METER_ReadMeterDataTiming> Timing Read Meter Complete!");
	return ReadMeterSuccess ? NO_ERR:ERR_1;      
}


/****************************************************************************************************
**	函 数  名 称: METER_ReadMeterDataCur
**	函 数  功 能: 集中器读取下挂热计量表的数据  实时读取数据
**	输 入  参 数: DELU_Protocol *pReadMeter -- 德鲁协议数据; 
                  uint16 MeterSn -- 表序号
**	输 出  参 数: 
**  返   回   值: NO_ERR, ERR_1 -- 没有接收到数据
**	备		  注: 
*****************************************************************************************************/
uint8 METER_ReadMeterDataCur(DELU_Protocol *pReadMeter, uint16 MeterSn)
{
	uint8 i				= 0x00;
	uint8 Err 		 	= 0x00;
	uint8 RetryTimes 	= 0x02;
	uint8 DataLen		= 0x00;
	uint8 DataBuf[128]	= {0x00};

	MeterFileType	mf;
    
    LOG_assert_param(pReadMeter == NULL);
	LOG_assert_param(MeterSn > METER_NUM_MAX);
    
    Err = PARA_ReadMeterInfo(MeterSn, &mf);
    if(Err != NO_ERR)
    	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_ReadMeterDataCur> Read Meter filed failed, MeterSn is %4XH, Return is %d"
    								,MeterSn, Err);
    	}
	
	METER_ChangeChannel(mf.ChannelIndex);


	//内容待添加。
	
	return Err;      
}
uint8 Get_Current_Channel(void)
{
  return gCurrent_Channel;
}

void Reset_Current_Channel(void)
{
  gCurrent_Channel = 0;
}

/****************************************************************************************************
**	函 数  名 称: DisableAllMBusChannel
**	函 数  功 能: 
**	输 入  参 数: 
**	输 出  参 数: 
**  返   回   值: 
**	备		  注: 
*****************************************************************************************************/
void DisableAllMBusChannel(void)
{
	DISABLE_MBUS();

}

/****************************************************************************************************
**	函 数  名 称: METER_ChangeChannel
**	函 数  功 能: 集中器控制切换MBUS通道 1--6
**	输 入  参 数: uint8 Channel -- 通道标志
**	输 出  参 数: 
**  返   回   值: NO_ERR
**	备		  注: 
*****************************************************************************************************/

uint8 METER_ChangeChannel(uint8 Channel)
{
	uint8 lu8ChannelChangeFlag = 0; //标记通道是否变化，0-没变，1-变化。
	
	LOG_assert_param(Channel<1);
	LOG_assert_param(Channel>7);

	FeedTaskDog();

	gu8NowMBUSChannel = Channel;

    switch(Channel){
        case 1:			
			if(gCurrent_Channel != 1){
                 ENABLE_MBUS_1();
				 gDownCommDev485 = DOWN_COMM_DEV_MBUS;
				 lu8ChannelChangeFlag = 1;
			}
						
         break;
        
        case 2:	
			if(gCurrent_Channel != 2){
				ENABLE_MBUS_2();
				gDownCommDev485 = DOWN_COMM_DEV_MBUS;
				lu8ChannelChangeFlag = 1;
			}
						
        break;
		
        case 3:
			if(gCurrent_Channel != 3){
            	ENABLE_MBUS_3();
				gDownCommDev485 = DOWN_COMM_DEV_MBUS;
				lu8ChannelChangeFlag = 1;
			}
						
        break;
		
        case 4:	
			if(gCurrent_Channel != 4){
	        	ENABLE_MBUS_4();
				gDownCommDev485 = DOWN_COMM_DEV_MBUS;
				lu8ChannelChangeFlag = 1;
			}
						
        break;
		
        case 5:
			if(gCurrent_Channel != 5){
            	ENABLE_MBUS_5();
				gDownCommDev485 = DOWN_COMM_DEV_MBUS;
				lu8ChannelChangeFlag = 1;

			}
						
        break;
		
        case 6:
			if(gCurrent_Channel != 6){
            	ENABLE_MBUS_6();
				gDownCommDev485 = DOWN_COMM_DEV_MBUS;
				lu8ChannelChangeFlag = 1;
			}
			
        break;
            		
        case 7:{
            DISABLE_MBUS();
			gDownCommDev485 = DOWN_COMM_DEV_485;  //第7通道，固定为485总线。
        	}
			
        break;
            	
        default:
			gCurrent_Channel = 0;
               break;


			   
	}	
	
     //LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ChangeChannel> Change MBUS Channel Successful! Channel is %d", Channel);
		 if(lu8ChannelChangeFlag == 1){  //通道发生变化时延时待电气稳定。
		  		OSTimeDly(OS_TICKS_PER_SEC);/* 通道切换延时太短导致表无法成功*/
		  }
		 
	 FeedTaskDog();

    
	 return NO_ERR; 
}

/****************************************************************************************************
**	函 数  名 称: METER_DataItem
**	函 数  功 能: 抄读热计量表，阀门，温控面板等具体的数据项
**	输 入  参 数: DELU_Protocol *pProtocoalInfo -- 抄表的参数输入
**	输 出  参 数: DELU_Protocol *pProtocoalInfo -- 输出数据复用
**  返   回   值: NO_ERR -- 抄表成功; 1 -- 抄表未成功，超时
**	备		  注: 
*****************************************************************************************************/
uint8 METER_DataItem(DELU_Protocol *pProtocoalInfo)
{
	uint8 i 					= 0;
	uint8 Err					= 0;
	/*begin:yangfei modified 2013-10-14 for 减少重试次数*/
	//uint8 RetryTimes			= 2;
	/*end:yangfei modified 2013-10-14 for 减少重试次数*/
	char MeterAddrString[40] 	= {0x00};
	uint8 DataTemp[128]			= {0x00};
	uint8 LenTemp				= 0;
	uint8 lu8ReadMeterTimes = 0; //最多抄表次数。
	uint16 lu16Second = 0;  //秒钟
	uint16 lu16ms = 0;      //毫秒
	
	LOG_assert_param(pProtocoalInfo == NULL);

	lu8ReadMeterTimes = gPARA_ReplenishPara.MeterReplenishTimes + 1;  //共抄表次数。
	lu16Second = gPARA_ReplenishPara.MeterInterval / 1000;   
	lu16ms = gPARA_ReplenishPara.MeterInterval % 1000;

	PUBLIC_MeterAddrToString(pProtocoalInfo->MeterAddr, MeterAddrString,7);
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Reading Meter Addr is %s", MeterAddrString);
	
 	for(i=0; i<lu8ReadMeterTimes; i++)
 	{
 		Err = METER_MeterCommunicate(pProtocoalInfo, DataTemp, &LenTemp);
 		if(Err == NO_ERR){
 				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Recived Valid Datas ");
 				Err = METER_DELU_AnalDataFrame(pProtocoalInfo, DataTemp);
				if(Err == NO_ERR){
						LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Analysis Datas successful!");
						break;
					}
				else{
						LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Analysis Datas Failed!");
					}
 		}
 		else{
 				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Recvied Datas Failed! RetryIndex is %d", i);
				OSTimeDlyHMSM(0,0,lu16Second,lu16ms);
		}
 	}
	
 	if(i >= lu8ReadMeterTimes){			//重试2次未抄读到数据
 		return 1;
 	}
 	
 	return NO_ERR;
}


/****************************************************************************************************
**	函 数  名 称: METER_DataItem
**	函 数  功 能: 抄读热计量表，阀门，温控面板等具体的数据项
**	输 入  参 数: DELU_Protocol *pProtocoalInfo -- 抄表的参数输入
**	输 出  参 数: DELU_Protocol *pProtocoalInfo -- 输出数据复用
**  返   回   值: NO_ERR -- 抄表成功; 1 -- 抄表未成功，超时
**	备		  注: 
*****************************************************************************************************/
uint8 VALVE_YILIN_DataItem(DELU_Protocol *pProtocoalInfo)
{
	uint8 i 					= 0;
	uint8 Err					= 0;
	/*begin:yangfei modified 2013-10-14 for 减少重试次数*/
	//uint8 RetryTimes			= 3;
	uint8 RetryTimes			= 2;
	/*end:yangfei modified 2013-10-14 for 减少重试次数*/
	char MeterAddrString[40] 	= {0x00};
	uint8 DataTemp[128]			= {0x00};
	uint8 LenTemp				= 0;
	
	LOG_assert_param(pProtocoalInfo == NULL);

	
	PUBLIC_MeterAddrToString(pProtocoalInfo->MeterAddr, MeterAddrString,2);
	
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Reading Meter Addr is %s", MeterAddrString);
	
 	for(i=0; i<RetryTimes; i++)
 	{
 		Err = METER_MeterCommunicate(pProtocoalInfo, DataTemp, &LenTemp);
 		if(Err == NO_ERR)
 			{
 				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Recived Valid Datas ");
 				Err = METER_DELU_AnalDataFrame(pProtocoalInfo, DataTemp);
				if(Err == NO_ERR)
					{
						LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Analysis Datas successful!");
						break;
					}
				else
					{
						LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Analysis Datas Failed!");
					}
 			}
 		else
 			{
 				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Recvied Datas Failed! RetryIndex is %d", i);
 			}
 	}
 	if(i>=RetryTimes)										//重试2次未抄读到数据
 		{
 			return 1;
 		}
 	
 	return NO_ERR;
}

/****************************************************************************************************
**	函 数  名 称: METER_DataItem
**	函 数  功 能: 抄读热计量表，阀门，温控面板等具体的数据项
**	输 入  参 数: DELU_Protocol *pProtocoalInfo -- 抄表的参数输入
**	输 出  参 数: DELU_Protocol *pProtocoalInfo -- 输出数据复用
**  返   回   值: NO_ERR -- 抄表成功; 1 -- 抄表未成功，超时
**	备		  注: 
*****************************************************************************************************/
uint8 METER_MeterDataForValve(DELU_Protocol *pProtocoalInfo, uint8 ProtocolVer)
{
      CJ188_Format  MeterData={0,0x05,0,0x05,0,0x17,0,0x35,0,0x2c};
      uint8 err;
	switch(ProtocolVer)
      {
       /*协议为4(包括4)以上的转化为DELU格式*/
      case 0:  /*DELU*/
          {
            memcpy(&MeterData,pProtocoalInfo->DataBuf,sizeof(MeterData));   /* 德鲁协议完全符合CJ188标准*/
            break;
          }
      case 1:  /*WANHUA*/
          {
            WANHUA_Format  Data;
            uint8* pTemp = (uint8*)&Data;
            uint8 i;
            memcpy(&Data,pProtocoalInfo->DataBuf,sizeof(Data));
            for(i=0;i<sizeof(Data);i++) /*数据域需要减0x33*/
              {
              *pTemp-=0x33;
              pTemp++;
              }
            MeterData.DailyHeat = Data.DailyHeat;
            MeterData.CurrentHeat = Data.CurrentHeat;
            MeterData.HeatPower = Data.HeatPower;
            MeterData.Flow = Data.Flow;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;
            
            break;
          }   
      case 2:  /*DANFOSS*/
          {
            Danfoss_Format  DanfossData;
            memcpy(&DanfossData,pProtocoalInfo->DataBuf,sizeof(DanfossData));
            To_Little_endian((uint8*)DanfossData.DailyHeat,4);
            To_Little_endian((uint8*)DanfossData.CurrentHeat,4);
            To_Little_endian((uint8*)DanfossData.HeatPower,4);
            To_Little_endian((uint8*)DanfossData.Flow,4);
            To_Little_endian((uint8*)DanfossData.AccumulateFlow,4);
            To_Little_endian(DanfossData.WaterInTemp,3);
            To_Little_endian(DanfossData.WaterOutTemp,3);
            To_Little_endian(DanfossData.AccumulateWorkTime,3);
            To_Little_endian(DanfossData.RealTime,7);
            To_Little_endian((uint8*)DanfossData.ST,2);
            memcpy(&MeterData,&DanfossData,sizeof(MeterData));
            break;
          }
      case 3:  /*LICHUANG*/
          {
            LICHUANG_Format  Data;
            memcpy(&Data,pProtocoalInfo->DataBuf,sizeof(Data));
            MeterData.DailyHeat = Data.DailyHeat;
            MeterData.CurrentHeat = Data.CurrentHeat;
            MeterData.HeatPower = Data.HeatPower;
            MeterData.Flow = Data.Flow;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;
            break;
          }
	  default:
          {
           MeterData = METER_Data_To_CJ188Format(ProtocolVer,pProtocoalInfo->DataBuf,pProtocoalInfo->Length-3,&err); 
	       break;
           }
     
	}
	memcpy(pProtocoalInfo->DataBuf,&MeterData,sizeof(MeterData));/*将转化的数据保存*/
	return ProtocolVer;
	#if 0
	if(1 == ProtocolVer) /*wanhua DN20*/
	{
		    /*begin:yangfei modified 2013-08-08 for WAN_HUA format to CJ188 */
		    #if 0
			for(i=0; i<pProtocoalInfo->Length-3; i++)
			{
				pProtocoalInfo->DataBuf[i] = pProtocoalInfo->DataBuf[i] - 0x33;
			}
            #endif
            CJ188_Format  MeterData={0,0x05,0,0x05,0,0x17,0,0x35,0,0x2c};
            WANHUA_Format  Data;
            uint8* pTemp = (uint8*)&Data;
            uint8 i;
            memcpy(&Data,pProtocoalInfo->DataBuf,sizeof(Data));
            for(i=0;i<sizeof(Data);i++) /*数据域需要减0x33*/
              {
                *pTemp-=0x33;
                pTemp++;
              }
            MeterData.DailyHeat = Data.DailyHeat;
            MeterData.CurrentHeat = Data.CurrentHeat;
            MeterData.HeatPower = Data.HeatPower;
            MeterData.Flow = Data.Flow;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
            
            MeterData.AccumulateFlowUnit = Data.AccumulateFlowUnit;/*单位转换，万华单位为29(L)*/
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;

            memcpy(pProtocoalInfo->DataBuf,&MeterData,sizeof(MeterData));/*将转化的数据保存*/
             /*end:yangfei modified 2013-08-08 for WAN_HUA format to CJ188 */
		}
    else if(5 == ProtocolVer)
      {
       /*begin:yangfei modified 2013-08-16 for WAN_HUA DN25 format to CJ188 */
            CJ188_Format  MeterData={0,0x05,0,0x05,0,0x17,0,0x35,0,0x2c};
            WANHUA_Format  Data;
            
            memcpy(&Data,pProtocoalInfo->DataBuf,sizeof(Data));
            
            MeterData.DailyHeat = Data.DailyHeat;
            MeterData.DailyHeatUnit= Data.DailyHeatUnit;
            MeterData.CurrentHeat = Data.CurrentHeat;
            MeterData.CurrentHeatUnit= Data.CurrentHeatUnit;
            MeterData.HeatPower = Data.HeatPower;
            MeterData.HeatPowerUnit= Data.HeatPowerUnit;
            MeterData.Flow = Data.Flow;
            MeterData.FlowUnit= Data.FlowUnit;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
            MeterData.AccumulateFlowUnit = Data.AccumulateFlowUnit;/*单位转换，万华单位为29(L)*/
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;

            memcpy(pProtocoalInfo->DataBuf,&MeterData,sizeof(MeterData));/*将转化的数据保存*/
             /*end:yangfei modified 2013-08-16 for WAN_HUA format to CJ188 */
      }
	else
		{
			
		}
	return ProtocolVer;
	#endif
}


/*
*/
CJ188_Format METER_Data_To_CJ188Format(uint8 ProtocolVer,uint8* DataBuf,uint8 len,uint8* err)
{
    CJ188_Format  MeterData={0,0x05,0,0x05,0,0x17,0,0x35,0,0x2c};
   
    
    switch(ProtocolVer)
      {
      #if 0    /*协议为4(包括4)以上的转化为DELU格式*/
      case 0:  /*DELU*/
          {
            memcpy(&MeterData,DataBuf,sizeof(MeterData));   /* 德鲁协议完全符合CJ188标准*/
            break;
          }
      case 1:  /*WANHUA*/
          {
            WANHUA_Format  Data;
            uint8* pTemp = (uint8*)&Data;
            uint8 i;
            memcpy(&Data,DataBuf,sizeof(Data));
            for(i=0;i<sizeof(Data);i++) /*数据域需要减0x33*/
              {
              *pTemp-=0x33;
              pTemp++;
              }
            MeterData.DailyHeat = Data.DailyHeat;
            MeterData.CurrentHeat = Data.CurrentHeat;
            MeterData.HeatPower = Data.HeatPower;
            MeterData.Flow = Data.Flow;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;
            
            break;
          }   
      case 2:  /*DANFOSS*/
          {
            Danfoss_Format  DanfossData;
            memcpy(&DanfossData,DataBuf,sizeof(DanfossData));
            To_Little_endian((uint8*)DanfossData.DailyHeat,4);
            To_Little_endian((uint8*)DanfossData.CurrentHeat,4);
            To_Little_endian((uint8*)DanfossData.HeatPower,4);
            To_Little_endian((uint8*)DanfossData.Flow,4);
            To_Little_endian((uint8*)DanfossData.AccumulateFlow,4);
            To_Little_endian(DanfossData.WaterInTemp,3);
            To_Little_endian(DanfossData.WaterOutTemp,3);
            To_Little_endian(DanfossData.AccumulateWorkTime,3);
            To_Little_endian(DanfossData.RealTime,7);
            To_Little_endian((uint8*)DanfossData.ST,2);
            memcpy(&MeterData,&DanfossData,sizeof(MeterData));
            break;
          }
      case 3:  /*LICHUANG*/
          {
            LICHUANG_Format  Data;
            memcpy(&Data,DataBuf,sizeof(Data));
            MeterData.DailyHeat = Data.DailyHeat;
            MeterData.CurrentHeat = Data.CurrentHeat;
            MeterData.HeatPower = Data.HeatPower;
            MeterData.Flow = Data.Flow;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;
            
            break;
          }
      #endif
	  
	  case WANHUA_VER:{  //万华协议单位在前，数据在后,WANHUA_VER版本是在上位机处理，这里不做处理。
			memcpy(&MeterData,DataBuf,sizeof(MeterData));
			
			break;
		 }
      case 4:  /*乐业485 LYRB-USF*/
          {
            LEYE485_Format Data;
            memcpy(&Data,DataBuf,sizeof(MeterData));
            memcpy(&MeterData,DataBuf,sizeof(MeterData));
            MeterData.DailyHeat = Data.DailyHeat<<8;
            MeterData.HeatPower = 0;
            MeterData.HeatPowerUnit= 0x17;
            MeterData.CurrentHeat = (Data.CurrentHeat<<8) + Data.CurrentHeatDecimal[1];
            MeterData.AccumulateFlow = (Data.AccumulateFlow<<8) + Data.AccumulateFlowDecimal[2];
            MeterData.WaterInTemp[2] = 0;
            MeterData.WaterOutTemp[2] = 0;
            break;
          }
       case 5:  /*WANHUA 大表*/
          {
            WANHUA_Format  Data;
            
            memcpy(&Data,DataBuf,sizeof(Data));
            
            MeterData.DailyHeat = Data.DailyHeat;
            MeterData.DailyHeatUnit= Data.DailyHeatUnit;
            MeterData.CurrentHeat = Data.CurrentHeat;
            MeterData.CurrentHeatUnit= Data.CurrentHeatUnit;
            MeterData.HeatPower = Data.HeatPower;
            MeterData.HeatPowerUnit= Data.HeatPowerUnit;
            MeterData.Flow = Data.Flow;
            MeterData.FlowUnit= Data.FlowUnit;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
            MeterData.AccumulateFlowUnit = Data.AccumulateFlowUnit;/*单位转换，万华单位为29(L)*/
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;
            
            break;
          } 
      case HYDROMETER775_VER :
          {
            HYDROMETER_TO_CJ188(&MeterData,DataBuf,len);
			 
            break;
          }
      case ZENNER_VER :
          {
            ZENNER_Format  ZENNER_data;
            memcpy(&ZENNER_data,DataBuf,sizeof(ZENNER_data));
	     if(ZENNER_data.Vertion >= 4)
	     	{
		     if(ZENNER_data.Status != 0x03)
		     	{
		       MeterData.ST = (ZENNER_data.Status&0xfc);/*yangfei added for ST*/
		     	LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d ZENNER_data.Status = %x",__FUNCTION__,__LINE__,ZENNER_data.Status);
		     	}
	     	}
	     else    /*b0:电池低, b1:温度传感器坏, b2:流量传感器坏.老的定义V004版本之前*/
	     	{
	     		if(ZENNER_data.Status&BIT0== BIT0){MeterData.ST |= BIT2_LOW_VOLTAGE;}
			if(ZENNER_data.Status&BIT1== BIT1){MeterData.ST |= BIT5_TEMP_SENSOR;}
			if(ZENNER_data.Status&BIT2== BIT2){MeterData.ST |= BIT6_FLOW_SENSOR;}	
	     	}
	      if(ZENNER_data.CurrentHeatUnit[0] != 0x04||ZENNER_data.CurrentHeatUnit[1] != 0x06)
		     	{
		     		
		     	     uint8 unit;
			     unit = ZENNER_data.CurrentHeatUnit[1]&0x07 ;
	                   if(ZENNER_data.CurrentHeatUnit[0] == 0x04)
	                   	{
	                   	  MeterData.CurrentHeat = HexToBcdUint32(ZENNER_data.CurrentHeat*(pow(10,unit-3-3+2))); /*10(nnn-3) Wh 0,001 Wh to 10 000 Wh*//*wh-->>kwh 2:0.01kw*/
	                   	}
		     	    LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d ZENNER_data.CurrentHeatUnit = %x%x",\
					__FUNCTION__,__LINE__,ZENNER_data.CurrentHeatUnit[0],ZENNER_data.CurrentHeatUnit[1]);
		     	}
		  else
		  	{
                        MeterData.CurrentHeat = HexToBcdUint32(ZENNER_data.CurrentHeat*100);
                        debug_info(gDebugModule[METER_DATA],"ZENNER_data.CurrentHeat=%d  MeterData.CurrentHeat = %x  \r\n",ZENNER_data.CurrentHeat,MeterData.CurrentHeat);
		  	}
		if(ZENNER_data.PowerUnit[0] != 0x04||ZENNER_data.PowerUnit[1] != 0x2b)
		     	{
		     	uint8 unit = 0;
		     	unit = ZENNER_data.PowerUnit[1]&0x07;
		     	if(ZENNER_data.PowerUnit[0] == 0x04)
		     		{
		     		MeterData.HeatPower = HexToBcdUint32(ZENNER_data.Power*(pow(10,unit-4)));/**/
		     		}
		     	LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d ZENNER_data.PowerUnit = %x%x",\
					__FUNCTION__,__LINE__,ZENNER_data.PowerUnit[0],ZENNER_data.PowerUnit[1]);
		     	} 
		else
			{
			
                         MeterData.HeatPower = HexToBcdUint32(ZENNER_data.Power/10);
                         debug_info(gDebugModule[METER_DATA],"ZENNER_data.Power=%d  MeterData.HeatPower = %x  \r\n",ZENNER_data.Power,MeterData.HeatPower);
      	              }
	     if(ZENNER_data.FlowRateUnit[0] != 0x04||ZENNER_data.FlowRateUnit[1] != 0x3b)
		     	{
		     	 uint8 unit = 0;
		     	 unit = ZENNER_data.FlowRateUnit[1]&0x07;
			if(ZENNER_data.FlowRateUnit[0] == 0x04)
	     		{
	     		MeterData.Flow = HexToBcdUint32(ZENNER_data.FlowRate*(pow(10,unit-2))); /*E011 1nnn Volume Flow 10(nnn-6) m3/h*/
	     		}
		     	LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d ZENNER_data.FlowRateUnit = %x%x",\
					__FUNCTION__,__LINE__,ZENNER_data.FlowRateUnit[0],ZENNER_data.FlowRateUnit[1]);
		     	}
		 else
		 	{
            			MeterData.Flow = HexToBcdUint32(ZENNER_data.FlowRate*10);
            			debug_info(gDebugModule[METER_DATA],"ZENNER_data.FlowRate=%d  MeterData.Flow = %x  \r\n",ZENNER_data.FlowRate,MeterData.Flow);
		 	}
	     if(ZENNER_data.VolumeUnit[0] != 0x04||ZENNER_data.VolumeUnit[1] != 0x14)
		     	{
		     	uint8 unit = 0;
		     	 unit = ZENNER_data.VolumeUnit[1]&0x07;
			if(ZENNER_data.VolumeUnit[0] == 0x04)
	     		{
	     		MeterData.AccumulateFlow= HexToBcdUint32(ZENNER_data.Volume*(pow(10,unit-4)));/*E001 0nnn Volume 10(nnn-6) m3*/
	     		}
		     	LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d ZENNER_data.VolumeUnit = %x%x",\
					__FUNCTION__,__LINE__,ZENNER_data.VolumeUnit[0],ZENNER_data.VolumeUnit[1]);
		     	}
		 else
		 	{
                      MeterData.AccumulateFlow= HexToBcdUint32(ZENNER_data.Volume);
                      debug_info(gDebugModule[METER_DATA],"ZENNER_data.Volume=%d  MeterData.AccumulateFlow = %x  \r\n",ZENNER_data.Volume,MeterData.AccumulateFlow);
		 	}
            {
			 if(ZENNER_data.FlowTemperatureUnit[0] != 0x02||ZENNER_data.FlowTemperatureUnit[1] != 0x59)
		     	{
		     		uint8 unit = 0;
			     	 unit = ZENNER_data.FlowTemperatureUnit[1]&0x03;
			     	if(ZENNER_data.FlowTemperatureUnit[0] == 0x02)/*10(nn-3) °C*/
			     	{
			     	  uint32 Temperature;
                           Temperature = HexToBcdUint32(ZENNER_data.FlowTemperature*(pow(10,unit-3+2)));
                            memcpy(MeterData.WaterInTemp,(char*)&Temperature,3);
			     	}
		     	LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d ZENNER_data.FlowTemperatureUnit = %x%x",\
					__FUNCTION__,__LINE__,ZENNER_data.FlowTemperatureUnit[0],ZENNER_data.FlowTemperatureUnit[1]);
		     	}
		 else
		 	{
		              uint32 Temperature;
		              Temperature = HexToBcdUint32(ZENNER_data.FlowTemperature);/*0.1 °C*/
		              memcpy(MeterData.WaterInTemp,(char*)&Temperature,3);
		              debug_info(gDebugModule[METER_DATA],"ZENNER_data.FlowTemperature=%d Temperature = %x  \r\n",ZENNER_data.FlowTemperature,Temperature);
		       }
		  if(ZENNER_data.ReturnTemperatureUnit[0] != 0x02||ZENNER_data.ReturnTemperatureUnit[1] != 0x5d)
		     	{
			     	uint8 unit = 0;
			     	 unit = ZENNER_data.ReturnTemperatureUnit[1]&0x03;
			     	if(ZENNER_data.ReturnTemperatureUnit[0] == 0x02)/*10(nn-3) °C*/
			     	{
			     	  uint32 Temperature;
                              Temperature = HexToBcdUint32(ZENNER_data.ReturnTemperature*(pow(10,unit-3+2)));
                       	  memcpy(MeterData.WaterOutTemp,(char*)&Temperature,3);
			     	}
			     	LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d ZENNER_data.ReturnTemperatureUnit = %x%x",\
						__FUNCTION__,__LINE__,ZENNER_data.ReturnTemperatureUnit[0],ZENNER_data.ReturnTemperatureUnit[1]);
		     	}
		  else
		  	{
		  	   uint32 Temperature;
                       Temperature = HexToBcdUint32(ZENNER_data.ReturnTemperature);/*0.1 °C*/
                     //MeterData.WaterInTemp = HexToBcdUint32(ZENNER_data.FlowTemperature)<<8;
                        memcpy(MeterData.WaterOutTemp,(char*)&Temperature,3);
                      debug_info(gDebugModule[METER_DATA],"ZENNER_data.ReturnTemperature=%d Temperature = %x  \r\n",ZENNER_data.ReturnTemperature,Temperature);
		  	}
            }

            {
              uint32 WorkTime=0;
              WorkTime =  HexToBcdUint32(ZENNER_data.Operating_Time);
              memcpy(MeterData.AccumulateWorkTime,(char*)&WorkTime,3);
              debug_info(gDebugModule[METER_DATA],"ZENNER_data.Operating_Time=%d WorkTime = %x  \r\n",ZENNER_data.Operating_Time,WorkTime);
              //MeterData.AccumulateWorkTime= HexToBcdUint32(ZENNER_data.Operating_Time)<<8;
            }
            {/*添加实时时间*/
                uint8 SystemTime[6] = {0}; 
                int i=0;
            	memcpy(SystemTime, gSystemTime, 6);
               
                 MeterData.RealTime[6] = 0x20;
                  for(i=0;i<6;i++)
                  {
                  MeterData.RealTime[i] = HexToBcd(SystemTime[i]);
                  debug_info(gDebugModule[METER_DATA]," MeterData.RealTime[%d] = %x \r\n",i,MeterData.RealTime[i] );
                  }
                 
            }
            break;
          }
	  case LANDISGYR_VER:
	    {
            HYDROMETER_TO_CJ188(&MeterData,DataBuf,len);   
        }
	  	break;
	 case ENGELMANN_VER:
	    {
            ENLEMAN_TO_CJ188(&MeterData,DataBuf,len); 
        }
	  	break;
		
	 case ZHENYU_VER_2:
	   {
	   		ZHENYU2_Format  Data;         
			
			memcpy(&Data,DataBuf,sizeof(MeterData));
            //memcpy(&MeterData,DataBuf,sizeof(MeterData));		 
           
		    MeterData.DailyHeat = Data.DailyHeat;	  //结算日热量
		     MeterData.DailyHeatUnit= Data.DailyHeatUnit;
            MeterData.CurrentHeat = Data.CurrentHeat;	 //当前热量
             MeterData.CurrentHeatUnit= Data.CurrentHeatUnit;
            MeterData.HeatPower = Data.HeatPower;	    //热功率
             MeterData.HeatPowerUnit= Data.HeatPowerUnit;
            MeterData.Flow = Data.Flow;		 //瞬时流量	 -------两种力创表的不同点
			//MeterData.Flow =   MeterData.Flow << 4;
			 MeterData.FlowUnit = Data.FlowUnit;
            MeterData.AccumulateFlow = Data.AccumulateFlow;	  //结算日累计流量
             MeterData.AccumulateFlowUnit= Data.AccumulateFlowUnit;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);			//进水温度
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);			 //回水温度
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
		    MeterData.RealTime[6]  = 0x20;

            MeterData.ST = Data.ST;
			
	
            break;
	   
	   }
	 case JINGWEI_VER:	  //经纬热量表
	     {
		    JingWeiXiping_Format   Data;	  //万华组帧格式与海威茨的组帧格式一样，但协议不兼容：硬件协议与解析协议
    
            memcpy(&Data,DataBuf,sizeof(Data));

            MeterData.DailyHeat = Data.DailyHeat;	//结算日热量
            MeterData.DailyHeatUnit= Data.DailyHeatUnit;
            MeterData.CurrentHeat = Data.CurrentHeat;
			MeterData.CurrentHeatUnit= Data.CurrentHeatUnit;
            MeterData.HeatPower = Data.HeatPower;	//热功率
            MeterData.HeatPowerUnit= Data.HeatPowerUnit;
            MeterData.Flow = Data.Flow;	//瞬时流量
            MeterData.FlowUnit = Data.FlowUnit;	
            MeterData.AccumulateFlow = Data.AccumulateFlow;	  //结算日累计流量
            MeterData.AccumulateFlowUnit= Data.AccumulateFlowUnit;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);	//进水温度
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);		  //回水温度
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);	  //累计工作时间
            memcpy(MeterData.RealTime,Data.RealTime,7);		//实时时间
            MeterData.ST = Data.ST;
            
            break;
		 }
	  case JINGWEIFE_VER:	  //经纬热量表
	     {
		 	JingWeiFE_Format  Data;
			memcpy(&Data,DataBuf,sizeof(Data));

            MeterData.DailyHeat = Data.DailyHeat;	//结算日热量

			MeterData.DailyHeat =	 MeterData.DailyHeat << 8;
            MeterData.CurrentHeat = Data.CurrentHeat;

			MeterData.CurrentHeat =	 MeterData.CurrentHeat << 8;

		    MeterData.HeatPower = 0x00;// Data.BiaoHao;	//表号 占用，直接赋值为0
           // MeterData.Flow = Data.Flow;	//瞬时流量	//标识	占用，直接赋值为0

            MeterData.Flow = Data.Flow;		 //瞬时流量	 -------两种力创表的不同点
			MeterData.Flow =   MeterData.Flow << 4;

            MeterData.AccumulateFlow = Data.AccumulateFlow;	  //结算日累计流量
			 MeterData.AccumulateFlow =   MeterData.AccumulateFlow << 8;

            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);	//进水温度
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);		  //回水温度
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);	  //累计工作时间
            memcpy(MeterData.RealTime,Data.RealTime,7);		//实时时间
            MeterData.ST = Data.ST;
            
            break;
		 
		 }

	  case LI_CHUANG_2:
	     {


		     LICHUANG_Format  Data;
         
			
			memcpy(&Data,DataBuf,sizeof(MeterData));
            //memcpy(&MeterData,DataBuf,sizeof(MeterData));		 
           
		    MeterData.DailyHeat = Data.DailyHeat;	  //结算日热量
		    MeterData.DailyHeatUnit = Data.DailyHeatUnit;
            MeterData.CurrentHeat = Data.CurrentHeat;	 //当前热量
            MeterData.CurrentHeatUnit= Data.CurrentHeatUnit;
            MeterData.HeatPower = Data.HeatPower;	    //热功率
            MeterData.HeatPowerUnit = Data.HeatPowerUnit;
            MeterData.Flow = Data.Flow;		 //瞬时流量	 -------两种力创表的不同点
           	MeterData.Flow =   MeterData.Flow << 4;
            MeterData.AccumulateFlow = Data.AccumulateFlow;	  //结算日累计流量
            MeterData.AccumulateFlowUnit= Data.AccumulateFlowUnit;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);			//进水温度
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);			 //回水温度
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;
			
	
            break;
		 }


		 case JICHENG_VER:
		  {
	 
			 JICHENG_Format  Data;	//积成热表协议组帧
			 memcpy(&Data,DataBuf,(sizeof(MeterData)-1));	 //积成的状态位相比较CJ188来说，是一个字节，而CJ188是2个字
			 
			 //Data.DailyHeat.gp22_u8[0] =	DataBuf[17]  ;
			 //Data.DailyHeat.gp22_u8[0] = 0x55  ;
			 //Data.DailyHeat.gp22_u8[1] =	  0x75	;
			 //Data.DailyHeat.gp22_u8[2] =	  0x0f	;
			 //Data.DailyHeat.gp22_u8[3] =	0x44  ;
	 
			  Data.DailyHeat.gp22_u8[0] = DataBuf[3] ;
			 Data.DailyHeat.gp22_u8[1] =  DataBuf[2]  ;
			 Data.DailyHeat.gp22_u8[2] =  DataBuf[1]  ;
			 Data.DailyHeat.gp22_u8[3] =  DataBuf[0]  ;
	 
			 MeterData.DailyHeat = (uint32)Data.DailyHeat.gp22_float;  //当日结算热
			 MeterData.DailyHeat =	  HexToBcdUint32 (MeterData.DailyHeat);
	 
			 
			  MeterData.CurrentHeat = Data.CurrentHeat;   //当前热量
			  MeterData.HeatPower = Data.HeatPower;   //热功率
			  MeterData.Flow = Data.Flow; //瞬时流量
			  MeterData.AccumulateFlow = Data.AccumulateFlow;	//结算日累计流量
			  memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);   //进水温度
			  memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);		//回水温度
			  memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);	//累计工作时间
			  //memcpy(MeterData.RealTime,Data.RealTime,7); 	  //实时时间
			  MeterData.RealTime[0]  = Data.RealTime[6];
			  MeterData.RealTime[1]  = Data.RealTime[5];
			  MeterData.RealTime[2]  = Data.RealTime[4];
			  MeterData.RealTime[3]  = Data.RealTime[3];
			  MeterData.RealTime[4]  = Data.RealTime[2];
	 
			  MeterData.RealTime[5]  = Data.RealTime[1];
			  MeterData.RealTime[6]  = Data.RealTime[0];
	 
			  MeterData.ST = 1;
			  
			  break;
		  }



	 case YITONGDA_VER:
	 {
		 	YITONGDA_Format Data;
			uint32 temp = 0;
			memcpy(&Data,DataBuf,sizeof(Data));	 
			temp = Data.CurrentCool;  //亿通达将当前累计热量放到了当前冷量位置。
			temp = temp << 8;  //薛城用亿通达热量表对邋CJ188有改动，当前热量无小数位。
		    MeterData.DailyHeat = temp;	  //结算日热量,亿通达当前热量就是总热量。
		    MeterData.DailyHeatUnit = 0x05;
            MeterData.CurrentHeat = temp;	 //当前热量
            MeterData.CurrentHeatUnit = 0x05;
            //MeterData.HeatPower = Data.HeatPower;	    //亿达通热表没有热功率
            MeterData.HeatPower = 0;
			MeterData.HeatPowerUnit = 0x17;

			temp = Data.Flow;
			temp = temp << 4; //薛城用亿通达热量表对邋CJ188有改动，流速3位小数位。
            MeterData.Flow = temp;		 //瞬时流
            MeterData.FlowUnit = 0x35;
			temp = Data.AccumulateFlow;
			temp = temp << 4;//薛城用亿通达热量表对邋CJ188有改动，累计流量1位小数位。
            MeterData.AccumulateFlow = temp;	  //结算日累计流量
            MeterData.AccumulateFlowUnit = 0x2c;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);			//进水温度
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);			 //回水温度
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);

            MeterData.ST = Data.ST;

		break;
		}

	 	case LIANQIANG_VER485:  /*联强，嘉洁能的485表的协议组帧与乐业485组帧格式一样，但硬件通道不同*/
          {
            LEYE485_Format Data;
            memcpy(&Data,DataBuf,sizeof(MeterData));
            memcpy(&MeterData,DataBuf,sizeof(MeterData));
            MeterData.DailyHeat = Data.DailyHeat<<8;
            MeterData.HeatPower = 0;		  //其协议没有热功率这一项
            MeterData.HeatPowerUnit= 0x17;
            MeterData.CurrentHeat = (Data.CurrentHeat<<8) + Data.CurrentHeatDecimal[1];
            MeterData.AccumulateFlow = (Data.AccumulateFlow<<8) + Data.AccumulateFlowDecimal[2];
            MeterData.WaterInTemp[2] = 0;
            MeterData.WaterOutTemp[2] = 0;
            break;
          }

		 case LIANQIANG_VERMBUS:  /*联强，嘉洁能的485表的协议组帧与乐业485组帧格式一样，但硬件通道不同*/
          {
            LEYE485_Format Data;
            memcpy(&Data,DataBuf,sizeof(MeterData));
            memcpy(&MeterData,DataBuf,sizeof(MeterData));
            MeterData.DailyHeat = Data.DailyHeat<<8;
            MeterData.HeatPower = 0;
            MeterData.HeatPowerUnit= 0x17;
            MeterData.CurrentHeat = (Data.CurrentHeat<<8) + Data.CurrentHeatDecimal[1];
            MeterData.AccumulateFlow = (Data.AccumulateFlow<<8) + Data.AccumulateFlowDecimal[2];
            MeterData.WaterInTemp[2] = 0;
            MeterData.WaterOutTemp[2] = 0;
            break;
          }

		 case WANHUA_TO_DELU_VER:{  //万华协议单位在前，数据在后。
			WANHUA_Format  Data;
            uint8* pTemp = (uint8*)&Data;
            uint8 i = 0;
            memcpy(&Data,DataBuf,sizeof(Data));
            for(i=0;i<sizeof(Data);i++){  //天津万华解析时需要减去0x33.
              *pTemp -= 0x33;
              pTemp++;
            }
            MeterData.DailyHeat = Data.DailyHeat;
			MeterData.DailyHeatUnit = Data.DailyHeatUnit;
            MeterData.CurrentHeat = Data.CurrentHeat;
			MeterData.CurrentHeatUnit= Data.CurrentHeatUnit;
            MeterData.HeatPower = Data.HeatPower;
			MeterData.HeatPowerUnit= Data.HeatPowerUnit;
            MeterData.Flow = Data.Flow;
			MeterData.FlowUnit= Data.FlowUnit;
            MeterData.AccumulateFlow = Data.AccumulateFlow;
			MeterData.AccumulateFlowUnit= Data.AccumulateFlowUnit;
            memcpy(MeterData.WaterInTemp,Data.WaterInTemp,3);
            memcpy(MeterData.WaterOutTemp,Data.WaterOutTemp,3);
            memcpy(MeterData.AccumulateWorkTime,Data.AccumulateWorkTime,3);
            memcpy(MeterData.RealTime,Data.RealTime,7);
            MeterData.ST = Data.ST;

			break;
		 }
		 	

	
      default:  //海威茨，开元,hx,nl热量表完全符合本协议，不必增加协议解析---林晓彬
          {
           memcpy(&MeterData,DataBuf,sizeof(MeterData));   /* 德鲁协议完全符合CJ188标准*/ 
		
          }
        break;
      }
    
   return MeterData; 
}
/*
**	函 数  名 称: To_Little_endian
**	函 数  功 能: 大端转小端函数
**	输 入  参 数: uint8* Databuff
**	输 出  参 数: uint8* Databuff
**  返   回   值: NO_ERR 
**	备		  注: 
*/
uint8 To_Little_endian(uint8* Databuff,uint8 size)
{
    int i = 0;
    uint8 temp;
    for(i=0;i<size/2;i++)
      {
      temp = Databuff[i];
      Databuff[i] = Databuff[size-i-1];
      Databuff[size-i-1] = temp;
      }
    return NO_ERR;
}


uint8   HYDROMETER_TO_CJ188(CJ188_Format  *MeterData,uint8* DataBuf,uint8 len)
{
	 uint8 data[256]={0};
	 uint8 i=0;
	 ST_DIF  DIF = {0};
	 ST_DIFE  DIFE = {0};
	 ST_VIF  VIF = {0};
	 ST_VIFE VIFE = {0};
     //uint8   VIFE_Index =0;
	 
	 memcpy(data,DataBuf,len);

	 for(i=15;i<len;)
	 {
		DIF = *(ST_DIF *)(&data[i++]);
		if(DIF.Extension == 1)
			{
				   do
					{
						DIFE = *(ST_DIFE*)(&data[i++]);
					}while(DIFE.Extension == 1); /*DIFE:0 … 10 (1 Byte each)*/
				
			}
		VIF = *(ST_VIF*)(&data[i++]);
        //VIFE_Index = 0;
		if(VIF.Extension == 1)
			{
				   do
					{
						VIFE = *(ST_VIFE*)(&data[i++]);
					}while(VIFE.Extension == 1); /*VIFE:0 … 10 (1 Byte each)*/
				
			}
        if(DIF.Extension||DIF.LSB)
          {
           i += DIF.Data;
           continue;
          }
		if(DIF.Function == 0)/*Instantaneous value*/
			{
                if(VIF.Data>=0&&VIF.Data<=0X07)/*E000 0nnn Energy 10(nnn-3) Wh 0,001 Wh to 10 000 Wh*/
                  {
                      uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      uint32 HEX_Data = 0;
                      uint8 unit_add = 0;
                      unit = VIF.Data&0x07;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                        }
                      if(unit == 7)
                        {
                        if(HEX_Data >= 99999999)/*BCD 太大*/
                          {
                          HEX_Data = HEX_Data/100;   /*MWH ->100MWH*/
                          unit_add = 2;
                          }
                          if(HEX_Data)
                            {
                            BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                            }
                          MeterData->CurrentHeat= BCD_Data>>4*abs(unit-3-6+2);/*0.01 MWh*/
                          MeterData->CurrentHeatUnit = MWH+unit_add;
                        }
                      else if(unit>=4)
                       {
                         if(HEX_Data >= 99999999)/*BCD 太大*/
                          {
                          HEX_Data = HEX_Data/1000;   /*MWH ->100MWH*/
                          unit_add = 3;
                          }
                         if(HEX_Data)
                          {
                          BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                          }
                          if((BCD_Data > 0x999999)&&(unit-3-3+2>=2))
                            {
                             BCD_Data>>=3*4;
                             unit_add = 3;
                            }
                           else if((BCD_Data > 0x9999999)&&(unit-3-3+2>=1))
                            {
                             BCD_Data>>=3*4;
                             unit_add = 3;
                            }
                          MeterData->CurrentHeat= BCD_Data<<4*(unit-3-3+2);/*0.01 kWH*/
                          MeterData->CurrentHeatUnit =  KWH+unit_add;
                        }
                       else
                        {
                        MeterData->CurrentHeat = BCD_Data>>4*abs(unit-3-3+2);/*0.01 kWH*/
                        MeterData->CurrentHeatUnit =  KWH+unit_add;
                        }
                        debug_info(gDebugModule[EN13757],"line=%d  CurrentHeat = %x  unit = %x \r\n",__LINE__,MeterData->CurrentHeat,MeterData->CurrentHeatUnit);
                  }
                else if(VIF.Data>=0x08&&VIF.Data<=0x0F)/*E000 1nnn Energy 10(nnn) J 0,001 kJ to 10 000 kJ*/
                  {
                     uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      uint32 HEX_Data = 0;
                      uint8 unit_add = 0;
                      unit = VIF.Data&0x07;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                        }
                      if(unit == 7)
                        {
                        if(HEX_Data >= 99999999)/*BCD 太大*/
                          {
                          HEX_Data = HEX_Data/100;   /*GJ ->100GJ*/
                          unit_add = 2;
                          }
                           if(HEX_Data)
                            {
                            BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                            }
                          MeterData->CurrentHeat= BCD_Data>>4*abs(unit-3-6+2);/*0.01 GJ*/
                          MeterData->CurrentHeatUnit = GJ+unit_add;
                        }
                      else if(unit>=4)
                       {
                         if(HEX_Data >= 99999999)/*BCD 太大*/
                          {
                          HEX_Data = HEX_Data/1000;   
                          unit_add = 3;
                          }
                            if(HEX_Data)
                              {
                              BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                              }
                          if((BCD_Data > 0x999999)&&(unit-3-3+2>=2))
                            {
                             BCD_Data>>=3*4;
                             unit_add = 3;
                            }
                           else if((BCD_Data > 0x9999999)&&(unit-3-3+2>=1))
                            {
                             BCD_Data>>=3*4;
                             unit_add = 3;
                            }
                          MeterData->CurrentHeat= BCD_Data<<4*(unit-3-3+2);/*0.01 MJ*/
                          MeterData->CurrentHeatUnit =  MJ+unit_add;
                        }
                       else
                        {
                        MeterData->CurrentHeat = BCD_Data>>4*abs(unit-3-3+2);/*0.01 MJ*/
                        }
                        debug_info(gDebugModule[EN13757],"line=%d  CurrentHeat = %x  unit = %x \r\n",__LINE__,MeterData->CurrentHeat,MeterData->CurrentHeatUnit);
                  }
                else if(VIF.Data>=0X10&&VIF.Data<=0X17)/*E001 0nnn Volume 10(nnn-6) m3 0,001 l to 10 000 l*/
                  {
                      uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      unit = VIF.Data&0x07;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         uint32 HEX_Data = 0;
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                          BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                        }
                       if(unit>=4)
                        {
                        MeterData->AccumulateFlow = BCD_Data<<4*(unit-6+2);/*0.01 m^3*/
                        }
                       else
                        {
                        MeterData->AccumulateFlow = BCD_Data>>4*abs(unit-6+2);
                        }
                       debug_info(gDebugModule[EN13757],"line=%d  AccumulateFlow = %x  unit = %x \r\n",__LINE__,MeterData->AccumulateFlow,MeterData->AccumulateFlowUnit);
                  }
                else if(VIF.Data>=0X28&&VIF.Data<=0X2F)/*E010 1nnn Power 10(nnn-3) W 0,001 W to 10 000 W*/
                  {
                      uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      unit = VIF.Data&0x07;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         uint32 HEX_Data = 0;
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                         if(HEX_Data >= 99999999)/*BCD 太大*/
                          {
                          HEX_Data = HEX_Data/1000;
                          MeterData->HeatPowerUnit = MW;
                          }
                          BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                        }
                       if(unit>=4)
                        {
                        MeterData->HeatPower= BCD_Data<<4*(unit-3-3+2);/*0.01 kW*/
                        }
                       else
                        {
                        MeterData->HeatPower = BCD_Data>>4*abs(unit-3-3+2);
                        }
                       debug_info(gDebugModule[EN13757],"line=%d  HeatPower = %x  unit = %x \r\n",__LINE__,MeterData->HeatPower,MeterData->HeatPowerUnit);
                  }
                else if(VIF.Data>=0X30&&VIF.Data<=0X37)/*E011 0nnn Power 10(nnn) J/h 0,001 kJ/h to 10 000 kJ/h*/
                  {
                      uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      unit = VIF.Data&0x07;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         uint32 HEX_Data = 0;
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                         if(HEX_Data >= 99999999)/*BCD 太大*/
                          {
                          HEX_Data = HEX_Data/1000;
                          MeterData->HeatPowerUnit = MW;
                          }
                          BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                        }
                       if(unit>=4)
                        {
                        MeterData->HeatPower= BCD_Data<<4*(unit-3-3+2);/*0.01 MJ*/
                        }
                       else
                        {
                        MeterData->HeatPower = BCD_Data>>4*abs(unit-3-3+2);
                        }
                       MeterData->HeatPower  = MeterData->HeatPower/3.6;
                       debug_info(gDebugModule[EN13757],"line=%d  HeatPower = %x  unit = %x \r\n",__LINE__,MeterData->HeatPower,MeterData->HeatPowerUnit);
                  }
                else if(VIF.Data>=0X38&&VIF.Data<=0X3F)/*E011 1nnn Volume Flow 10(nnn-6) m3/h 0,001 l/h to 10 000 l/h*/
                  {
                      uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      unit = VIF.Data&0x07;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         uint32 HEX_Data = 0;
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                         if(HEX_Data >= 99999999)/*BCD 太大*/
                          {
                          
                          //MeterData->HeatPowerUnit = MW;
                          }
                          BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                        }
                       if(unit>=2)
                        {
                        MeterData->Flow= BCD_Data<<4*(unit-6+4);/*0.0001 m3/h*/
                        }
                       else
                        {
                        MeterData->Flow = BCD_Data>>4*abs(unit-6+4);
                        }
                        debug_info(gDebugModule[EN13757],"line=%d  Flow = %x  unit = %x \r\n",__LINE__,MeterData->Flow,MeterData->FlowUnit);
                  }
                else if(VIF.Data>=0X58&&VIF.Data<=0X5B)/*E101 10nn Flow Temperature 10(nn-3) °C 0,001 °C to 1 °C*/
                  {
                      uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      unit = VIF.Data&0x03;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         uint32 HEX_Data = 0;
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                          BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                        }
                       if(unit>=1)
                        {
                        BCD_Data= BCD_Data<<4*(unit-3+2);/*0.01 °C*/
                        }
                       else
                        {
                        BCD_Data = BCD_Data>>4*abs(unit-3+2);
                        }
                       memcpy(MeterData->WaterInTemp,(char*)&BCD_Data,3);
                       debug_info(gDebugModule[EN13757],"line=%d  WaterInTemp = %x.%x  \r\n",__LINE__,MeterData->WaterInTemp[1],MeterData->WaterInTemp[1]);
                  }
                else if(VIF.Data>=0X5C&&VIF.Data<=0X5F)/*E101 11nn Return Temperature 10(nn-3) °C 0,001 °C to 1 °C*/
                  {
                      uint8 unit=0;
                      uint8 num=0;
                      uint32 BCD_Data=0;
                      unit = VIF.Data&0x03;
                      if(DIF.BCD)
                        {
                         for(num=0;num<DIF.Data;num++)
                          {
                          BCD_Data += (data[i++]<<8*num);
                          }
                        }
                      else
                        {
                         uint32 HEX_Data = 0;
                         for(num=0;num<DIF.Data;num++)
                          {
                          HEX_Data += (data[i++]<<8*num);
                          }
                          BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                        }
                       if(unit>=1)
                        {
                        BCD_Data = BCD_Data<<4*(unit-3+2);/*0.01 °C*/
                        }
                       else
                        {
                        BCD_Data = BCD_Data>>4*abs(unit-3+2);
                        }
                       memcpy(MeterData->WaterOutTemp,(char*)&BCD_Data,3);
                       debug_info(gDebugModule[EN13757],"line=%d  WaterOutTemp = %x.%x \r\n",__LINE__,MeterData->WaterOutTemp[1],MeterData->WaterOutTemp[0]);
                  }
                else if(VIF.Data>=0X24&&VIF.Data<=0X27)
                  /*E010 01nn Operating Time   nn = 00b seconds  nn = 01b minutes nn = 10b hours nn = 11b days nn = 11 days*/
                  {
                    i+=DIF.Data;
                  }
                else if(VIF.Data==0X7B)
                  {
                      if(VIFE.value==0||VIFE.value==1)/*E000 000n Energy 10(n-1) MWh 0.1MWh to 1MWh*/
                        {
                            uint8 unit=0;
                            uint8 num=0;
                            uint32 BCD_Data=0;
                            uint32 HEX_Data = 0;
                            uint8 unit_add = 0;
                            unit = VIFE.value&0x01;
                            if(DIF.BCD)
                              {
                               for(num=0;num<DIF.Data;num++)
                                {
                                BCD_Data += (data[i++]<<8*num);
                                }
                                if(BCD_Data>0x999999)
                                {
                                 BCD_Data = BCD_Data>>8;   /*MWH ->100MWH*/
                                 unit_add = 2;
                                }
                              }
                            else
                              {
                               for(num=0;num<DIF.Data;num++)
                                {
                                HEX_Data += (data[i++]<<8*num);
                                }
                                if(HEX_Data > 999999)/*BCD 太大*/
                                {
                                HEX_Data = HEX_Data/100;   /*MWH ->100MWH*/
                                unit_add = 2;
                                }
                                BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                              }
                       
                              {
                                MeterData->CurrentHeat= BCD_Data<<4*abs(unit-1+2);/*0.01 MWH*/
                                MeterData->CurrentHeatUnit = MWH+unit_add;
                              }
                              debug_info(gDebugModule[EN13757],"line=%d  CurrentHeat = %x  unit = %x \r\n",__LINE__,MeterData->CurrentHeat,MeterData->CurrentHeatUnit);
                        
                        }
                      else if(VIFE.value==8||VIFE.value==9)/*E000 100n Energy 10(n-1) GJ 0.1GJ to 1GJ*/
                        {
                            uint8 unit=0;
                            uint8 num=0;
                            uint32 BCD_Data=0;
                            uint32 HEX_Data = 0;
                            uint8 unit_add = 0;
                            unit = VIFE.value&0x01;
                            if(DIF.BCD)
                              {
                               for(num=0;num<DIF.Data;num++)
                                {
                                BCD_Data += (data[i++]<<8*num);
                                }
                                if(BCD_Data>0x999999)
                                {
                                 BCD_Data = BCD_Data>>8;   /*GJ ->100GJ*/
                                 unit_add = 2;
                                }
                              }
                            else
                              {
                               for(num=0;num<DIF.Data;num++)
                                {
                                HEX_Data += (data[i++]<<8*num);
                                }
                                if(HEX_Data >= 999999)/*BCD 太大*/
                                {
                                HEX_Data = HEX_Data/100;   /*GJ ->100GJ*/
                                unit_add = 2;
                                }
                                BCD_Data = HexToBcdUint32(HEX_Data);//HEX_Data MAX 0x5F5E0FF
                              }
                       
                              {
                                MeterData->CurrentHeat= BCD_Data<<4*abs(unit-1+2);/*0.01 GJ*/
                                MeterData->CurrentHeatUnit = GJ+unit_add;
                              }
                              debug_info(gDebugModule[EN13757],"line=%d  CurrentHeat = %x  unit = %x \r\n",__LINE__,MeterData->CurrentHeat,MeterData->CurrentHeatUnit);
                        }
                      else
                        {
                        i+=DIF.Data;
                        }
                  }
                else
                  {
                     i+=DIF.Data;
                  }
                
              
			}
		else if(DIF.Function == 1)/*Maximum value*/
			{
			i+=DIF.Data;
            }
		else if(DIF.Function == 2)/*Minimum value*/
			{
			i+=DIF.Data;
			}
		else if(DIF.Function == 3)/*Value during error state*/
			{
			i+=DIF.Data;
			}
	 }
   
    {/*添加实时时间*/
                uint8 SystemTime[6] = {0}; 
                int i=0;
            	memcpy(SystemTime, gSystemTime, 6);
                MeterData->RealTime[6] = 0x20;
                for(i=0;i<6;i++)
                {
                MeterData->RealTime[i] = HexToBcd(SystemTime[i]);
                } 
               
                if(gDebugModule[EN13757]>=7)
                {
                debug("20%x-%x-%x %x:%x:%x\r\n",MeterData->RealTime[5],MeterData->RealTime[4],MeterData->RealTime[3],MeterData->RealTime[2],\
                  MeterData->RealTime[1],MeterData->RealTime[0]);
                }
     }
	return 0;
}




/*liuzy add 恩乐曼抄上的数据转换 成cj188*/

uint8  ENLEMAN_TO_CJ188(CJ188_Format  *MeterData,uint8* DataBuf,uint8 len)
{
	uint8 err = 0;
	
	uint8 i = 0;
	
	uint8 u8LenCount = 0;

	uint8 u8LenPerVar = 0;
	
   uint32 u32TotalValue = 0;

   uint32 u32TotlHeatValue = 0;

   uint32 u32flow = 0;

   uint32 u32HeatPower = 0;

   uint32 u32CurrentHeat = 0;

   uint8 u8data[4] = {0};

   uint16 u16TempIn = 0;

   uint16 u16TempOut = 0;

   uint16 u16AccWorkTime = 0;

  
   memcpy(u8data,DataBuf+29,4);  //把总流量数据考入 第29个为流量数据

  
   err = To_Little_endian(u8data,4);  //把流量数据按照协议反转
	for(i=0;i<4;i++)
	   {
		   u32TotalValue |=u8data[i]<<(3-i)*8;
	   }

	switch (*(DataBuf+28))  //添加单位流量单位没有加仑

	{
	   
		case 0x13:  //0.001m3

			u32TotalValue = u32TotalValue/10;
			
			u32TotalValue = HexToBcdUint32(u32TotalValue);	 

	        MeterData->AccumulateFlow = u32TotalValue;  // 总流量
	        
			MeterData->AccumulateFlowUnit = M3;
			break;
		case 0x14:  //0.01m3

			u32TotalValue = u32TotalValue;
			
			u32TotalValue = HexToBcdUint32(u32TotalValue);	 

	        MeterData->AccumulateFlow = u32TotalValue;  // 总流量
		
			MeterData->AccumulateFlowUnit = M3;
			break;
		case 0x15:  //0.1m3
			u32TotalValue = u32TotalValue*10;
			
			u32TotalValue = HexToBcdUint32(u32TotalValue);	 

	        MeterData->AccumulateFlow = u32TotalValue;  // 总流量
			
			MeterData->AccumulateFlowUnit = M3;
			break;
		case 0x16:  //m3
			u32TotalValue = u32TotalValue*100;
			
			u32TotalValue = HexToBcdUint32(u32TotalValue);	 

	        MeterData->AccumulateFlow = u32TotalValue;  // 总流量
			MeterData->AccumulateFlowUnit = M3;
			break;
			
		default:
			
			break;
			
	}

  u8LenCount = 29+(*(DataBuf+27)); //  开始流量处加上长度定位到热量的长度判断处

  u8LenPerVar = *(DataBuf+u8LenCount);
  
  if(*(DataBuf+u8LenCount+1) == 0xfb) //单位由两个字节判断
  	{
		memcpy(u8data,DataBuf+u8LenCount+3,4);  //把热量数据拷贝到数组
		err = To_Little_endian(u8data,4);   //反转字节
		  for(i=0;i<4;i++)
		   {
			   u32CurrentHeat |=u8data[i]<<(3-i)*8;
		   }
		switch (*(DataBuf+u8LenCount+2)) //由0xfb下一个字节判断
		{
		   
			case 0x00:  //单位 0.1mwh
			//u32CurrentHeat = (uint32)u8data;
			u32CurrentHeat = u32CurrentHeat*10;  //因为CJ188是后两位为小数
			u32CurrentHeat = HexToBcdUint32(u32CurrentHeat);	 
	    	MeterData->CurrentHeat= u32CurrentHeat;  // 总热量	
			MeterData->CurrentHeatUnit = MWH;
				
			break;
			
			case 0x01: //单位MWH
			//u32CurrentHeat = (uint32)u8data;
			u32CurrentHeat = u32CurrentHeat*100;  
			u32CurrentHeat = HexToBcdUint32(u32CurrentHeat);	 
	    	MeterData->CurrentHeat= u32CurrentHeat;  // 总热量	
			MeterData->CurrentHeatUnit = MWH;
			break;

			default:
			break;
		}
		u8LenCount = u8LenCount+3+4; 
    }
  	else  //单位为一个字节
  	{
  	    memcpy(u8data,DataBuf+u8LenCount+2,4);  //把热量数据拷贝到数组
		err = To_Little_endian(u8data,4);   //反转字节
		  for(i=0;i<4;i++)
		   {
			   u32CurrentHeat |=u8data[i]<<(3-i)*8;
		   }
		switch(*(DataBuf+u8LenCount+1))
		{
			case 0x05:  //0.1kwh
			//u32CurrentHeat = (uint32)u8data;
			u32CurrentHeat = u32CurrentHeat*10;  //因为CJ188是后两位为小数
			u32CurrentHeat = HexToBcdUint32(u32CurrentHeat);	 
	    	MeterData->CurrentHeat= u32CurrentHeat;  // 总热量	
			MeterData->CurrentHeatUnit = KWH;
				break;
			case 0x06: //0.001mwh
			//u32CurrentHeat = (uint32)u8data;
			u32CurrentHeat = u32CurrentHeat/10;  //因为CJ188是后两位为小数
			u32CurrentHeat = HexToBcdUint32(u32CurrentHeat);	 
	    	MeterData->CurrentHeat= u32CurrentHeat;  // 总热量	
			MeterData->CurrentHeatUnit = MWH;
				break;
			case 0x07: //0.01MWH
			//u32CurrentHeat = (uint32)u8data;
			u32CurrentHeat = u32CurrentHeat;  //因为CJ188是后两位为小数
			u32CurrentHeat = HexToBcdUint32(u32CurrentHeat);	 
	    	MeterData->CurrentHeat= u32CurrentHeat;  // 总热量	
			MeterData->CurrentHeatUnit = MWH;
				break;
			case 0x0e:  //0.001Gj 即284kwh 0.284MWH
			//u32CurrentHeat = (uint32)u8data;
			u32CurrentHeat = u32CurrentHeat*284/10;  //因为CJ188是后两位为小数
			u32CurrentHeat = HexToBcdUint32(u32CurrentHeat);	 
	    	MeterData->CurrentHeat= u32CurrentHeat;  // 总热量	
			MeterData->CurrentHeatUnit = MWH;
				break;
			default:
				break;
			
		}
		u8LenCount = u8LenCount+2+4;
	}

    u8LenCount = u8LenCount + 16 ; //下面两个是收费登记 直接跳过

	 memcpy(u8data,DataBuf+u8LenCount+2,4);  //把流量数据拷贝到数组
	 
	 err = To_Little_endian(u8data,4);   //反转字节

	   for(i=0;i<4;i++)
		   {
			   u32flow |=u8data[i]<<(3-i)*8;
		   }

	 switch(*(DataBuf+u8LenCount+1))
		{

		    case 0x39:  //0.01l/h

			//u32flow = (uint32)u8data;
			u32flow = u32flow;  //因为CJ188是后两位为小数
			u32flow = HexToBcdUint32(u32flow);	 
	    	MeterData->Flow= u32flow;  
			MeterData->FlowUnit= L_H;
				
				break;
				
			case 0x3a:  //0.1l/h
			//u32flow = (uint32)u8data;
			u32flow = u32flow*10;  //因为CJ188是后两位为小数
			u32flow = HexToBcdUint32(u32flow);	 
	    	MeterData->Flow= u32flow;  	
			MeterData->FlowUnit= L_H;
			
			break;
			
			case 0x3b:   //1L/h
			//u32flow = (uint32)u8data;
			u32flow = u32flow*100;  //因为CJ188是后两位为小数
			u32flow = HexToBcdUint32(u32flow);	 
	    	MeterData->Flow= u32flow;  	
			MeterData->FlowUnit= L_H;
				break;
				
			case 0x3c:  //0.01m3/h
			//u32flow = (uint32)u8data;
			u32flow = u32flow;  //因为CJ188是后两位为小数
			u32flow = HexToBcdUint32(u32flow);	 
	    	MeterData->Flow= u32flow;  	
			MeterData->FlowUnit= M3_H;
				break;
			case 0x3d: //0.1m3/h
			//u32flow = (uint32)u8data;
			u32flow = u32flow*10;  //因为CJ188是后两位为小数
			u32flow = HexToBcdUint32(u32flow);	 
	    	MeterData->Flow= u32flow;  
			MeterData->FlowUnit = M3_H;
				break;
			case 0x3e: //1m3/h
			//u32flow = (uint32)u8data;
			u32flow = u32flow*100;  //因为CJ188是后两位为小数
			u32flow = HexToBcdUint32(u32flow);	 
	    	MeterData->Flow= u32flow; 
			MeterData->FlowUnit = M3_H;
				break;
			
			default:
				break;
			
		}
		u8LenCount = u8LenCount+2+4;

		
		memcpy(u8data,DataBuf+u8LenCount+2,4);	//把功率数据拷贝到数组
			 
		err = To_Little_endian(u8data,4);	 //反转字节

		 for(i=0;i<4;i++)
		   {
			   u32HeatPower |=u8data[i]<<(3-i)*8;
		   }

		switch(*(DataBuf+u8LenCount+1))
		{
			case 0x2a: //0.1W
			//u32HeatPower= (uint32)u8data;
			u32HeatPower = u32HeatPower*10;  //因为CJ188是后两位为小数
			u32HeatPower = HexToBcdUint32(u32HeatPower);	 
	    	MeterData->HeatPower= u32HeatPower; 
			MeterData->HeatPowerUnit= W;
				break;
			case 0x2b:  //W
			//u32HeatPower = (uint32)u8data;
			u32HeatPower = u32HeatPower*100;  //因为CJ188是后两位为小数
			u32HeatPower = HexToBcdUint32(u32HeatPower);	 
	    	MeterData->HeatPower= u32HeatPower; 
			MeterData->HeatPowerUnit= W;
				break;
			case 0x2c: //0.01KW
			//u32HeatPower = (uint32)u8data;
			u32HeatPower = u32HeatPower;  //因为CJ188是后两位为小数
			u32HeatPower = HexToBcdUint32(u32HeatPower);	 
	    	MeterData->HeatPower= u32HeatPower; 
			MeterData->HeatPowerUnit= KW;
				break;
			case 0x2d:  //0.1KW
			//u32HeatPower = (uint32)u8data;
			u32HeatPower = u32HeatPower*10;  //因为CJ188是后两位为小数
			u32HeatPower = HexToBcdUint32(u32HeatPower);	 
	    	MeterData->HeatPower= u32HeatPower; 
			MeterData->HeatPowerUnit= KW;
				break;
			case 0x2e:  //1KW
			//u32HeatPower = (uint32)u8data;
			u32HeatPower = u32HeatPower*100;  //因为CJ188是后两位为小数
			u32HeatPower = HexToBcdUint32(u32HeatPower);	 
	    	MeterData->HeatPower= u32HeatPower; 
			MeterData->HeatPowerUnit= KW;
				break;
				
			default:
				break;
		}

		
		
		u8LenCount = u8LenCount+2+4;

		memcpy(u8data,DataBuf+u8LenCount+2,2);	//把进水温度拷贝到数组
			 
		err = To_Little_endian(u8data,2);	 //反转字节
		
		u16TempIn = ((u8data[0]<<8)|u8data[1]);

		u16TempIn = HexToBcd2bit(u16TempIn);
	
        memcpy(&(MeterData->WaterInTemp[1]),(char*)&u16TempIn,2);

		MeterData->WaterInTemp[0] = 0;

		u8LenCount = u8LenCount+4;

		

		memcpy(u8data,DataBuf+u8LenCount+2,2);	//把回水温度拷贝到数组
			 
		err = To_Little_endian(u8data,2);	 //反转字节
		
		u16TempOut= ((u8data[0]<<8)|u8data[1]);
		
		u16TempOut = HexToBcd2bit(u16TempOut);

		 memcpy(&(MeterData->WaterOutTemp[1]),(char*)&u16TempOut,2);

		MeterData->WaterOutTemp[0] = 0;

		u8LenCount = u8LenCount+4;
		
		
		u8LenCount = u8LenCount+8; //跳过温差还有累积时间的长度和单位直接到数据
			

		memcpy(u8data,DataBuf+u8LenCount+2,2);	//把累积工作时间拷贝到数组
			 
		err = To_Little_endian(u8data,2);	 //反转字节

		u16AccWorkTime = ((u8data[1]<<8)|u8data[0]);  //单位是天

	    u16AccWorkTime = u16AccWorkTime*24;  //转换成小时

		u16AccWorkTime = HexToBcd2bit(u16AccWorkTime);

		 memcpy(&(MeterData->AccumulateWorkTime[0]),(char*)&u16AccWorkTime,2);

		//MeterData->AccumulateWorkTime[0] = 0;

		{/*添加实时时间*/
                uint8 SystemTime[6] = {0}; 
                int i=0;
            	memcpy(SystemTime, gSystemTime, 6);
                MeterData->RealTime[6] = 0x20;
                for(i=0;i<6;i++)
                {
                MeterData->RealTime[i] = HexToBcd(SystemTime[i]);
                } 
               
                if(gDebugModule[EN13757]>=7)
                {
                debug("20%x-%x-%x %x:%x:%x\r\n",MeterData->RealTime[5],MeterData->RealTime[4],MeterData->RealTime[3],MeterData->RealTime[2],\
                  MeterData->RealTime[1],MeterData->RealTime[0]);
                }
         }
	
	
	   return 0;
	
}



   
   
//任意透传用。 
 uint8 Receive_Read_ParamFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen)
 {
	 uint8 data=0;
	 uint16 i=0;
	 uint8 readinfo[256]={0};  //最多支持接收100字节。
	 uint8 lu8RecDateLen = 0;
	 
  
	 i = 0;
   
	 while(i<256){												  
		if(DuGetch(dev, &data, 1*OS_TICKS_PER_SEC))
		 	{break;}

			readinfo[i++] = data;
			lu8RecDateLen += 1;
			   
	 }
	 
	 *datalen = lu8RecDateLen;
	 memcpy(buf,readinfo,lu8RecDateLen);

   	if(lu8RecDateLen == 0)
   		return 1;
   	else
	  	return 0;
	   
		   
}  


//任意透传用。 

uint8 ReadData_Communicate(uint8 *DataFrame, uint8 *DataLen)
{   
	uint8 err;

	//uint8 dev = DOWN_COMM_DEV_MBUS;
	uint8 dev = 0;
	uint8 i=0; 
	uint8 len=0;
		 
	uint8 DataBuf[METER_FRAME_LEN_MAX*2];
	LOG_assert_param(DataFrame == NULL);
	LOG_assert_param(DataLen == NULL);

	dev = gDownCommDev485;   //gDownCommDev485决定是458总线还是MBus总线。
	   
	DuQueueFlush(dev);				//清空缓冲区	
	DuSend(dev, (uint8*)DataFrame,  *DataLen);
			  
	OSTimeDly(OS_TICKS_PER_SEC/2);
	//OSTimeDlyHMSM(0,0,1,0);
			  
	err = Receive_Read_ParamFrame(dev, DataBuf, 0, &len);
	   
	if(err==NO_ERR){
			memcpy(DataFrame, &DataBuf[0], len);
			*DataLen = len;
			return NO_ERR;
	}
	   
			
	return 1;
	
}



//专用于亿林协议阀控器数据接收。
uint8 Yilin_Valve_ReceiveFrame(uint8 *buf, uint8 *datalen)
{
	uint8 data 	= 0x00;
	uint8 Cs	= 0x00;
	uint8 dev = 0;
	uint32 i,j;
	
	LOG_assert_param(buf == NULL);
	LOG_assert_param(datalen == NULL);

	dev = gDownCommDev485;   //gDownCommDev485决定是458总线还是MBus总线。
	
	DuQueueFlush(dev);				//清空缓冲区	
	DuSend(dev, buf,  *datalen);
			  
	OSTimeDly(OS_TICKS_PER_SEC/2);
	
	*datalen = 0;
		
	i = 30;
	while(i--)														//找帧头
	{
		FeedTaskDog();   
		if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC))  			
			return 1;
		if(data==0x50)	break; 	     
	}
	Cs 		= data;
	*buf++ 	= data;
	
    for(i=0; i<2; i++)												//地址
    {
		if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  	
			return 2;
		Cs 		+= data;
		*buf++	 = data;
	}

	for(j=0; j<8; j++)											//数据域
	{
	    if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  	
			return 3;
		*buf++  = data;
		Cs     += data;
    }  
     
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  		
		return 4;		//校验字节
	Cs = Cs ^ 0xA5;
	if(data != Cs)         							
		return 5;   
	*buf++	= data; 
	   
  	*datalen 	= 12;
  	
    return NO_ERR;
}


   
   

/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
