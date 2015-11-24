/*******************************************Copyright (c)*******************************************
         							山东华宇空间技术公司(西安分部)                                                 
**  文   件   名: StoreManage.c
**  创   建   人: 勾江涛
**  创 建  日 期: 2012.08.07 
**  描        述: 系统数据存储管理，
**  修 改  记 录:   	
*****************************************************************************************************/

/********************************************** include *********************************************/
#include <includes.h>
#include "app_down.h"
#include "HZTable.h"
extern uint8 gRestartFlag ;

/********************************************** extern *********************************************/

/********************************************** global *********************************************/
// 1, 由于存在C++ 调用 C 的情况，所以这些全局索引变量，均定义在.C文件中。
// 2, 由于全局变量数据分散加载到外部SRAM中，所以统一定义在该文件中.
// 3, 定义在外部SRAM中的全局变量，需在应用前，系统运行前必须用函数进行初始化操作，编译器不能自动初始化
// 4, 这些全局变量需要在开机初始化，初始化时分sd卡中是否写过数据和未写过数据2种情况
/************************************** HeatMeterFile *********************************************/
uint16 gPARA_MeterNum 			= 0;							//当前表档案总数量
MeterFileType gPARA_MeterFiles[METER_NUM_MAX];					//内存中存储当前表档案信息
uint8 gPARA_SendNum_Prevous		= 0;							//备份上一包发送的表档案个数
uint8 gPARA_SendIndex_Prevous	= 0;							//备份上一包数据包的序列号
/*begin:yangfei added 2013-03-25 for add leye 485 meter*/
uint16 gPARA_MeterChannel[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];			//6个通道热计量表信息, MeterSn
uint8  gPARA_Meter_Failed[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];	/*7个通道失败记录*/
uint16 gPARA_MeterChannelNum[METER_CHANNEL_NUM];								//每个通道表数量
uint16 gPARA_ConPanelChannel[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];			//6个通道每个通道的温控面板信息, MeterSn
uint16 gPARA_ConPanelChannelNum[METER_CHANNEL_NUM];								//每个通道加载的温控面板数量
uint16 gPARA_ValveConChannel[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];			//6个通道每个通道的阀门信息, MeterSn
uint16 gPARA_ValveConChannelNum[METER_CHANNEL_NUM];								//每个通道加载的阀门数量
/*end:yangfei added 2013-03-25 for add leye 485 meter*/
uint16 gPARA_HeatMeterNum = 0; //集中器中热表总数。
uint16 gPARA_ValveNum = 0; //集中器中阀控器总数。
uint16 gu16ReadHeatMeterNum  = 0; //抄表过程中，已经抄的热表数量计数。
uint16 gu16ReadValveNum = 0;	  //抄表过程中，已经抄的阀控器数量计数。



/************************************** 抄表时间节点 **********************************************/
uint16 gPARA_TimeNodes[TIME_NODE_NUM_MAX]	 = {0};				//当前抄表时间点，在内存中的备份
uint16 gPARA_TimeNodesNum = 0;			//存放设置的抄表时间点数量。
/************************************** 集中器基本参数 ********************************************/
TermParaSaveType gPARA_TermPara;								//集中器基本参数
UpCommParaSaveType	gPARA_UpPara;								//上行通讯基本参数
CommPara uPCommPara;//muxiaoqing
ReplenishReadMeterSaveType gPARA_ReplenishPara;  //补抄表参数设置,added by zjjin.


GlobalPara gStore_CommPara;
/************************************** SystemLog *************************************************/
uint8 gLOG_SystemLog[1024] = {0x00};							//记录系统运行日志
OS_EVENT *LogQueSem; 											//产生新日志的信号量
OS_EVENT *LogWriteSem;											//请求写入日志的信号量	
LCDContrastPara NVLCDConraston;

/********************抄表信息查看与轮显用到的变量****************************/
uint8 gu8ReadAllMeterFlag = 0;   //标记当前是否正在抄全表，0-不在，1-正在全抄。
uint8 gu8ReadDataStore[METER_FRAME_LEN_MAX] = {0};  //用于存放刚刚抄过的表的信息，用于轮显或查询。



/********************MBUS短路保护用到的全局变量*********************************/
uint8 gu8MBusShortRising = 0;  //MBUS短路指示引脚是否出现上升沿，0-没有，1-出现上升沿。
uint8 gu8MBusShortFlag[METER_CHANNEL_NUM] = {0};  //标记对应MBUS通道通道短路,1-短路，0-不短路。
uint8 gu8NowMBUSChannel = 0;    //记录当前MBUS通道。

uint8 gu8MBusShortTime = 0; //检测MBUS通道短路振荡时间记录。

/********************抄阀控器失败标记***********************************************/
uint8 gu8ReadValveFail = 0;  //标记抄阀控器是否失败，0-没失败，1-失败。


uint8 gu8ErrFlag = 0;//标记是否有故障发生，有故障则蜂鸣器响。1-有故障。
uint16 gu16ErrOverTime = 0;//蜂鸣器响时间，响一定时间后则停止。


//通断时间面积法中，将上位机发送来的各用户分摊值下发到各用户的阀控，
//此全局数组路径用于记录分摊值存放路径。
char  gTAHeatVluePath[30] = {0};  //路径长度正常不超过20字节，注意不要超限。





#ifdef DEMO_APP

uint8  DemoDateIsGet=0;
#endif


/********************************************** static *********************************************/

/********************************************** extern *********************************************/
extern uint16 gUpdateBegin ;
#if 0 /*此函数没用*/
/****************************************************************************************************
**	函 数  名 称: STORE_GetAddrOfMeterData
**	函 数  功 能: 获取某个热计量表的数据或参数的存储位置
**	输 入  参 数: uint8 *pTime -- 需查找的时间; uint16 MeterSn -- 热计量表序号; 
**				  uint8 Flag -- FIND_HISTORY, FIND_CURRENT, FIND_DATA, FIND_PARA
**	输 出  参 数: uint32 *AddrOut -- 输出SD卡存储的位置地址
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
uint8 STORE_GetAddrOfMeterData(uint8 *pTime, uint16 MeterSn, uint32 *AddrOut, uint8 Flag)
{
	uint8  AddrErrFlag = 0;							//返回各种错误标志
	uint8  ReadTime[6] = {0};						//需读取的时间，小时，日，月，年为有效数据
	uint32 TimeOffSet  = 0;							//所需读取的时间与基准时间偏移量
	uint8  MonthHEX	   = 1;							//月份变量，用于判断条件，是否在供暖季
	
	LOG_assert_param(pTime == NULL);
	LOG_assert_param(AddrOut == NULL);
	LOG_assert_param(MeterSn > METER_NUM_MAX);
	
	memcpy(ReadTime, pTime, 6);
	if(TimeCheck(ReadTime))			return ERR_1;	//检验时间正确性
	
	MonthHEX = BcdToHex(ReadTime[ARRAY_MONTH]);	
	
	AddrErrFlag  = STORE_CalcTime(ReadTime, &TimeOffSet, Flag);	//获取需读取的时间与基准时间的偏移量，以一个时间节点为单位
	if((MonthHEX >= 0x0B) && (MonthHEX <= 0x03))	//判断是否在采暖季，采暖季每个月的数据都存储
		{
			if(AddrErrFlag == NO_ERR)
				{
					if( (Flag&0xF0) == FIND_DATA )
						{
							*AddrOut    = ( METERS_DATA_STORE_ADDR + \
						   				(TimeOffSet*METERS_DATA_LEN) + ((MeterSn+1)*METER_FRAME_LEN_MAX) );
						}
					if( (Flag&0xF0) == FIND_PARA )
						{
							*AddrOut    = METERS_DATA_STORE_ADDR + (TimeOffSet*METERS_DATA_LEN);
						}
				}
			else
				{
					*AddrOut	= LOGIC_ADDR_NULL;
				}
		}
	else												//非采暖季，只分配一个月公用的存储空间
		{
			if( (Flag&0xF0) == FIND_DATA )
				{
					*AddrOut    = ( METERS_DATA_STORE_PUBLIC + \
				   				(TimeOffSet*METERS_DATA_LEN) + ((MeterSn+1)*METER_FRAME_LEN_MAX) );
				}
			if( (Flag&0xF0) == FIND_PARA )
				{
					*AddrOut    = METERS_DATA_STORE_PUBLIC + (TimeOffSet*METERS_DATA_LEN);
				}
		}
	
	return AddrErrFlag;
}
#endif
/****************************************************************************************************
**	函 数  名 称: PARA_ReadMeterInfo
**	函 数  功 能: 获取某个热计量表的表档案信息
**	输 入  参 数: uint16 MeterSn -- 热计量表序号; 
**	输 出  参 数: MeterFileType *pMeterFile -- 输出表档案信息
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
uint8 PARA_ReadMeterInfo(uint16 MeterSn, MeterFileType *pMeterFile)
{
	uint8 Err = NO_ERR;
	CPU_SR	cpu_sr;
	LOG_assert_param(MeterSn > METER_NUM_MAX);
	LOG_assert_param(pMeterFile == NULL);

	OS_ENTER_CRITICAL();
	memcpy((uint8 *)&(pMeterFile->MeterID), (uint8 *)&(gPARA_MeterFiles[MeterSn].MeterID), sizeof(MeterFileType));
	OS_EXIT_CRITICAL();
	
	return Err;
}




/****************************************************************************************************
**	函 数  名 称: STORE_GetTimeNodeInfo
**	函 数  功 能: 指定时间在当前抄表时间节点配置下的时间节点序号，和与最近时间节点的差多少分钟
**	输 入  参 数: uint16 InTime 
**      	 	  uint8 *pNodeIndex 
**			      int8 *pNodeOffset
**  输 出  参 数: 
**  返   回   值: NO_ERR;
**	备		  注: 在现有逻辑下   上面减法不可能出现负数
*****************************************************************************************************/
uint8 STORE_GetTimeNodeInfo(uint16 InTime, uint8 *pNodeIndex, int8 *pNodeOffset)
{
	uint8 Err				= 0x00;
	uint8 MinuteBinIn 		= 0x00;
	uint8 MinuteBinNode 	= 0x00;
	uint8 NodeIndex			= 0x00;
	uint16 TimeNodeCur[TIME_NODE_NUM_MAX] 	= {0x00};
	CPU_SR		cpu_sr;

	LOG_assert_param( pNodeIndex == NULL );
	LOG_assert_param( pNodeOffset == NULL );

	OS_ENTER_CRITICAL();
    memcpy((uint8 *)TimeNodeCur, (uint8 *)gPARA_TimeNodes, sizeof(gPARA_TimeNodes));
    OS_EXIT_CRITICAL();
    
    Err = STORE_FindTimeNodeIndex(InTime, TimeNodeCur, &NodeIndex);
	*pNodeIndex = NodeIndex;
    
    //做时间差几分钟的比较
    if(Err == NO_ERR)
    	{
    		if( (InTime&0xFF00) == (TimeNodeCur[NodeIndex]&0xFF00))
				{
					MinuteBinIn	=  BcdToHex(InTime);
					MinuteBinNode =  BcdToHex(TimeNodeCur[NodeIndex]);
					*pNodeOffset = 	MinuteBinIn - MinuteBinNode;
				}
    	}
	
    return Err;
}

/****************************************************************************************************
**	函 数  名 称: STORE_FindTimeNodeIndex
**	函 数  功 能: 时间节点的索引
**	输 入  参 数: uint16 InTime 
**      	 	  uint16 *pTimeNode 
**			      uint8 *pNodeIndexOut
**  输 出  参 数:
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
uint8 STORE_FindTimeNodeIndex(uint16 InTime, uint16 *pTimeNode, uint8 *pNodeIndexOut)
{
	uint8 i=0;
	uint8 FindFlag	 = 0x00;
	uint8 CheckBinHour = 0x00;
	uint8 CheckBinMinute = 0x00;
	uint16 TimeNodeCur[TIME_NODE_NUM_MAX] = {0x00};
    
    LOG_assert_param( pTimeNode == NULL );
	LOG_assert_param( pNodeIndexOut == NULL );
	memcpy((uint8 *)TimeNodeCur, (uint8 *)pTimeNode, (TIME_NODE_NUM_MAX)*sizeof(uint16));

    for(i=0; i<TIME_NODE_NUM_MAX; i++)
    {
    	//做时间的BCD检查
		CheckBinMinute	=  BcdToHex(TimeNodeCur[i]);
		CheckBinHour =  BcdToHex(TimeNodeCur[i]>>8);
		if( (CheckBinHour>0x18) || (CheckBinMinute>=0x3C))
			{
				return 1;							//时间节点错误
			}

    	if( (TimeNodeCur[i] <= InTime) && (InTime < TimeNodeCur[i+1]) )
    		{
    			FindFlag = i;
    			break;
    		}
		if( (i==(TIME_NODE_NUM_MAX-1)) && (TimeNodeCur[i] <= InTime) ) //最后一个时间节点
			{
			 	FindFlag = i;
				break;
			}

    }
    FindFlag = i;			//防止编译器优化掉
    if(FindFlag < TIME_NODE_NUM_MAX)
    	{
    		*pNodeIndexOut = FindFlag;				//找到某个时间节点
    	}
    else
    	{											//未找到相应的时间节点
    		return 3;
    	}
    	
    return NO_ERR;
}


/****************************************************************************************************
**	函 数  名 称: FindTimeNodeIndex_Forward
**	函 数  功 能: 获取时间节点的索引
**	输 入  参 数: uint16 InTime 
**      	 	  uint16 *pTimeNode 
**			      uint8 *pNodeIndexOut
**  输 出  参 数:
**  返   回   值: NO_ERR;
**	备		  注: 给定时间，查询获取给定时间之前的最近时间节点索引，
**                         如果当天查询不到，则给出前一天最后一个时间节点索引。
*****************************************************************************************************/
uint8 FindTimeNodeIndex_Forward(uint16 InTime,DataStoreParaType *datastorePara, uint8 *pNodeIndexOut,uint8 *pYesterday)
{
	uint8 i=0;
	uint8 FindFlag	 = 0x00;
	uint8 CheckBinHour = 0x00;
	uint8 CheckBinMinute = 0x00;
	uint16 TimeNodeCur[TIME_NODE_NUM_MAX] = {0x00};
	uint16 lu16MinNode = 0;  
	uint16 lu16MaxNode = 0;
	uint8 lu8MaxNodeIndex = 0;
    
    LOG_assert_param( datastorePara == NULL );
	LOG_assert_param( pNodeIndexOut == NULL );
	memcpy((uint8 *)TimeNodeCur, (uint8 *)datastorePara->TimeNode, (TIME_NODE_NUM_MAX)*sizeof(uint16));


	lu16MinNode = TimeNodeCur[0];
	lu16MaxNode = TimeNodeCur[0];
	for(i=0; i<TIME_NODE_NUM_MAX; i++){   //找到最小最大的定时抄表时间点。
		if(TimeNodeCur[i] != 0xffff){
			if(	lu16MinNode > TimeNodeCur[i])
				lu16MinNode = TimeNodeCur[i];

			if(lu16MaxNode < TimeNodeCur[i]){
				lu16MaxNode = TimeNodeCur[i];
				lu8MaxNodeIndex = i;
				}

			}
	}
		

    for(i=0; i<TIME_NODE_NUM_MAX; i++)
    {
    	//做时间的BCD检查
		CheckBinMinute	=  BcdToHex(TimeNodeCur[i]);
		CheckBinHour =  BcdToHex(TimeNodeCur[i]>>8);
		if( (CheckBinHour>0x18) || (CheckBinMinute>=0x3C))
			{
				return 1;							//时间节点错误
			}

		if(lu16MinNode >= InTime)
    		{
    			FindFlag = lu8MaxNodeIndex;
				*pYesterday = 1;  //昨天的最后一个时间点。
    			break;
    		}

    	if( (TimeNodeCur[i] <= InTime) && (InTime < TimeNodeCur[i+1]) )
    		{
    			FindFlag = i;
    			break;
    		}
		if( (i==(TIME_NODE_NUM_MAX-1)) && (TimeNodeCur[i] <= InTime) ) //最后一个时间节点
			{
			 	FindFlag = i;
				break;
			}

    }
    FindFlag = FindFlag;			//防止编译器优化掉
    if(FindFlag < TIME_NODE_NUM_MAX)
    	{
    		*pNodeIndexOut = FindFlag;				//找到某个时间节点
    	}
    else
    	{											//未找到相应的时间节点
    		return 3;
    	}
    	
    return NO_ERR;
}


/****************************************************************************************************
**	函 数  名 称: StoreYesterday
**	函 数  功 能: 记住昨天，将昨天的日期存入今天，方便查询使用。
**	输 入  参 数: 
**      	 	 
**  输 出  参 数:
**  返   回   值: 
**	备		  注: 
*****************************************************************************************************/
/********************昨天抄表数据存储路径记录全局变量**********************/
char  gcYesterdayPath[] = "/2015/06/09/1200";

uint8 StoreYesterday(void)
{
	uint8 Err = 0;
	char  YesterdayPath[] = "/2015/06/09/yesterday";
	uint8 SystemTime[6] = {0x00};
	

	memcpy(SystemTime, gSystemTime, 6);
	//先将昨天路径存入今天路径中，以备查询使用。
	GetFilePath(YesterdayPath, SystemTime,ARRAY_DAY);
	OSMutexPend (FlashMutex,0,&Err);
	
	Err = SDSaveData(YesterdayPath, (uint8 *)gcYesterdayPath, sizeof(gcYesterdayPath),0);
	if(Err!=NO_ERR)
	{
		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
	}
	OSMutexPost (FlashMutex);
	GetFilePath(gcYesterdayPath, SystemTime,ARRAY_DAY);  //更新为今天，备明天用

	if(Err == NO_ERR)
		return 0;
	else
		return 1;


}



/****************************************************************************************************
**	函 数  名 称: STORE_InitTimeNodePara
**	函 数  功 能: 初始化时间节点的参数
**	输 入  参 数: uint8 *pDateTime 
**      	 	  uint8 NodeIndex 
**  输 出  参 数:
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
uint8 STORE_InitTimeNodePara(uint8 *pDateTime, uint8 NodeIndex)
{
	uint8 Err			= 0x00;
	//uint32 LogicAddr	= 0xFFFFFFFF;
	DataStoreParaType	InitPara;
	CPU_SR		cpu_sr;
	/*begin:yangfei added 2012-12-24 find path*/
	uint8 SystemTime[6] = {0x00};
	char  NodePath[] = "/2012/12/24/timenode";

	memset(&InitPara,0,sizeof(InitPara));
	/*end   :yangfei added 2012-12-24 */
    LOG_assert_param( pDateTime == NULL );	
	memcpy(InitPara.DataRecordTime, pDateTime, 6);
	InitPara.DataRecordTime[ARRAY_SECOND]		= 0x00;	
     //memset(InitPara.ReadCmplFlag, 0x00, 75);
	READ_ParaInit();

	/*begin:yangfei added 2012-12-24 find path*/
	
	if(!NodeIndex)		//只允许每天一开始的时间节点数据有效
		{
			OS_ENTER_CRITICAL();
			InitPara.MeterNums = gPARA_MeterNum;
			memcpy((uint8 *)InitPara.TimeNode, (uint8 *)gPARA_TimeNodes, sizeof(gPARA_TimeNodes));
			memcpy(SystemTime, gSystemTime, 6);
			OS_EXIT_CRITICAL();


			GetFilePath(NodePath, SystemTime,ARRAY_DAY);
			OSMutexPend (FlashMutex,0,&Err);
			Err =SDSaveData(NodePath, &InitPara, sizeof(DataStoreParaType),0);
			if(Err!=NO_ERR)
	    	{
	    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
	    	}
		    OSMutexPost (FlashMutex);
		}
	#if 0
	else
		{
			memset((uint8 *)InitPara.TimeNode, 0xFF, sizeof(gPARA_TimeNodes));
		}
	STORE_GetAddrOfMeterData(pDateTime, 0, &LogicAddr, FIND_CURRENT|FIND_PARA);

	OSMutexPend (FlashMutex,0,&Err);
    Err =AppSdWrRdBytes(LogicAddr, sizeof(DataStoreParaType), (uint8 *)&InitPara, SD_WR);
    OSMutexPost (FlashMutex);
	#endif
	/*end   :yangfei added 2012-12-24 */
	return 0;
}
#if 0
/****************************************************************************************************
**	函 数  名 称: STORE_FindDayStartAddr
**	函 数  功 能: 查找开始地址
**	输 入  参 数: uint8 *pReadTime 
**      	 	  uint32 *pAddrOut 
**  输 出  参 数:
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
uint8 STORE_FindDayStartAddr(uint8 *pReadTime, uint32 *pAddrOut)
{
	//先找到需查找到时间节点的 年，月，日
	//再在日内遍历查找是否有该时, 分的抄表存储节点
	uint8 YearHex	= 0x00;
	uint8 MonthHex	= 0x01;
	uint8 DayHex	= 0x01;
	uint8 ReadTime[6] = {0x00};
	uint32 CalcTemp	= 0x00;
	uint16 CalcYearTemp = 0x00;
	uint16 CalcMonthTemp = 0x00;
	uint16 CalcDayTemp = 0x00;
	uint16 TimeTempSource = 0x00;

    LOG_assert_param( pReadTime == NULL );
	LOG_assert_param( pAddrOut == NULL );	
	
	memcpy(ReadTime, pReadTime, 6);
	if(TimeCheck(ReadTime))				return 1;
	
	YearHex 	= BcdToHex(ReadTime[ARRAY_YEAR]);
	MonthHex	= BcdToHex(ReadTime[ARRAY_MONTH]);
	DayHex		= BcdToHex(ReadTime[ARRAY_DAY]);
	TimeTempSource 	= ReadTime[ARRAY_MINUTE] + (ReadTime[ARRAY_HOUR]<<8);
	
	//判断是否是供暖期，若不是  则直接返回
	if((MonthHex >= 0x0B) && (MonthHex <= 0x03))
		{
			CalcYearTemp 	= (YearHex-0x0C)*5*31;
			
			if(MonthHex >= 0x0B)
				{
					CalcMonthTemp = (MonthHex - 0x0B)*31;
				}
			if(MonthHex <= 0x03)
				{
					CalcMonthTemp = (2 + MonthHex - 1)*31;
				}
			
			CalcDayTemp = (DayHex - 0x01)*TIME_NODE_NUM_MAX;
			
			CalcTemp = CalcYearTemp + CalcMonthTemp + CalcDayTemp;
		
		//已经定位到某一天的存储起始地址, 在每天的起始存储地址，前128字节为存储特定参数
			*pAddrOut = METERS_DATA_STORE_ADDR + CalcTemp*METERS_DATA_LEN;
		}
		else
		{
			CalcTemp = (DayHex - 0x01)*TIME_NODE_NUM_MAX;
			*pAddrOut = METERS_DATA_STORE_PUBLIC + CalcTemp*METERS_DATA_LEN;
		}
	return NO_ERR;
}
#endif
/****************************************************************************************************
**	函 数  名 称: PARA_InitGlobalVar
**	函 数  功 能: 对集中器参数进行初始化
**	输 入  参 数: none
**  输 出  参 数:
**  返   回   值: NO_ERR;
**	备		  注: 
*****************************************************************************************************/
void PARA_InitGlobalVar(void)
{
	gPARA_MeterNum 			= 0;
	gPARA_SendIndex_Prevous	= 0;
	gSD_Err = 0x00;
	gUpdateBegin = 0;
	gCurrent_Channel = 0;
    /*begin:yangfei added 2013-04-02 for wait sd ok*/
    //delay(0x007fffff);
    /*end:yangfei added 2013-04-02 for wait sd ok*/
	mountSd();                    /* 挂载SD  卡*/  

	GetVipDataFormSD();
}

/****************************************************************************************************
**	函 数  名 称: GetVipDataFormSD
**	函 数  功 能: 对集中器中SD卡存储的关键信息，进行内存索引生成，以提高效率
**	输 入  参 数: none
**  输 出  参 数:
**  返   回   值: NO_ERR;
**	备		  注: 以下数据，均需要在SD卡中判断是否写入标志(writeFlag)，
**                若该参数未写入过，则调用该参数的初始化函数;
**                若写入过参数，则读取参数数据，最好后期能做参数有效性判断和正确范围判断。
**                对于表档案等信息做CRC校验来保证数据的正确性。
*****************************************************************************************************/
void GetVipDataFormSD(void)
{
	uint16 i	= 0;
	uint8 Err 	= 0;
	uint8 ChannelTemp = 0;
	uint8 mfChannel	  = 0;
	uint8 SystemTime[6] = {0x00};
	uint16 TimeNode[TIME_NODE_NUM_MAX+2] = {0x00};

		
#ifdef DEMO_APP
	
//	uint8 addrtemp_DeLu[7] = {0x89, 0x57, 0x10, 0x21, 0x00, 0x11, 0x11};
//	uint8 addrtemp_LanJE[7] = {0x99, 0x44, 0x88, 0x23, 0x00, 0x00, 0x00};
//	uint8 addrtemp_TG[7] = {0x99, 0x44, 0x88, 0x23, 0x00, 0x00, 0x00};
#endif			

	//uint8 addrtemp[7] = {0x89, 0x57, 0x10, 0x21, 0x00, 0x11, 0x11};
//	uint8 addrtemp[7] = {0x99, 0x44, 0x88, 0x23, 0x00, 0x00, 0x00};
	//uint32 LogicAddr = LOGIC_ADDR_NULL;
	MeterFilePara	para;
	MeterFileType mf;
	TermParaSaveType TermPara;
	UpCommParaSaveType	UpPara;
	ReplenishReadMeterSaveType ReplenishPara;
	//DataStoreParaType	DataPara;
	CPU_SR			cpu_sr;
	uint8  counter=0;
	/*begin:yangfei modified 2012-12-24 find path*/
	char  NodePath[] = "/2012/12/24/timenode";
	/*end   :yangfei modified 2012-12-24*/
	//初始化集中器参数

	
	/*begin:yangfei added 2013-4-3 for 第一次读取会失败，防止下边的失败*/
     Err=MakeFile("/test");
	if(Err)
		{
		
		}
	else
		{
	
            do{
 //           	OSMutexPend (FlashMutex,0,&Err);
            	Err = SDReadData("/test", &TermPara, sizeof(TermParaSaveType), 0);
 //               OSMutexPost (FlashMutex);
				
            	if(Err!=NO_ERR)	
            	  {
            	   // debug("%s %d read test err=%d!\r\n",__FUNCTION__,__LINE__,Err);
            	   counter++;
            	   
            	  }
				if (counter == 5)
					{
					counter =0;
					break;
					}
            }while(Err!=NO_ERR);
		}
	
	  
    /*end:yangfei modified 2013-4-3 */
	//OSMutexPend (FlashMutex,0,&Err);
	Err = SDReadData("/TERMINAL_PARA", &TermPara, sizeof(TermParaSaveType), 0);
    //OSMutexPost (FlashMutex);
#ifdef DEMO_APP
	
	DemoDateIsGet =0;  
#endif
	if(Err!=NO_ERR)	{
	    debug("%s %d read TERMINAL_PARA err=%d!\r\n",__FUNCTION__,__LINE__,Err);
	  }
	
	if(TermPara.WriteFlag == 0xAA55){
			gPARA_TermPara = TermPara;
			
			//gPARA_TermPara.TermAddr[2]	= 0;
			//gPARA_TermPara.TermAddr[3]	= 0; 
			//gPARA_TermPara.TermAddr[4]	= 0;
			//gPARA_TermPara.TermAddr[5]	= 0; 

			//gPARA_TermPara.HostAddr[2]	= 0;
			//gPARA_TermPara.HostAddr[3]	= 0; 
			//gPARA_TermPara.HostAddr[4]	= 0;
			//gPARA_TermPara.HostAddr[5]	= 0; 
			
			
		}
	else{
			memset(gPARA_TermPara.TermAddr, 0x00, 6);
			memset(gPARA_TermPara.HostAddr, 0x00, 6);
			gPARA_TermPara.TermAddr[0]	= 0x34;
			gPARA_TermPara.TermAddr[1]	= 0x12; 
			//gPARA_TermPara.TermAddr[2]	= 0;
			//gPARA_TermPara.TermAddr[3]	= 0; 
			//gPARA_TermPara.TermAddr[4]	= 0;
			//gPARA_TermPara.TermAddr[5]	= 0; 

			gPARA_TermPara.HostAddr[0]	= 0x01;
			gPARA_TermPara.HostAddr[1]	= 0x00;
			//gPARA_TermPara.HostAddr[2]	= 0;
			//gPARA_TermPara.HostAddr[3]	= 0; 
			//gPARA_TermPara.HostAddr[4]	= 0;
			//gPARA_TermPara.HostAddr[5]	= 0; 
			/*begin:yangfei added 20140224 for add support time_on_off*/
			gPARA_TermPara.DeviceType   = HEAT_METER_TYPE;
			/*end:yangfei added 20140224 for add support time_on_off*/
		}
	/*begin:yangfei modified 2013-10-14 for modified send max number*/
	   gPARA_TermPara.DebugPara.FrameLen_Max = 450;
	   //gPARA_TermPara.DebugPara.FrameLen_Max = PACK_MAX;
	/*end:yangfei modified 2013-10-14 for modified send max number*/


//begin: 初始化补抄表参数(补抄次数及时间间隔)。
	Err = SDReadData("/REPLENISH_PARA", &ReplenishPara, sizeof(ReplenishPara), 0);
	if(Err!=NO_ERR) {
		  debug("%s %d read REPLENISH_PARA err=%d!\r\n",__FUNCTION__,__LINE__,Err);
	}
	if(ReplenishPara.WriteFlag == 0xAA55){
		gPARA_ReplenishPara = ReplenishPara;

		//防错检查。
		if(gPARA_ReplenishPara.MeterReplenishTimes>9)
			gPARA_ReplenishPara.MeterReplenishTimes = 2;
		if((gPARA_ReplenishPara.MeterInterval>9999) || (gPARA_ReplenishPara.MeterInterval<100))
			gPARA_ReplenishPara.MeterInterval = 200;
		if(gPARA_ReplenishPara.ValveReplenishTimes>9)
			gPARA_ReplenishPara.ValveReplenishTimes = 2;
		if((gPARA_ReplenishPara.ValveInterval>9999) || (gPARA_ReplenishPara.ValveInterval<100))
			gPARA_ReplenishPara.ValveInterval = 500;
	}
	else{
		gPARA_ReplenishPara.MeterReplenishTimes = 2;  //默认表补抄2次。
		gPARA_ReplenishPara.MeterInterval = 200;	 //默认抄表时间间隔200ms。
		gPARA_ReplenishPara.ValveReplenishTimes = 2;  //默认阀补抄2次。
		gPARA_ReplenishPara.ValveInterval = 500;	 //默认抄阀时间间隔500ms。


	}

//end: 初始化补抄表参数(补抄次数及时间间隔)。
	   

	//初始化表档案信息
	//OSMutexPend (FlashMutex,0,&Err);
	Err = SDReadData("/METER_FILE_PARA", &para, sizeof(MeterFilePara), 0);
   // OSMutexPost (FlashMutex);
	if(Err!=NO_ERR)	 {
	    debug("%s %d read METER_FILE_PARA err=%d!\r\n",__FUNCTION__,__LINE__,Err);
	 }
	
	if(para.WriteFlag == 0xAA55){				//已经写入了有效参数数据
			memset((uint8 *)gPARA_MeterFiles, 0x00, sizeof(gPARA_MeterFiles));
			memset((uint8 *)gPARA_MeterChannel, 0x00, sizeof(gPARA_MeterChannel));
			memset((uint8 *)gPARA_Meter_Failed, 0x00, sizeof(gPARA_Meter_Failed));/*yangfei added 2013-11-17*/
			memset((uint8 *)gPARA_MeterChannelNum, 0x00, sizeof(gPARA_MeterChannelNum));
			memset((uint8 *)gPARA_ConPanelChannel, 0x00, sizeof(gPARA_ConPanelChannel));
			memset((uint8 *)gPARA_ConPanelChannelNum, 0x00, sizeof(gPARA_ConPanelChannelNum));
			memset((uint8 *)gPARA_ValveConChannel, 0x00, sizeof(gPARA_ValveConChannel));
			memset((uint8 *)gPARA_ValveConChannelNum, 0x00, sizeof(gPARA_ValveConChannelNum));
			gPARA_HeatMeterNum = 0; //集中器中热表总数。
			gPARA_ValveNum = 0; //集中器中阀控器总数。
			 
			for(i=0; i<para.MeterStoreNums; i++)
			{
				/*begin:yangfei modified 2012-12-27*/

				//	OSMutexPend (FlashMutex,0,&Err);
				Err = SDReadData("/METER_FILE_ADDR", &mf, sizeof(MeterFileType), sizeof(MeterFileType)*i);
    			//	OSMutexPost (FlashMutex);
				/*end  :yangfei modified 2012-12-27  */
    			gPARA_MeterFiles[i] = mf;

				if(mf.EquipmentType == HEAT_METER_TYPE)  //分别统计热表和阀控总数。
					gPARA_HeatMeterNum++;
				else
					gPARA_ValveNum++;
    			
				mfChannel = mf.ChannelIndex-1;				//集中器存储通道从0开始，而主站设置通道从1开始
				if(mfChannel > (METER_CHANNEL_NUM - 1))  //防止因表参数信息错误，导致下面赋值时抄过数组范围。
						mfChannel = METER_CHANNEL_NUM - 1;
				
				//热计量表按照通道分类
				/*begin:yangfei deleted 2014-02-27 for 时间通断面积法中热表地址为0，地址导致表个数减少*/
				//if(MeterNoBcdCheck(mf.MeterAddr))
				//	{
						ChannelTemp = gPARA_MeterChannelNum[mfChannel]++;
						if(ChannelTemp >= METER_PER_CHANNEL_NUM)  //防止数组超限。
							ChannelTemp = METER_PER_CHANNEL_NUM - 1;
						
						gPARA_MeterChannel[mfChannel][ChannelTemp] = i;
				//	}
					
    			//分配温控面板通道
				if(MeterNoBcdCheck(mf.ControlPanelAddr))
					{
						ChannelTemp = gPARA_ConPanelChannelNum[mfChannel]++;
						if(ChannelTemp >= METER_PER_CHANNEL_NUM)  //防止数组超限。
							ChannelTemp = METER_PER_CHANNEL_NUM - 1;
						
						gPARA_ConPanelChannel[mfChannel][ChannelTemp] = i;
					}
		
				//分配温控面板通道
				if(MeterNoBcdCheck(mf.ValveAddr))
					{
						ChannelTemp = gPARA_ValveConChannelNum[mfChannel]++;
						if(ChannelTemp >= METER_PER_CHANNEL_NUM)  //防止数组超限。
							ChannelTemp = METER_PER_CHANNEL_NUM - 1;
						
						gPARA_ValveConChannel[mfChannel][ChannelTemp] = i;
					}
			}
			
			gPARA_MeterNum = para.MeterStoreNums;
			
#ifdef DEMO_APP
        
		 if(para.DEMOWriteFlag== 0xAA55)
         DemoDateIsGet =1;  

#endif
		}
	else{				//未写入过有效参数数据
			gPARA_MeterNum 			= 0;
			gPARA_SendIndex_Prevous	= 0;
			memset((uint8 *)gPARA_MeterFiles, 0x00, sizeof(gPARA_MeterFiles));
			memset((uint8 *)gPARA_MeterChannel, 0x00, sizeof(gPARA_MeterChannel));
			memset((uint8 *)gPARA_MeterChannelNum, 0x00, sizeof(gPARA_MeterChannelNum));
			memset((uint8 *)gPARA_ConPanelChannel, 0x00, sizeof(gPARA_ConPanelChannel));
			memset((uint8 *)gPARA_ConPanelChannelNum, 0x00, sizeof(gPARA_ConPanelChannelNum));
			memset((uint8 *)gPARA_ValveConChannel, 0x00, sizeof(gPARA_ValveConChannel));
			memset((uint8 *)gPARA_ValveConChannelNum, 0x00, sizeof(gPARA_ValveConChannelNum));
			gPARA_HeatMeterNum = 0; //集中器中热表总数。
			gPARA_ValveNum = 0; //集中器中阀控器总数。
		}


	
	//初始化抄表时间节点信息
//	OSMutexPend (FlashMutex,0,&Err);
	Err = SDReadData("/TIME_NODE_ADDR", TimeNode, sizeof(TimeNode), 0);
 //   OSMutexPost (FlashMutex);
	if(Err!=NO_ERR)	 {
	    debug("%s %d read TIME_NODE_ADDR err=%d!\r\n",__FUNCTION__,__LINE__,Err);
	  }
	
	if(Err==NO_ERR)//防止卡错误时写数据死机
	{
	/*begin:yangfei added 2013-5-15*/
//	MakeDirInit(1);
	/*end   :yangfei added 2013-5-15*/
	if(TimeNode[0] == 0xAA55){
		//	OS_ENTER_CRITICAL();
			memcpy((uint8 *)gPARA_TimeNodes, (uint8 *)&TimeNode[1], (TIME_NODE_NUM_MAX)*sizeof(uint16));
			gPARA_TimeNodesNum = TimeNode[TIME_NODE_NUM_MAX+1];
			if(gPARA_TimeNodesNum > 24)
				gPARA_TimeNodesNum = 24;
			
		//	OS_EXIT_CRITICAL();
			
	}
	else{
		//	OS_ENTER_CRITICAL();
			memset((uint8 *)gPARA_TimeNodes, 0xFF, (TIME_NODE_NUM_MAX)*sizeof(uint16));
		//	OS_EXIT_CRITICAL();
			gPARA_TimeNodes[0] = 0x0100;
			gPARA_TimeNodes[1] = 0x0700;
			gPARA_TimeNodes[2] = 0x1100;
			gPARA_TimeNodes[3] = 0x2000;
			gPARA_TimeNodesNum = 4;
	}
	
	/*begin:yangfei modified 2012-12-24 for gPARA_TimeNodes save to timenode */
	//OS_ENTER_CRITICAL();
	memcpy(SystemTime,gSystemTime,sizeof(gSystemTime));
	//OS_EXIT_CRITICAL();
	GetFilePath(NodePath, SystemTime, ARRAY_DAY);
	
	//OSMutexPend (FlashMutex, 0, &Err);
	
	Err = SDSaveData(NodePath, &gPARA_MeterNum, sizeof(gPARA_MeterNum),6); 
	Err = SDSaveData(NodePath, gPARA_TimeNodes, sizeof(gPARA_TimeNodes),8); 
	Err = SDSaveData(NodePath, &gPARA_TimeNodesNum, sizeof(gPARA_TimeNodesNum),8+2*TIME_NODE_NUM_MAX); 

	
 	//OSMutexPost (FlashMutex);
	
	//初始化上行通讯参数

	//OSMutexPend (FlashMutex,0,&Err);
	Err = SDReadData("/UP_COMM_PARA", &UpPara, sizeof(UpCommParaSaveType), 0);
   // OSMutexPost (FlashMutex);
	if(Err!=NO_ERR)	{
	    debug("%s %d read UP_COMM_PARAerr=%d!\r\n",__FUNCTION__,__LINE__,Err);
	}

	if(UpPara.WriteFlag == 0xAA55){
			//OS_ENTER_CRITICAL();
			gPARA_UpPara = UpPara;
			//OS_EXIT_CRITICAL();
	}	
	else{
			//OS_ENTER_CRITICAL();
			gPARA_UpPara.HostIPAddr[3] = 0x7A;
			gPARA_UpPara.HostIPAddr[2] = 0x05;
			gPARA_UpPara.HostIPAddr[1] = 0x12;
			gPARA_UpPara.HostIPAddr[0] = 0x01;
			
			gPARA_UpPara.HostPort	   = 0x475;
			gPARA_UpPara.HeartCycle	   = 0x01;
			gPARA_UpPara.RelandDelay   = 0x02;
			gPARA_UpPara.RelandTimes   = 0x05;
			//OS_EXIT_CRITICAL();
	}

	
	Err = SDReadData((const char*)"/LCD_CONTRAST_PARA", (uint8 *)&NVLCDConraston, sizeof(LCDContrastPara),0); 
	if(Err!=NO_ERR)	{
	    debug("%s %d read LCD_CONTRAST_PARA=%d!\r\n",__FUNCTION__,__LINE__,Err);
	}

	if(NVLCDConraston.WriteFlag == 0xAA55){
		 if(NVLCDConraston.LCDContrast<121)
		 	NVLCDConraston.LCDContrast =LCD_CON_VALUE;
	}	
	else{
			//OS_ENTER_CRITICAL();
			NVLCDConraston.LCDContrast = LCD_CON_VALUE;
			//OS_EXIT_CRITICAL();
	}
	SetContrast(NVLCDConraston.LCDContrast);//设置对比值
	
	/*end   :yangfei modified 2012-12-24*/
	MakeDirInit(1);
  }
	else{
     		{
     				//	OS_ENTER_CRITICAL();
     					memset((uint8 *)gPARA_TimeNodes, 0xFF, (TIME_NODE_NUM_MAX)*sizeof(uint16));
     					
     				//	OS_EXIT_CRITICAL();
     					gPARA_TimeNodes[0] = 0x0100;
     					gPARA_TimeNodes[1] = 0x0700;
     					gPARA_TimeNodes[2] = 0x1100;
     					gPARA_TimeNodes[3] = 0x2000;
						gPARA_TimeNodesNum = 4;
     					
     		}

	        
			 //初始化上行通讯参数
			
			 //OSMutexPend (FlashMutex,0,&Err);
			 Err = SDReadData("/UP_COMM_PARA", &UpPara, sizeof(UpCommParaSaveType), 0);
			// OSMutexPost (FlashMutex);
			 if(Err!=NO_ERR) {
				 debug("%s %d read UP_COMM_PARAerr=%d!\r\n",__FUNCTION__,__LINE__,Err);
			 }
			
			 if(UpPara.WriteFlag == 0xAA55){
					 //OS_ENTER_CRITICAL();
					 gPARA_UpPara = UpPara;
					 //OS_EXIT_CRITICAL();
			 }	 
			 else{
					 //OS_ENTER_CRITICAL();
					 gPARA_UpPara.HostIPAddr[3] = 0x7A;
					 gPARA_UpPara.HostIPAddr[2] = 0x05;
					 gPARA_UpPara.HostIPAddr[1] = 0x12;
					 gPARA_UpPara.HostIPAddr[0] = 0x01;
					 
					 gPARA_UpPara.HostPort		= 0x475;
					 gPARA_UpPara.HeartCycle	= 0x01;
					 gPARA_UpPara.RelandDelay	= 0x02;
					 gPARA_UpPara.RelandTimes	= 0x05;
					 //OS_EXIT_CRITICAL();
			 }
	        
		Err = SDReadData((const char*)"/LCD_CONTRAST_PARA", (uint8 *)&NVLCDConraston, sizeof(LCDContrastPara),0); 
		if(Err!=NO_ERR)	{
	    	debug("%s %d read LCD_CONTRAST_PARA=%d!\r\n",__FUNCTION__,__LINE__,Err);
		}

		if(NVLCDConraston.WriteFlag == 0xAA55){
			if(NVLCDConraston.LCDContrast<121)
		  		 NVLCDConraston.LCDContrast =LCD_CON_VALUE;
		}	
		else{
			//OS_ENTER_CRITICAL();
			NVLCDConraston.LCDContrast = LCD_CON_VALUE;
			//OS_EXIT_CRITICAL();
    	}
	
		SetContrast(NVLCDConraston.LCDContrast);//设置对比值

	
		memcpy(SystemTime,gSystemTime,sizeof(gSystemTime));
		GetFilePath(NodePath, SystemTime, ARRAY_DAY);
				   
		Err=MakeFile(NodePath);
		 if(Err){
						  
		 }
	 	else {
					  
		    do{
				Err = SDReadData(NodePath, (uint8 *)&TimeNode[1], (TIME_NODE_NUM_MAX)*sizeof(uint16), 8);
								  
				if(Err!=NO_ERR) {
					 counter++; 
				}
				if (counter ==5 ){
                    counter =0;
                    break;
                 }
			}while(Err!=NO_ERR);
		}

		Err = SDSaveData(NodePath, &gPARA_MeterNum, sizeof(gPARA_MeterNum),6); 
		Err = SDSaveData(NodePath, gPARA_TimeNodes, sizeof(gPARA_TimeNodes),8); 
		Err = SDSaveData(NodePath, &gPARA_TimeNodesNum, sizeof(gPARA_TimeNodesNum),8+2*TIME_NODE_NUM_MAX); 
				   
		MakeDirInit(1);//再建立一次


	}


	
}


/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
