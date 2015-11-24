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

extern char gPrintData[1024];	

uint16 VirtualMeterSn = 1;

uint8 HeatCost_JCQ_ReceiveFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen);

uint8 HeatCost_ReadMeterDataTiming(uint16 MeterSn, uint8 *EleBuf,char*path)
{
	uint8 Err 		 	= 0x00;
	uint8 err = 0;
	MeterFileType	mf;
	DELU_Protocol	ProtocoalInfo;
	CPU_SR		cpu_sr;
	DataStoreParaType	InitPara;
	uint8 SystemTime[6] = {0x00};
	char  NodePath[] = "/2012/12/24/timenode";
	memset(&InitPara,0,sizeof(InitPara));
	
	LOG_assert_param(EleBuf == NULL);
	LOG_assert_param(MeterSn > METER_NUM_MAX);
	
    Err = PARA_ReadMeterInfo(MeterSn, &mf);
    if(Err != NO_ERR)
    {
    	LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <METER_ReadMeterDataTiming> Read Meter filed failed, MeterSn is %4XH, Return is %d",MeterSn, Err);
		debug( "WARNING: <METER_ReadMeterDataTiming> Read Meter filed failed, MeterSn is %4XH, Return is %d",MeterSn, Err);
    }
	
	(*METER_ComParaSetArray[gMETER_Table[13][0]])();
	//(*METER_ComParaSetArray[gMETER_Table[mf.ProtocolVer][0]])();
	ProtocoalInfo.PreSmybolNum  = gMETER_Table[mf.ProtocolVer][2];
	
	ProtocoalInfo.MeterType 	= 0xA0;
	ProtocoalInfo.DataIdentifier= gMETER_Table[mf.ProtocolVer][1];
	memset(ProtocoalInfo.DataBuf, 0x00, METER_FRAME_LEN_MAX*2);
	ProtocoalInfo.ControlCode 	= 0x00;
	ProtocoalInfo.Length		= 0x00;

	Err = HeatCost_METER_DataItem(&ProtocoalInfo,path);

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
	}
    OSMutexPost (FlashMutex);
	
	return Err;
}


/****************************************************************************************************
**	函 数  名 称: HeatCost_METER_DataItem
**	函 数  功 能: 抄读分配表采集器的热分配表数据
**	输 入  参 数: DELU_Protocol *pProtocoalInfo -- 抄表的参数输入
					  char*path----- 保存路径
**	输 出  参 数: DELU_Protocol *pProtocoalInfo -- 输出数据复用
**  返   回   值: NO_ERR -- 抄表成功; 1 -- 抄表未成功，超时
**	备		  注: 
*****************************************************************************************************/
uint8 HeatCost_METER_DataItem(DELU_Protocol *pProtocoalInfo,char*path)
{
	uint8 i 					= 0;
	uint8 Err					= 0;
	//uint8 RetryTimes			= 3;
	uint8 RetryTimes			= 2;

	char MeterAddrString[40] 	= {0x00};
	uint8 DataTemp[128]			= {0x00};
	uint8 LenTemp				= 0;
	HeatCost_DataHead stHeatCost_DataHead={1,0};
    uint8 PackageSerial=0;
	
	LOG_assert_param(pProtocoalInfo == NULL);

	PUBLIC_MeterAddrToString(pProtocoalInfo->MeterAddr, MeterAddrString,sizeof(pProtocoalInfo->MeterAddr));
	debug("Reading Meter Addr is %s", MeterAddrString);
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Reading Meter Addr is %s", MeterAddrString);

    for(PackageSerial=0;PackageSerial < stHeatCost_DataHead.PackageTotal;PackageSerial++)
    {     
        if(stHeatCost_DataHead.HavePackage)
        {
          pProtocoalInfo ->Length = 1;
          pProtocoalInfo ->ControlCode = 0x10;
          pProtocoalInfo ->DataBuf[0] = stHeatCost_DataHead.PackageSerial+1;
        }
		else
		{
		  pProtocoalInfo ->Length = 0;
          pProtocoalInfo ->ControlCode = 0x00;
		}
        for(i=0; i<RetryTimes; i++)
     	{
     		Err = HeatCost_METER_MeterCommunicate(pProtocoalInfo, DataTemp, &LenTemp);
     		if(Err == NO_ERR)
 			{
 				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Recived Valid Datas ");
 				Err = HeatCost_METER_DELU_AnalDataFrame(pProtocoalInfo, DataTemp,&stHeatCost_DataHead);
				if(Err == NO_ERR)
				{
					 /*SAVE*/
					memset(DataTemp,0,sizeof(DataTemp));/*清0确保为此次数据*/
					DataTemp[0]	= pProtocoalInfo->Length;						//数据域长度
					memcpy(&DataTemp[1], pProtocoalInfo->DataBuf, pProtocoalInfo->Length);
					DataTemp[pProtocoalInfo->Length+1] = PUBLIC_CountCS(pProtocoalInfo->DataBuf, pProtocoalInfo->Length);
                    /*begin:yangfei modified 2012-12-24 find crruent file path*/
					#if 0
                    memset(gPrintData,sizeof(gPrintData),0);
                    PUBLIC_HexStreamToString(pProtocoalInfo->DataBuf, pProtocoalInfo->Length, gPrintData);
                    strcat(gPrintData,"\n");
					#endif
        			OSMutexPend (FlashMutex,0,&Err);
                    //Err = SDSaveLog(path,gPrintData, strlen(gPrintData),0);
        			Err =SDSaveData(path, DataTemp, METER_FRAME_LEN_MAX,128*(VirtualMeterSn++));
        			OSMutexPost (FlashMutex);
                    memset(gPrintData,sizeof(gPrintData),0);
        			if(Err!=NO_ERR)
        	    	 {
        	    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
        	    	 }
        			/*end   :yangfei modified 2012-12-24*/ 
                    LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <METER_ReadMeterDataTiming> Analysis Datas successful!");
					debug("INFO:HeatCost_METER_MeterCommunicate  Datas successful!");
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
				debug("ERROR:HeatCost_METER_MeterCommunicate  Datas fail!");
 			}
     	}
     	if(i>=RetryTimes)										//重试2次未抄读到数据
     		{
     			return 1;
     		}

    }
 	
 	return NO_ERR;
}

/****************************************************************************************************
**	函 数  名 称: HeatCost_METER_CreateFrame
**	函 数  功 能: 集中器对热计量表数据组帧
**	输 入  参 数: DELU_Protocol *pReadMeter -- 读取热计量表的数据结构
**	输 出  参 数: uint8  *_sendBuf -- 组建出的数据帧; 
**                uint16 *plenFrame -- 组建出的数据帧长度
**  返   回   值: 无
**	备		  注: 本组帧函数，根据热计量表数据帧结构体内信息进行组帧
前导符	0xFE	2	HEX
帧头	    0x68	1	HEX
地址	    A0	1	BCD
　	            A1	1	BCD
　	            A2	1	BCD
　	            A3	1	BCD
标识符	DI	    1	HEX
数据长度	Length	1	HEX
数据域	DATA	N	　
累加和  CS	1	HEX
帧尾	   0x16	1	HEX

*****************************************************************************************************/
void HeatCost_METER_CreateFrame(DELU_Protocol *pData, uint8 *pSendFrame, uint8 *plenFrame)
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
	memcpy(pTemp, pData->MeterAddr, 4);				//组建 地址域
	pTemp += 4;
	lenFrame += 5;
	CheckLen += 5;
	
	*pTemp++ = pData->ControlCode;					//组建 控制码
	*pTemp++ = pData->Length;						//组建 数据域长度
	lenFrame += 2;
	CheckLen += 2;
	
	templen = pData->Length;
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

/****************************************************************************************************
**	函 数  名 称: HeatCost_METER_MeterCommunicate
**	函 数  功 能: 集中器与热计量表通信
**	输 入  参 数: DELU_Protocol *pData -- 需下发的德鲁协议数据帧
**	输 出  参 数: uint8 *pResBuf -- 接收到的数据帧回应; 
**                uint8 *pDataLenBack -- 数据帧长度
**  返   回   值: NO_ERR, ERR_1 -- 没有接收到数据
**	备		  注: 
*****************************************************************************************************/

uint8 HeatCost_METER_MeterCommunicate(DELU_Protocol *pData, uint8 *pResBuf, uint8 *pDataLenBack)
{
	  uint8 err;
//      uint8 *cp = NULL;
      uint8 DataFrame[METER_FRAME_LEN_MAX*2];
      uint8 DataLen = 0x00;
      uint8 dev = DOWN_COMM_DEV_MBUS;
	  uint8 RetryTimes = 3;
	  uint8 i = 0;
	  uint8 ret;
	  
      LOG_assert_param(pData == NULL);
	  LOG_assert_param(pResBuf == NULL);
	  LOG_assert_param(pDataLenBack == NULL);
      
      HeatCost_METER_CreateFrame(pData, DataFrame, &DataLen);
      
      do{
			FeedTaskDog();
			OSSemPend(UpAskMeterSem, 5*OS_TICKS_PER_SEC, &err);                         //申请MBUS的通道
	  }while(err!=OS_ERR_NONE);
	  
	  if(gDebugModule[TASKDOWN_MODULE] >= KERN_DEBUG)
		{
			PUBLIC_HexStreamToString(DataFrame, DataLen, gPrintData);
			debug_debug(gDebugModule[TASKDOWN_MODULE],"Meter send data:%s\r\n",gPrintData);	
		}
	
   	  //cp=(uint8*)Uart0SendThenReceive_PostPend(DataFrame, &DataLen);
   	 for(i=0;i<RetryTimes;i++)
     {
     	 
		 DuQueueFlush(dev);   										//清空缓冲区 	
	     DuSend(dev, DataFrame, DataLen);

		 ret = HeatCost_JCQ_ReceiveFrame(dev, DataFrame,2, &DataLen);
		 
		 if(ret == NO_ERR)
		 	{
		 	 memcpy(pResBuf, DataFrame, DataLen);
      		 *pDataLenBack = DataLen;
		 	 debug_debug(gDebugModule[HEAT_COST],"HeatCost_JCQ_ReceiveFrame OK!\r\n");	
			 break;
		 	}
		 else
		 	{
		 	debug_err(gDebugModule[HEAT_COST],"ERR:HeatCost_JCQ_ReceiveFrame ret = %d!\r\n",ret);	
		 	}
   	 }
   	 OSSemPost(UpAskMeterSem); 

     return NO_ERR;
}

/****************************************************************************************************
**	函 数  名 称: HeatCost_METER_AnalDataFrame
**	函 数  功 能: 集解析读取来的热分配表数据
**	输 入  参 数: Duint8 *pRecFrame -- 输入的待解帧的数据
**	输 出  参 数: DELU_Protocol *pProtoclData -- 解帧后的数据结构
**  返   回   值: NO_ERR
**	备		  注: 
*****************************************************************************************************/

uint8 HeatCost_METER_DELU_AnalDataFrame(DELU_Protocol *pProtoclData, uint8 *pRecFrame,HeatCost_DataHead *pHeatCost_DataHead)
{
	uint8 *pTemp 		= pRecFrame;
	
	LOG_assert_param(pProtoclData == NULL);
	LOG_assert_param(pRecFrame == NULL);
    
	
	if(0x68 != *pTemp++)	return 1;	
	
	memcpy(pProtoclData->MeterAddr, pTemp, 4);								//仪表地址
	pTemp += 4;
	
	pProtoclData->ControlCode	= *pTemp++;									//控制码
	pProtoclData->Length		= *pTemp++;									//长度

    if((pProtoclData->Length - sizeof(HeatCost_DataHead))<=0)
      {
        debug_err(gDebugModule[HEAT_COST],"ERROR: HeatCost_METER is %d!", pProtoclData->Length);
        return ERR_1;
      }
   /*AnalData
   PackageTotal(1B)+PackageSerial(1B)+HavePackage(1B)+MeterTotal(1B)+Timenode(5B)+ MeterTotal*(Address(2B)+LEN(1B)+Data(LEN B))
   */ 
    pHeatCost_DataHead->PackageTotal = *pTemp++;	
    pHeatCost_DataHead->PackageSerial = *pTemp++;	
    pHeatCost_DataHead->HavePackage = *pTemp++;	
    pHeatCost_DataHead->MeterTotal = *pTemp++;	
    memcpy(pHeatCost_DataHead->Timenode, pTemp, sizeof(pHeatCost_DataHead->Timenode));	
    pTemp += sizeof(pHeatCost_DataHead->Timenode);
    
	memcpy(pProtoclData->DataBuf, pTemp, (pProtoclData->Length - sizeof(HeatCost_DataHead)));			//数据域数据
	pTemp += pProtoclData->Length;
    pProtoclData->Length -= sizeof(HeatCost_DataHead);  /*减去包头，计算得到实际BUF长度*/

	return NO_ERR;
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

uint8 HeatCost_JCQ_ReceiveFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen)
{
	uint8 data 	= 0x00;
	uint8 len	= 0x00;
	uint8 Cs	= 0x00;
	uint32 i,j;
	
	LOG_assert_param(dev != 0x00);
	LOG_assert_param(buf == NULL);
	LOG_assert_param(datalen == NULL);
	
	*datalen = 0;
		
	i = 30;
	while(i--)														//找帧头
	{
		FeedTaskDog();   
		if(DuGetch(dev, &data, Out_Time))  			{return 1;}
		if(data==0x68)	break; 	     
	}
	Cs 		= data;
	*buf++ 	= data;
	
    for(i=0; i<4; i++)												//地址
    {
		if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  {return 3;}	
		Cs 		+= data;
		*buf++	 = data;
	}
	
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC)) {return 4;}		//控制码		
	Cs 		+= data;
	*buf++	 = data;
	
	if(DuGetch(dev, &len, OS_TICKS_PER_SEC))  		{return 5;}		//数据长度
	
	if(len > METER_FRAME_LEN_MAX)             		{return 10;}
	Cs 		+= len;
	*buf++	 = len;

	for(j=0; j<len; j++)											//数据域
	{
	    if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  {return 6;}	
		*buf++  = data;
		Cs     += data;
    }  
     
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  		{return 7;}		//校验字节
	if(data != Cs)         							{return 11;}   
	*buf++	= data; 
	
	if(DuGetch(dev, &data, OS_TICKS_PER_SEC))  		{return 8;}		//结束符
	if(data != 0x16)      {return 12;}   						
		
	   
  	*buf++		= data; 
  	*datalen 	= len + 9;
  	
    return NO_ERR;
}

/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
