/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: timingpro.c
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年8月10日 
**  描        述: 定时抄表处理文件
**	修 改  记 录:   	
*****************************************************************************************************/

/********************************************** include *********************************************/
#include <includes.h>
#include "app_flashmem.h"
#include "app_down.h"
#include "Tasks_up.h"

/********************************************** define *********************************************/
//#define  HYDROMETER_DEBUG
#define METER_PRE 		1
#define METER_START		2

#define TIME_AREA_PATH  "/timearea"
/********************************************** global *********************************************/
TimingState	gREAD_TimingState;										//抄表状态
/********************************************** static *********************************************/
//在协议修改的基础上，在抄读热计量表数据时，需要对其的温控面板数据抄读。现不对2者做数据项区分，必须2者都抄读成功
//才设置抄表成功标志,   不管抄表是否成功，均存储表数据信息
static uint8  gREAD_ReadCmplFlag[75] = {0};							//抄表完成标志 

extern uint8 gDebugBuffer[250];
extern uint8  gDownCommDev485;
uint16 gDebugLength=0;


extern OS_EVENT* HeartFrmMbox;
extern uint16 VirtualMeterSn;

#ifdef DEMO_APP
extern uint8 Channel_1_SUCESS ;
extern uint8 Channel_2_SUCESS ;
extern uint8 Channel_3_SUCESS ;
extern uint8 Meter_R_End ;

#endif


extern void SaveVirtualMeterSn(uint16 VirtualMeterSn);


/**********************************************************************************************
**	函 数  名 称: CopyDatatoDisplay(uint16 MeterSN,uint8 *p_DstAddr,uint8 *p_SrcAddr,uint8 len)
**	函 数  功 能: 将抄表数据放入相应数据供显示器显示，如果格式不一致，
						需要调整成德鲁格式。
**	输 入  参 数: 
**	输 出  参 数: 
**    返   回   值    : 无

**********************************************************************************************/
void CopyDatatoDisplay(uint16 MeterSN,uint8 *p_DstAddr,uint8 *p_SrcAddr,uint8 len)
{
	uint8 Err = 0;
	MeterFileType mf;
	uint8 lu8tmp = 0;
	CJ188_Format  MeterData={0,0x05,0,0x05,0,0x17,0,0x35,0,0x2c};

	if(len > METER_FRAME_LEN_MAX)  //超限检查。
		len = METER_FRAME_LEN_MAX;
	
	Err = PARA_ReadMeterInfo(MeterSN, &mf);
	
	memcpy(p_DstAddr,p_SrcAddr,len);  //先转移数据到目的地址。

	if(Err == NO_ERR){
		switch(mf.ProtocolVer){
			case WANHUA_VER:{//天津万华单位在前，并且解析时需要减去0x33.

				WANHUA_Format  Data;
            	uint8* pTemp = (uint8*)&Data;
            	uint8 i = 0;
				
				lu8tmp = *(p_DstAddr+17);
            	memcpy(&Data,(p_DstAddr+18),lu8tmp);
           		for(i=0;i<lu8tmp;i++){  
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

				memcpy((p_DstAddr+18),&MeterData,lu8tmp);

				break;
			}

			default:{
				
				break;
			}

		}			

	}


}


/****************************************************************************************************
**	函 数  名 称: READ_ReadOneMeter
**	函 数  功 能: 读取一个表的数据
**	输 入  参 数: char*path--保存文件路径 uint16 MeterSn -- 表序号
**	输 出  参 数: 
**    返   回   值    : 无
**	备		  注        : char  path[] = "/2012/12/24/1530"; 1530 :DataStoreParaType=128byte + MeterSn*128byte
**     修改记录   :1. add input para char*path 2012-12-26  
                                        old:READ_ReadOneMeter(uint16 MeterSn)
**                                    new:READ_ReadOneMeter(char*path,uint16 MeterSn)
*****************************************************************************************************/
uint8 READ_ReadOneMeter(char*path,uint16 MeterSn)
{
	uint8 Err = 0;
	uint8 Res = 0;
	uint8 ReadTime[6]					   	= {0x00};
	MeterFileType MeterFile;
//	uint32 totalHot = 0;
//	CJ188_Format *cj188;
	uint8 MeterDataBuf[METER_FRAME_LEN_MAX] = {0x00};				//暂时存储抄读到的表数据
	CPU_SR		cpu_sr;
		
	LOG_assert_param(MeterSn > METER_NUM_MAX);
	
	FeedTaskDog();
	ReadDateTime(ReadTime);											//读取当前时间
	gu8ReadValveFail = 0; //抄阀标志清零，防止影响抄热表失败记录。
	
	Res = PARA_ReadMeterInfo(MeterSn, &MeterFile);
	if(Res != NO_ERR){
    	LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <READ_ReadOneMeter> Read Meter filed failed, MeterSn is %4XH, Return is %d",MeterSn, Res);
    }

	if(MeterFile.EquipmentType == HEAT_METER_TYPE){  //如果是热量表
		Res = METER_ReadMeterDataTiming(MeterSn, MeterDataBuf);
	
	}
	else{	//否则就认为是阀控。
		Res = VALVE_ReadMeterDataTiming(MeterSn, MeterDataBuf);
		
	}

	if(Res == NO_ERR){	
		OS_ENTER_CRITICAL();   //抄成功数量++.
		gREAD_TimingState.ReadCmplNums++;
		gu16ReadMeterSuccessNum += 1;
		OS_EXIT_CRITICAL();
        debug_info(gDebugModule[TASKDOWN_MODULE],"INFO: %s  MeterSn=%d Read Successful!",__FUNCTION__,MeterSn);
	}
	else{
		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <READ_ReadOneMeter> The HeatMeter is no answer!");
	}

	/*不管抄是否成功都保存数据到SD卡中*/
	OSMutexPend (FlashMutex,0,&Err);
	if(MeterFile.EquipmentType == HEAT_METER_TYPE){ //前面保持热表的数据
		Err =SDSaveData(path, MeterDataBuf, METER_FRAME_LEN_MAX,gu16ReadHeatMeterNum*128);
		if(gu16ReadHeatMeterNum < gPARA_HeatMeterNum)
			gu16ReadHeatMeterNum++;
	}
	else{ //后面保存阀控的数据。
		Err =SDSaveData(path, MeterDataBuf, METER_FRAME_LEN_MAX,(gPARA_HeatMeterNum+gu16ReadValveNum)*128);
		if(gu16ReadValveNum < gPARA_ValveNum)
			gu16ReadValveNum++;
	}
	OSMutexPost (FlashMutex);
	if(Err != NO_ERR)
	    LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
	
	//将抄表数据存入gu8ReadDataStore，备轮显用。
	CopyDatatoDisplay(MeterSn,gu8ReadDataStore,&MeterDataBuf[1],66);


	return Res;


	
}


/****************************************************************************************************
**	函 数  名 称: HandleOneMBusShortMeter
**	函 数  功 能:表所在邋邋MBUS短路，对表进行处理。
**	输 入  参 数: char*path--保存文件路径 uint16 MeterSn -- 表序号
**	输 出  参 数: 
**    返   回   值    : 无
**	备		  注        : char  path[] = "/2012/12/24/1530"; 1530 :DataStoreParaType=128byte + MeterSn*128byte
**     修改记录   :1. 

*****************************************************************************************************/
uint8 HandleOneMBusShortMeter(char*path,uint16 MeterSn)
{
	uint8 Err = 0;
	uint8 Res = 0;
	MeterFileType MeterFile;
	uint8 DataBuf[METER_FRAME_LEN_MAX]	= {0x00};
	uint8 *pTemp = DataBuf;
	uint8 DataLen = 0;
	uint8 lu8ReadTime[6] = {0};
	
	LOG_assert_param(MeterSn > METER_NUM_MAX);
	
	FeedTaskDog();
	
	Res = PARA_ReadMeterInfo(MeterSn, &MeterFile);
	if(Res != NO_ERR){
    	LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <READ_ReadOneMeter> Read Meter filed failed, MeterSn is %4XH, Return is %d",MeterSn, Res);
    }


	pTemp++;	//为数据长度预留
	memcpy(pTemp,(uint8 *)&(MeterFile.MeterID), 2);  //MeterID.
	pTemp += 2;
	DataLen = 2;
	*pTemp = MeterFile.EquipmentType;  //设备类型。
	pTemp += 1;
	DataLen += 1;

	if(MeterFile.EquipmentType == HEAT_METER_TYPE){  //如果是热量表
		memcpy(pTemp, MeterFile.MeterAddr, 7);
	}
	else{
		memcpy(pTemp, MeterFile.ValveAddr, 7);
	}
	
	pTemp += 7;
	DataLen += 7;
	
	*pTemp = MeterFile.BulidID;  //楼号。
	pTemp += 1;
	DataLen += 1;
	*pTemp = MeterFile.UnitID;  //单元号。
	pTemp += 1;
	DataLen += 1;
	memcpy(pTemp,(uint8 *)&(MeterFile.RoomID), 2); //房间号
	pTemp += 2;
	DataLen += 2;

	ReadDateTime(lu8ReadTime);
	memcpy(pTemp,lu8ReadTime, 3); //加入抄阀时间当前时间，只放时、分、秒。
	pTemp += 3;
	DataLen += 3;

	*pTemp = 1;//MBUS短路，这里热表数据固定为ff,长度为1，传到上位机表示短路。
	pTemp += 1;
	DataLen += 1;
	*pTemp = 0xff;
	pTemp += 1;
	DataLen += 1;

	DataBuf[0]	= DataLen;						//数据域长度
 	*pTemp = PUBLIC_CountCS(&DataBuf[1], DataLen);

	OSMutexPend (FlashMutex,0,&Err);
	if(MeterFile.EquipmentType == HEAT_METER_TYPE){ //前面保持热表的数据
		Err =SDSaveData(path, DataBuf, METER_FRAME_LEN_MAX,gu16ReadHeatMeterNum*128);
		gu8ReadValveFail = 0;  //如果是热表则清阀控失败标记。
		if(gu16ReadHeatMeterNum < gPARA_HeatMeterNum)
			gu16ReadHeatMeterNum++;
	}
	else{ //后面保存阀控的数据。
		Err =SDSaveData(path, DataBuf, METER_FRAME_LEN_MAX,(gPARA_HeatMeterNum+gu16ReadValveNum)*128);
		gu8ReadValveFail = 1;//如果是阀，进入此函数，认为抄阀失败。
		if(gu16ReadValveNum < gPARA_ValveNum)
			gu16ReadValveNum++;

		gu16ValveIDRecord[gu16ReadValveNum-1] = MeterFile.MeterID;  //记录MeterID。
	}
	OSMutexPost (FlashMutex);
	
	if(Err!=NO_ERR) {
		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
	}

	
	memcpy(gu8ReadDataStore,&DataBuf[1],66);  //将刚抄到的表数据存入gu8ReadDataStore，备轮显用。
	
	return Res;
}


/****************************************************************************************************
**	函 数  名 称: READ_ReadAllMeters
**	函 数  功 能: 读取所有表档案中的热计量表和阀门，温控面板
**	输 入  参 数: 无
**	输 出  参 数: 无
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
uint16 gu16ReadMeterNum = 0;  //显示抄表进度时有错误，此变量为重新计数包括所有表和阀的数量。
uint16 gu16ReadMeterSuccessNum = 0;

uint8 READ_ReadAllMeters(void)
{
	uint8 i,j;
	uint8 TempNums	= 0x00;
	uint8 ReadTime[6] = {0x00};
	uint16 TempArray[METER_PER_CHANNEL_NUM] = {0x00};
	uint16 ReadingMeterSn	= 0x00;
	/*begin:yangfei added 2012-12-24 find path*/
	uint8 SystemTime[6] = {0x00};
	char  DataPath[] = "/2012/12/24/1530";
	uint8 Res = 0;
//	uint8 Err = 0;
	uint8 err = 0;

	/*end   :yangfei added 2012-12-24*/
	CPU_SR			cpu_sr;	
	ReadDateTime(ReadTime);
	//初始化抄表状态, 所以该全局变量的有效时间段为2个抄表时间段内
	memset(gREAD_TimingState.TimingStartTime, 0x00, sizeof(gREAD_TimingState));
	memset((uint8 *)gPARA_Meter_Failed, 0x00, sizeof(gPARA_Meter_Failed));/*yangfei added 2013-11-17*/
	memset((uint8 *)gu8ReadDataStore,0,sizeof(gu8ReadDataStore));   //added by zjjin,抄表轮显用，首先清除。
	OS_ENTER_CRITICAL();
	memcpy(gREAD_TimingState.TimingStartTime, ReadTime, 6);
	gREAD_TimingState.TimingMeterNums = 0;
	/*begin:yangfei added 2012-12-24 find path*/
	memcpy(SystemTime, gSystemTime, 6);
	/*end   :yangfei added 2012-12-24*/

	gu16ReadMeterNum = 0;
	gu16ReadMeterSuccessNum = 0;	
	OS_EXIT_CRITICAL();

	gu16ReadHeatMeterNum   = 0;  //开始抄表，清空计数。
	gu16ReadValveNum  = 0;

	gu8ReadValveFail = 0;//开始抄表，清阀控是否失败标记。

	

	/*begin:yangfei added 2012-12-24 find path*/
	GetFilePath(DataPath, SystemTime, ARRAY_DAY);
	GetTimeNodeFilePath(DataPath,SystemTime,gPARA_TimeNodes);
	/*end   :yangfei added 2012-12-24*/
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <READ_ReadAllMeters> Start Reading All of Meters!");

	

	/*begin:yangfei added 20140309 for 防止通道被切换打乱*/
	 do{
			FeedTaskDog();
			OSSemPend(METERChangeChannelSem, 5*OS_TICKS_PER_SEC, &err); //申请MBUS通道切换权利
	}while(err!=OS_ERR_NONE);
	/*end:yangfei added 20140309 for 防止通道被切换打乱*/ 

	memset(gu8MBusShortFlag,0,sizeof(gu8MBusShortFlag)); //MBUS短路标识先清除。

	for(i=0; i<METER_CHANNEL_NUM; i++)/*METER_CHANNEL_NUM*/
	{
		gu8NowMBUSChannel = i + 1;//记录当前MBUS通道。

		if(i==METER_CHANNEL_NUM-1){
		    METER_ChangeChannel(i+1);	 /*485 热表通道*/	
			gDownCommDev485 = DOWN_COMM_DEV_485; //第7通道固定为邋RS485热表。
		}
		else{
			//if(!STA_MBUS_OFF())//muxiaoqing add
			METER_ChangeChannel(i+1);		/*通道号 从 1开始 切换Mbus通道*/
			gDownCommDev485 = DOWN_COMM_DEV_MBUS;    
								
		}

		OS_ENTER_CRITICAL();
		//将预先按照通道存放的表信息转存。
		memcpy((uint8 *)TempArray, (uint8 *)&gPARA_MeterChannel[i], gPARA_MeterChannelNum[i]*sizeof(uint16));
		TempNums = gPARA_MeterChannelNum[i];
		OS_EXIT_CRITICAL();
		if(TempNums > METER_PER_CHANNEL_NUM)  //超限检查。
			TempNums = METER_PER_CHANNEL_NUM;  


		//逐个抄本通道的表。
		for(j=0; j<TempNums; j++){
			
			OS_ENTER_CRITICAL();
			gREAD_TimingState.TimingMeterNums++;
			gu16ReadMeterNum += 1;
			OS_EXIT_CRITICAL();
			
			ReadingMeterSn = TempArray[j];
			debug("\r\n MeterSn = %d\r\n",ReadingMeterSn);

			if(gu8MBusShortFlag[i] == 0){
					Res = READ_ReadOneMeter(DataPath,ReadingMeterSn);
					OSTimeDlyHMSM(0,0,0,200);  //抄表时间间隔。
										
					if(Res == NO_ERR){	//抄表或阀成功，记为0xAA。
						gPARA_Meter_Failed[i][j]=0xaa; 
						debug_info(gDebugModule[METER_DATA],"ReadingMeterSn = %d  read success!\r\n",ReadingMeterSn);
					}
					else{
						if(gu8ReadValveFail == 0)  //抄热表失败，记为0x00.
							gPARA_Meter_Failed[i][j]=0x00; 
						else
							gPARA_Meter_Failed[i][j]=0xbb;  //抄阀控失败，记为0xbb.

						debug_err(gDebugModule[METER_DATA],"ReadingMeterSn = %d  read failed\r\n",ReadingMeterSn);
						gu8ReadValveFail = 0; //抄阀标志清零，防止影响抄热表失败记录。
					}
			}
			else{
				Res = HandleOneMBusShortMeter(DataPath,ReadingMeterSn); //MBUS短路对表进行相应处理。
				if(gu8ReadValveFail == 0)  //抄热表失败，记为0x00.
					gPARA_Meter_Failed[i][j]=0x00; 
				else
					gPARA_Meter_Failed[i][j]=0xbb;  //抄阀控失败，记为0xbb.
					
				OSTimeDlyHMSM(0,0,0,200);  //防止写SD卡太频繁。

			}
									
		}

	}
		
	OSSemPost(METERChangeChannelSem);

	//begin:将抄表失败信息保存到邋SD卡，供失败信息查询时读出调用。
	OSTimeDlyHMSM(0,0,0,500);
	
	OSMutexPend (FlashMutex,0,&err);
	for(i=0;i<METER_CHANNEL_NUM;i++){    //一次写入太多数据有时不成功，分多次写入SD卡中。
		for(j=0;j<5;j++){
			err = SDSaveData("/METER_Failed_Info", (uint8 *)(&gPARA_Meter_Failed[i][0]),METER_PER_CHANNEL_NUM,i*METER_PER_CHANNEL_NUM);
			if(err == NO_ERR){
				break;
			}
			else{
				OSTimeDlyHMSM(0,0,0,100);
			}
		}
		OSTimeDlyHMSM(0,0,0,100);
	}
	OSMutexPost (FlashMutex);
	//end:将抄表失败信息保存到邋SD卡，供失败信息查询时读出调用。

		
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <READ_ReadAllMeters> All of Meters Readed!");
	ReadDateTime(ReadTime);
	OS_ENTER_CRITICAL();
	memcpy(gREAD_TimingState.TimingEndTime, ReadTime, 6);
	OS_EXIT_CRITICAL();
	Meter_R_End =1;
	DisableAllMBusChannel();//断开所有通道。
	
	return NO_ERR;
}

/****************************************************************************************************
**	函 数  名 称: READ_ParaInit
**	函 数  功 能: 初始化timingpro.c文件中的全局变量
**	输 入  参 数: 无
**	输 出  参 数: 无
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
void READ_ParaInit(void)
{
	memset(gREAD_ReadCmplFlag, 0x00, sizeof(gREAD_ReadCmplFlag));
	memset(gREAD_TimingState.TimingStartTime, 0x00, sizeof(gREAD_TimingState));
}
/********************************************************************************************************
**                                             和德鲁美特表

yangfei added 20130802
68 0B 0B 68 53 FD 52 73 60 72 44 FF FF FF FF 27 16

68 04 04 68 53 fd 50 00 a0 16

10 7b fd 78 16

10 40 fd 3d 16
********************************************************************************************************/
int  HYDROMETER(DELU_Protocol *pProtocoalInfo,uint8 ProtocolVer)
{
    char  Selection[]={0x68 , 0x0B, 0x0B, 0x68, 0x53, 0xFD, 0x52,0x73,0x60,0x72,0x44,0xFF,0xFF,0xFF,0xFF,0x27,0x16};
//    char  Standard_data_reading[]={0x68, 0x04, 0x04, 0x68, 0x53, 0xfd, 0x50, 0x00, 0xa0, 0x16};
    //char  Standard_data_reading[]={0x68, 0x04, 0x04, 0x68, 0x53, 0xfd, 0x50, 0x50, 0xf0, 0x16};
    //char  Standard_data_reading[]={0x68, 0x04, 0x04, 0x68, 0x53, 0xfd, 0x50, 0x30, 0xd0, 0x16};
    char  Request_response[]={0x10,0x7b,0xfd,0x78,0x16};
    char  Deselection[]={0x10,0x40,0xfd,0x3d,0x16};
    uint8 cs		= 0x00;
    uint8 Res_data = 0;
	uint8 Err = 0;
    int i=0,j;
    //char  meterAddr[][4]={{0x44,0x69,0x96,0x41},{0x44,0x69,0x96,0x43},{0x44,0x67,0x15,0x27},{0x44,0x72,0x60,0x73}};
    //char  meterAddr[][4]={{0x41,0x96,0x69,0x44},{0x43,0x96,0x69,0x44},{0x27,0x15,0x67,0x44},{0x73,0x60,0x72,0x44}};
    
    uint8 dev = DOWN_COMM_DEV_MBUS;
  
    //uint8 *pBuf=gDebugBuffer;
    uint8 ret;
    //uint8 Err = NO_ERR;
    uint8 RetryTimes			= 2;
  
          memset((char*)gDebugBuffer,0,strlen((char*)gDebugBuffer));
          //METER_ChangeChannel(1);		
      
          memcpy(&Selection[7], pProtocoalInfo->MeterAddr, 4);				//组建 地址域
          //memcpy(&Selection[7], meterAddr[i], 4);				//组建 地址域
          cs = PUBLIC_CountCS((uint8*)&Selection[4], 11);			//计算 校验字节
          Selection[15] = cs;
          debug_info(gDebugModule[METER_DATA],"send frame is:\r\n");
          for(j=0;j<strlen(Selection);j++)
            {
            		if(gDebugModule[METER_DATA]>=KERN_DEBUG)
               	{
               	debug("%2x ",Selection[j]);
               	}
            }
          debug_info(gDebugModule[METER_DATA],"\r\n");
      
      	{
      	char Sn_str[16]={0};
      	PUBLIC_MeterAddrToString(pProtocoalInfo->MeterAddr, Sn_str,7);
      	debug_info(gDebugModule[METER_DATA],"SN = %s!", Sn_str); 
      	LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "SN = %s!", Sn_str);  
      	}
           
          //METER_ComSet1();/*yangfei deleted 2013-11-13*/
          //OSTimeDly(OS_TICKS_PER_SEC);/*yangfei deleted 2013-11-02*/
          
	do{
		FeedTaskDog();
		OSSemPend(UpAskMeterSem, 5*OS_TICKS_PER_SEC, &Err);                         //申请MBUS的通道
	}while(Err!=OS_ERR_NONE);
	
          for(i=0;i<RetryTimes;i++)
          {
              DuQueueFlush(dev);   										//清空缓冲区 	
              DuSend(dev, (uint8*)Selection, sizeof(Selection));
      	#if 0
      	if(DuGetch(dev, &Res_data, 2*OS_TICKS_PER_SEC))  			{debug_info(gDebugModule[METER_DATA],"outtime \r\n");continue;}
      	if(Res_data==0xE5)	
      		{debug_info(gDebugModule[METER_DATA],"Res_data==0xE5 \r\n");    }
      	else
      		{
      		debug_info(gDebugModule[METER_DATA],"Res_data!=0xE5 \r\n");
      		continue;
      		}
      	#endif
              OSTimeDly(OS_TICKS_PER_SEC);
           #if 0
      	 if(ProtocolVer!=ZENNER_VER)
      		{
      	        DuQueueFlush(dev);   										//清空缓冲区 	
      		 DuSend(dev, (uint8*)Standard_data_reading, sizeof(Standard_data_reading));
      		 debug_info(gDebugModule[METER_DATA],"strlen(Standard_data_reading)=%d\r\n",sizeof(Standard_data_reading));
      	        OSTimeDly(OS_TICKS_PER_SEC);
      		}
           #endif
      	 //for(i=0;i<RetryTimes;i++)
               {
      	        DuQueueFlush(dev);   										//清空缓冲区 	
      	        DuSend(dev, (uint8*)Request_response, sizeof(Request_response));
      	        ret = HYDROMETER_METER_ReceiveFrame(dev, pProtocoalInfo->DataBuf, 0, &pProtocoalInfo->Length,ProtocolVer);
      	        if(ret==0)
      	          {
      	          debug_info(gDebugModule[METER_DATA],"pProtocoalInfo->Length =%d \r\n",pProtocoalInfo->Length);
      	          debug_info(gDebugModule[METER_DATA],"HYDROMETER_METER_ReceiveFrame ok\r\n");
      		      LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "HYDROMETER_METER_ReceiveFrame ok\r\n");
          			{
                          #ifdef  HYDROMETER_DEBUG
          				uint8 Err = 0;
          		        uint8 Res = 0;
          			    char  path[] = "/ZENNER1.txt";
          				static uint8 MeterSn = 0;
          			      /*begin:yangfei modified 2013-11-15 for add debug*/
          				OSMutexPend (FlashMutex,0,&Err);
          				Res =SDSaveData(path, pProtocoalInfo->DataBuf, pProtocoalInfo->Length-3,(MeterSn++)*256);
          				OSMutexPost (FlashMutex);
          				if(Res!=NO_ERR)
          			      {
          				LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
          			       }
          			      /*end   :yangfei modified 2013-11-15 for add debug*/	
                          #endif
          	          }
                    DuQueueFlush(dev);   										//清空缓冲区 	
                    DuSend(dev, (uint8*)Deselection, sizeof(Deselection));
                    OSTimeDly(OS_TICKS_PER_SEC);/*yangfei deleted 2013-11-02*/
      	          break;
      	          }
      	        else
      	          {
          	          debug_err(gDebugModule[METER_DATA],"HYDROMETER_METER_ReceiveFrame ret = %d \r\n",ret);
          		      LOG_WriteSysLog_Format(LOG_LEVEL_WARN,"ERROR: HYDROMETER_METER_ReceiveFrame ret = %d \r\n",ret);
          		      OSTimeDly(OS_TICKS_PER_SEC);
      	          }
      	 }
              DuQueueFlush(dev);   										//清空缓冲区 	
              DuSend(dev, (uint8*)Deselection, sizeof(Deselection));
              OSTimeDly(OS_TICKS_PER_SEC);/*yangfei deleted 2013-11-02*/
         }
	
	OSSemPost(UpAskMeterSem);
        	if(i>=RetryTimes)										//重试3次未抄读到数据
       		{
                     debug_err(gDebugModule[METER_DATA],"%s line=%d  HYDROMETER read out  RetryTimes\r\n",__FUNCTION__,__LINE__);
                     return 1;
       		}
           
        return 0;
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
**	备		  注: 
68 1C 1C 68     -----1c 1c 为长度1c= 15byte+data
08 00 72 73 60 72 44 24 23 52 0C 1A 50 00 00  ----15byte
0C FB 08 00 00 00 00 04 6D 02 0F A1 18            ----data
5C 16     
*****************************************************************************************************/

uint8 HYDROMETER_METER_ReceiveFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen,uint8 ProtocolVer)
{
	uint8 data 	= 0x00;
	uint8 len	= 0x00;
	uint8 Cs	= 0x00;
	uint8 RevState = METER_PRE;
	uint32 i;
    uint8 RcvData[256]	= {0x00};
	//uint8 BufData[256]	= {0x00};
	
    #ifdef  HYDROMETER_DEBUG
	uint8 BufData[256]	= {0x00};
	*datalen = 0;
	for(i=0;i<100;i++)
		{
			if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC))  			{return 1;}
			BufData[i]= data;
			if(i==99)
				{
				PUBLIC_HexStreamToString(BufData, 100, RcvData);
				LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "%s", RcvData);  
				return 2;
				}
		}
	#endif
		
	  i = 7;
	if(ProtocolVer == ZENNER_VER)/*真兰协议固定长度为0XC7*/
		{
			while(i--)														//找帧头
			{
				if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC))  			{return 1;}

				switch(RevState)
				{
					case METER_PRE:
					{
						if(data == 0xc7)
						{
							RevState = METER_START;
							len = data;	
						}
						break;
					}
					case METER_START:
					{
						
						if(data == 0x68)
						{
							RevState = METER_PRE;
							goto FIND_DATA;
						}
						else if(data != 0xC7 )
						{
							RevState = METER_PRE;
						}
						break;
					}
					default:
					{
						RevState = METER_PRE;
						break;
					}
						
				}
					
			}
			if(i == 0x00)		{return 2;}
			
		}
	else
		{
			while(i--)														//找帧头
			{
				if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC))  			{return 1;}
				if(data==0x68)	{break; 	     }
				
			}
			if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  	{return 2;}
		       len = data;
			    if(DuGetch(dev, &data, OS_TICKS_PER_SEC)) {return 3;};/*len*/
			    if(data != len)
			    	{
			    	      LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "%s %d len is err len = %d data = %d\r\n",__FUNCTION__,__LINE__,len,data);
			      	      //return 4;
	    			}
		}
	
	
    if(DuGetch(dev, &data, OS_TICKS_PER_SEC)){return 4;}
    if(data != 0x68)
      {
        debug_err(gDebugModule[METER_DATA],"%s %d can't find 2nd 0x68\r\n",__FUNCTION__,__LINE__);
      	return 5;
      }
    debug_info(gDebugModule[METER_DATA],"len = %d \r\n",len);
FIND_DATA:
    for(i=0; i<len; i++)												//地址
    {
		if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  	{return 6;}
		Cs 		+= data;
		RcvData[i] = data;
         if(gDebugModule[METER_DATA]>=KERN_DEBUG)
         	{
         	debug("%2x ",data);
         	}
	}
    debug_info(gDebugModule[METER_DATA],"\r\n");

    //memcpy(buf,&RcvData[15],len-15);
    memcpy(buf,RcvData,len);/*yangfei modified 2013-11-12 for explain all data*/
     
    if(DuGetch(dev, &data, OS_TICKS_PER_SEC)){return 7;}
    if(Cs!=data)
      {
        debug_err(gDebugModule[METER_DATA],"HYDROMETER checksum error\r\n");
	return 8;
      }
    
    if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  		{return 9;}		//结束符
	if(data != 0x16)
	  {
	  debug_err(gDebugModule[METER_DATA],"HYDROMETER end error \r\n");
	  return 12;
	  }

    //*datalen = len-15 +3;/*为适应原来代码结构+3,实际长度为len-15*/
    *datalen = len + 3;/*为适应原来代码结构+3*/
    
    return NO_ERR;
}

/*保存热分配表法中VirtualMeterSn*/
void SaveVirtualMeterSn(uint16 VirtualMeterSn)
{
	uint8 err;
	CPU_SR		cpu_sr;
	DataStoreParaType	InitPara;
	uint8 SystemTime[6] = {0x00};
	char  NodePath[] = "/2012/12/24/timenode";
	memset(&InitPara,0,sizeof(InitPara));
	/*save VirtualMeterSn*/
	OS_ENTER_CRITICAL();
	InitPara.MeterNums = VirtualMeterSn;/*VirtualMeterSn:每个分配表采集器包数可能不一样*/
	memcpy((uint8 *)InitPara.TimeNode, (uint8 *)gPARA_TimeNodes, sizeof(gPARA_TimeNodes));
	memcpy(SystemTime, gSystemTime, 6);
	OS_EXIT_CRITICAL();

	GetFilePath(NodePath, SystemTime,ARRAY_DAY);
	OSMutexPend (FlashMutex,0,&err);
	err =SDSaveData(NodePath, &InitPara, sizeof(DataStoreParaType),0);
	if(err!=NO_ERR)
	{
		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
		debug("WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__);
	}
	OSMutexPost (FlashMutex);

}




/********************************************************************************************************
**  函 数  名 称: getValveSn				    							                       
**	函 数  功 能: 通断时间面积法中获取阀的的序号参数                                                                         			
**	输 入  参 数: uint16 meterID 计量点号
**                uint16 *pMeterSn...阀的序号							         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 本函数应该有未查找到错误返回标志	                                                                                   																									
*********************************************************************************************************/
uint8 getValveSn(uint16 meterID, uint16 *pValveSn)
{
	int8   Res		= 0x00;
	uint16 i 		= 0x00;
	uint16 MeterNum = 0x00;
	uint16 lu16MeterID = 0;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	MeterNum = gPARA_MeterNum;
	OS_EXIT_CRITICAL();

	LOG_assert_param( pValveSn == NULL );
	
	for(i=0; i<MeterNum; i++)
	{
		OS_ENTER_CRITICAL();
		lu16MeterID = gPARA_MeterFiles[i].MeterID;
		OS_EXIT_CRITICAL();

		if(lu16MeterID == meterID)							//2者比较一致
			{
				*pValveSn = i;
				return NO_ERR;
			}
	}
	
	return 1;
}



/****************************************************************************************************
**	函 数  名 称: IssueAllHeatValue
**	函 数  功 能: 通断时间面积法中，用于将上位机下发的分摊值依次发送到
					   各用户阀控器。
**	输 入  参 数: 无
**	输 出  参 数: 无
**	返	 回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
void IssueAllHeatValue(void)
{
	uint16 i = 0;
	char  DataPath[] = "/2012/12/24/1530";
	uint8 Err = 0;
	uint16 lu16HeatMeterNum = gPARA_HeatMeterNum;
	uint16 lu16ValveNum = gPARA_ValveNum;
	uint8 lu8DataBuf[METER_FRAME_LEN_MAX] = {0};
	MeterFileType	mf;
	uint8 lu8DataIn[20] = {0};	//在阀控控制时用于传递阀控控制信息。
	uint8 DataFrame[METER_FRAME_LEN_MAX];
	uint8 DataLen_Vave = 0x00;
	HeatValue_Format HeatValueFile;
	uint16 lu16MeterSn = 0;
	uint16 lu16MeterID = 0;
	

	memcpy((uint8 *)DataPath,(uint8 *)gTAHeatVluePath,sizeof(DataPath));

	for(i=0;i<lu16ValveNum;i++){
		FeedTaskDog();
		//从SD卡中读出
		OSMutexPend (FlashMutex, 0, &Err);
		Err = SDReadData(DataPath,lu8DataBuf,METER_FRAME_LEN_MAX,(lu16HeatMeterNum+i)*METER_FRAME_LEN_MAX);
		OSMutexPost (FlashMutex);
		OSTimeDlyHMSM(0,0,0,300);
		
		if(Err == NO_ERR){
			memcpy(&HeatValueFile,&lu8DataBuf[TA_HEATVALVE_SAVEOFFSET],sizeof(HeatValue_Format));
			//判断下发热量值是否有效
			lu16MeterID = *((uint16 *)(lu8DataBuf+1));
			if(lu16MeterID == HeatValueFile.MeterID){  //如果有效，则下发到阀控器。
				//从SD卡中查找阀控器具体信息，主要是通道号和协议号。
				Err = getValveSn(HeatValueFile.MeterID,&lu16MeterSn);
				if(Err == NO_ERR){
					PARA_ReadMeterInfo(lu16MeterSn, &mf);
					
					if(gu8MBusShortFlag[mf.ChannelIndex - 1] == 0){  //此通道不短路才执行，短路则不执行。
					
						//先切换通道
						METER_ChangeChannel(mf.ChannelIndex);  //切换通道	
						
						FeedTaskDog();

						//下发显示温控面板显示热量值指令。下发分摊热量值到阀控器。
						Err = ValveContron(&mf,SETHEAT_DISPLAY,lu8DataIn,DataFrame,&DataLen_Vave);
						if(Err == NO_ERR){	//向阀控下发热量值。

							memcpy(lu8DataIn,(uint8 *)(&HeatValueFile.ApportionValueSum), 5);
							if(mf.ValveProtocal == VALVE_ADE){  //航天德鲁协议。
								Err = ValveContron(&mf,WRITE_DIVHEATDATA,lu8DataIn,DataFrame,&DataLen_Vave);
							}
							else{
								Err = ValveContron(&mf,SETHEAT_VALUE,lu8DataIn,DataFrame,&DataLen_Vave);
							}
							FeedTaskDog();

							if(Err == NO_ERR){
								//debug_info(gDebugModule[TASKDOWN_MODULE],"%s %d Send HeatMeter data  to valve ok",__FUNCTION__,__LINE__);
							}
							else  {
								//debugX(LOG_LEVEL_ERROR,"%s %d Send HeatMeter data  to valve  failed!\r\n",__FUNCTION__,__LINE__);
							}
					
						}
					}
				}


			}
		
		}
		

	}

	METER_ChangeChannel(7);  //最后断开通道


	
	


}




/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
