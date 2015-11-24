/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: SysLog.c
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年9月10日 
**  描        述: 日志记录系统运行信息
**	修 改  记 录:   	
*****************************************************************************************************/

/********************************************** include *********************************************/
#include <includes.h>
#include <stdarg.h>

/********************************************** define *********************************************/
#define 	LOG_SD_SAVE_SIZE			512
static uint8 LOG_Flag_Level = LOG_LEVEL_INFO;//muxiaoqing add

/********************************************** global *********************************************/
uint8 gLOG_LogLevelFlag	= LOG_LEVEL_WARN;
extern uint8  EXTI_ERRO_Happen ;

/********************************************** static *********************************************/

/****************************************************************************************************
**	函 数  名 称: LOG_QueueCreate
**	函 数  功 能: 创建日志记录所需的数据队列，日志用信号量
**	输 入  参 数: 无
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void LOG_QueueCreate(void)
{
	//创建发送队列
    if (QueueCreate((void *)gLOG_SystemLog,
                     sizeof(gLOG_SystemLog),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
    
    //创建产生新日志的信号量
	LogQueSem = OSSemCreate(0);
    if (LogQueSem == NULL){
        while(1);
    }
	
	QueueFlush((void*)gLOG_SystemLog); 								//清空接收队列
    
}

/****************************************************************************************************
**	函 数  名 称: LOG_ParaInit
**	函 数  功 能: 日志记录文件，全局变量初始化
**	输 入  参 数: 无
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void LOG_ParaInit(void)
{
	//uint8 Err 		= 0x00;
	//uint32 AddrTemp = 0x00;
		
	gLOG_LogLevelFlag	= LOG_ALL;  
}

/****************************************************************************************************
**	任 务  名 称: TaskLog
**	任 务  功 能: 日志记录任务，所有任务的日志信息均由该任务处理	
**	输 入  参 数: 无
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 可选择从串口数据日志信息，也可以讲日志信息存储至SD卡中
*****************************************************************************************************/
void TaskLog(void *pdata)
{
	uint8 i,j;
	uint8 err;
	uint16 BufLen = 0x00;
	uint8 LogBuf[1024] = {0x00};
	uint8 *LogTemp = LogBuf;
	uint8 LogData = 0x00;
	uint8 CirTimes, Residual;
	//uint32 SD_SaveAddr = 0x8000000;
	CPU_SR		cpu_sr;
	/*begin:yangfei added 2012-12-24*/
	char    LogPath[]="/2012/12/24/log.txt";
	uint8   HexSystemTime[6] 			= {0x00};
	/*end   :yangfei added 2012-12-24*/
	#if 0      /*yangfei deleted 2012-12-19*/
	LOG_ParaInit();
	#endif
	QueueFlush((void*)gLOG_SystemLog); 								//清空接收队列
	
	while(1)
	{
		do{
			FeedTaskDog();
			OSSemPend(LogQueSem, OS_TICKS_PER_SEC*6, &err);	
		}while(err != OS_ERR_NONE);

		/*begin:yangfei added 2012-12-24*/
		OS_ENTER_CRITICAL();
		memcpy(HexSystemTime,gSystemTime,6);
		OS_EXIT_CRITICAL();
		GetFilePath(LogPath, HexSystemTime,  ARRAY_DAY)	;
		/*end   :yangfei added 2012-12-24*/
		BufLen = QueueNData(gLOG_SystemLog);
		LogTemp = LogBuf;

		if(BufLen >= LOG_SD_SAVE_SIZE)
			{
				CirTimes = BufLen/4;
				Residual = BufLen%4;
				
				for(i=0; i<CirTimes; i++)
				{
					OS_ENTER_CRITICAL();
					for(j=0; j<4; j++)
					{
						err = QueueRead_Int(&LogData, (void*)gLOG_SystemLog);
						if(err == QUEUE_EMPTY)	break;
						if(err!=QUEUE_OK)		while(1);			//不应该发生这种错误
						*LogTemp++ = LogData;
					}
					OS_EXIT_CRITICAL();
				}
				
				OS_ENTER_CRITICAL();
				for(j=0; j<Residual; j++)
				{
					err = QueueRead_Int(&LogData, (void*)gLOG_SystemLog);
					if(err == QUEUE_EMPTY)	break;
					if(err!=QUEUE_OK)		while(1);				//不应该发生这种错误
					*LogTemp++ = LogData;
				}
				OS_EXIT_CRITICAL();	
				

				OSMutexPend (FlashMutex,0,&err);

				err = SDSaveLog(LogPath, LogBuf, BufLen,0);

		    	OSMutexPost (FlashMutex);

			}
	}
}
/*add by yangmin 2013-09-17*/
void LOG_HandleLogBuf()
{
      uint8 i,j;
	uint8 err;
	CPU_SR		cpu_sr;
	uint8 LogData = 0x00;
	uint16 BufLen = 0x00;
	uint8 CirTimes, Residual;
      uint8 LogBuf[1024] = {0x00};
	uint8 *LogTemp = LogBuf;
	/*begin:yangfei added 2012-12-24*/
	char    LogPath[]="/2012/12/24/log.txt";
	uint8   HexSystemTime[6] 			= {0x00};
	/*end   :yangfei added 2012-12-24*/
   
/*begin:yangfei added 2012-12-24*/
	OS_ENTER_CRITICAL();
	memcpy(HexSystemTime,gSystemTime,6);
	OS_EXIT_CRITICAL();
	
	BufLen = QueueNData(gLOG_SystemLog);
	GetFilePath(LogPath, HexSystemTime,  ARRAY_DAY);

	CirTimes = BufLen/4;
	Residual = BufLen%4;
				
	for(i=0; i<CirTimes; i++)
	{
		OS_ENTER_CRITICAL();
		for(j=0; j<4; j++)
		{
			err = QueueRead_Int(&LogData, (void*)gLOG_SystemLog);
			if(err == QUEUE_EMPTY)	break;
			if(err!=QUEUE_OK)		while(1);			//不应该发生这种错误
			*LogTemp++ = LogData;
		}
		OS_EXIT_CRITICAL();
	}
				
	OS_ENTER_CRITICAL();
	for(j=0; j<Residual; j++)
	{
		err = QueueRead_Int(&LogData, (void*)gLOG_SystemLog);
		if(err == QUEUE_EMPTY)	break;
		if(err!=QUEUE_OK)		while(1);				//不应该发生这种错误
		*LogTemp++ = LogData;
	}
	OS_EXIT_CRITICAL();	
				
	OSMutexPend (FlashMutex,0,&err);
	err = SDSaveLog(LogPath, LogBuf, BufLen,0);
     OSMutexPost (FlashMutex);

}
void LOG_HandleEXTIBuf(const uint8 *pInLog,uint8 Index)

{
      uint8 i,j;
	uint8 err;
	CPU_SR		cpu_sr;
	uint8 LogData = 0x00;
	uint16 BufLen = 0x00;
	uint8 CirTimes, Residual;
 //     uint8 LogBuf[1024] = {0x00};
//	uint8 *LogTemp = LogBuf;
	char    LogPath[]="/EXTI_PWR_ERR_REC"; 
    char	 MBUSLogPath[]="/EXTI_MBUS_ERR_REC";
	uint8   HexSystemTime[6] 			= {0x00};
	uint8 Res;
    ERRLogFileType ERRLOGFile; 	
    ERRLogFilePara ERRLOGPara; 
    ERRLogMBUSFileType ERRLOGMBUSFile; 	
    ERRLogMBUSFilePara ERRLOGMBUSPara; 

#if 1	
		
	OSMutexPend (FlashMutex,0,&err);
	if(Index==2)//MBUS短路
		{
		Res = SDReadData((const char*)"/EXTI_MBUS_ERR_PARA", (uint8 *)&ERRLOGMBUSPara, sizeof(ERRLogMBUSFilePara),0); 

	    }
	else
	
    Res = SDReadData((const char*)"/EXTI_PWR_ERR_PARA", (uint8 *)&ERRLOGPara, sizeof(ERRLogFilePara),0); 

	OSMutexPost (FlashMutex);
	if(Res!=NO_ERR) 
	  {
		debug("%s %d read LOG_HandleEXTIBuf err=%d!\r\n",__FUNCTION__,__LINE__,Res);
//		return;
	  }

	if(Index==2)//MBUS短路
	{
		if(ERRLOGMBUSPara.WriteFlag == 0xAA55)
		{	   
		  if(ERRLOGMBUSPara.LOGStoreNums<11)//存储10条log信息
			
		  ERRLOGMBUSPara.LOGStoreNums++;
			else
		  ERRLOGMBUSPara.LOGStoreNums=1;		
		}
		else
			{
			  ERRLOGMBUSPara.WriteFlag = 0xAA55;
			  ERRLOGMBUSPara.LOGStoreNums =1;
			}

	}
	else
		{
        	if(ERRLOGPara.WriteFlag == 0xAA55)
        	{      
        	  if(ERRLOGPara.LOGStoreNums<11)//存储10条log信息
        	  	
        	  ERRLOGPara.LOGStoreNums++;
        	  	else
        	  ERRLOGPara.LOGStoreNums=1;		
        	}
            else
            	{
                  ERRLOGPara.WriteFlag = 0xAA55;
        		  ERRLOGPara.LOGStoreNums =1;
            	}
		}
	
	OSMutexPend (FlashMutex,0,&err);
	
	if(Index==2)//MBUS短路
	{
	
	err = SDSaveData((const char*)"/EXTI_MBUS_ERR_PARA", (uint8 *)&ERRLOGMBUSPara, sizeof(ERRLogMBUSFilePara),0);
	}
	else
	err = SDSaveData((const char*)"/EXTI_PWR_ERR_PARA", (uint8 *)&ERRLOGPara, sizeof(ERRLogFilePara),0);
     OSMutexPost (FlashMutex);
	#if 0
	BufLen = QueueNData(gLOG_SystemLog);

	CirTimes = BufLen/4;
	Residual = BufLen%4;
				
	for(i=0; i<CirTimes; i++)
	{
		OS_ENTER_CRITICAL();
		for(j=0; j<4; j++)
		{
			err = QueueRead_Int(&LogData, (void*)gLOG_SystemLog);
			if(err == QUEUE_EMPTY)	break;
			if(err!=QUEUE_OK)		while(1);			//不应该发生这种错误
			*LogTemp++ = LogData;
		}
		OS_EXIT_CRITICAL();
	}
				
	OS_ENTER_CRITICAL();
	for(j=0; j<Residual; j++)
	{
		err = QueueRead_Int(&LogData, (void*)gLOG_SystemLog);
		if(err == QUEUE_EMPTY)	break;
		if(err!=QUEUE_OK)		while(1);				//不应该发生这种错误
		*LogTemp++ = LogData;
	}
	OS_EXIT_CRITICAL();	
	#endif
	
	if(Index==2)//MBUS短路
	    {
	    #if 0
        	memcpy(ERRLOGMBUSFile.LOGsting,pInLog,96);	
        	ERRLOGMBUSFile.LOGLevel = LOG_LEVEL_ERROR;
        	OSMutexPend (FlashMutex,0,&err);
        	err = SDSaveData(MBUSLogPath, (uint8 *)&ERRLOGMBUSFile, sizeof(ERRLogMBUSFileType),(ERRLOGMBUSPara.LOGStoreNums-1)*128);
             OSMutexPost (FlashMutex);
		#endif
	    }
	else
		{
        	memcpy(ERRLOGFile.LOGsting,pInLog,96);	
        	ERRLOGFile.LOGLevel = LOG_LEVEL_ERROR;
        	OSMutexPend (FlashMutex,0,&err);
        	err = SDSaveData(LogPath, (uint8 *)&ERRLOGFile, sizeof(ERRLogFileType),(ERRLOGPara.LOGStoreNums-1)*128);
            OSMutexPost (FlashMutex);
		}
	 if(err!=NO_ERR) 
	   {
		 debug("%s %d read LOG_HandleEXTIBuf err=%d!\r\n",__FUNCTION__,__LINE__,err);
//		 return;
	   }
	 #endif
	if(Index ==1)
		{
            //重启
        	//NVIC_SETFAULTMASK();
        	//NVIC_GenerateSystemReset();
		}
	else if(Index ==2)//MBUS短路
		{
       //   DISABLE_MBUS();
	    }
}

/****************************************************************************************************
**	任 务  名 称: LOG_WriteSysLog
**	任 务  功 能: 将需记录的日志数据输入到日志数据队列中	
**	输 入  参 数: uint8 *pInLog	-- 日志数据指针; uint16 LogLen -- 日志数据长度
**	输 出  参 数: 无
**  返   回   值: NO_ERR
**	备		  注: 本函数采用关中断方式，本函数为可重入，所以多任务可以任意调用.
**				  日志信息为数据或不需要采用格式参数时使用该函数
*****************************************************************************************************/
uint8 LOG_WriteSysLog(const uint8 *pInLog, uint16 LogLen)
{
	uint32 N=4; //为了减少系统关中断时间,并减少开关中断次数，这里采用写N个字节关开一次中断
				//为了降低系统的中断延迟时间，原则上N 不应大于6。
	uint32 i,j,p,x,y;
	OS_CPU_SR cpu_sr;

   	LOG_assert_param( pInLog == NULL );

	if(pInLog==NULL)	while(1);			//出现错误
	
	//======================等待日志记录队列可用空间>=n
	if(QueueNotEnoughPend((void*)gLOG_SystemLog, LogLen)){
		return 1;
	}

	//======================
	x=LogLen/N;	y=LogLen%N;	p=0;
	for(i=0;i<x;i++){
		OS_ENTER_CRITICAL();
		for(j=0;j<N;j++){
			QueueWrite_Int((void *)gLOG_SystemLog, pInLog[p++]); 					//数据入队列
		}
		OS_EXIT_CRITICAL();
	}

	OS_ENTER_CRITICAL();
	for(j=0;j<y;j++){
		QueueWrite_Int((void *)gLOG_SystemLog, pInLog[p++]); 					//数据入队列
	}
	OS_EXIT_CRITICAL();
	
	OSSemPost(LogQueSem);

	return NO_ERR;
}

/****************************************************************************************************
**	任 务  名 称: LOG_WriteSysLog_Format
**	任 务  功 能: 格式化输入日志记录	
**	输 入  参 数: uint8 Flag -- 日志等级标志;	const char *format -- 格式化输入参数
**	输 出  参 数: 无
**  返   回   值: NO_ERR
**	备		  注: 格式化参数输入，最大数据长度为256字节, 若大于256，则后面的会被截断
*****************************************************************************************************/
uint8 LOG_WriteSysLog_Format(uint8 Flag, const char *format, ...)
{
//	uint8 err;
	char StringBuf[256] 	   	= {0x00};
//	uint8 SecondTemp		   	= 0x00;
	uint8 LevelTemp				= 0x00;
	uint8 lTimeDate[6]			= {0x00};
	int16 StringLen				= 0x00;
	//uint32 TimeCounter		   	= 0x00;
	va_list argptr;
	CPU_SR		cpu_sr;
    uint8  EXTIhappenIndex =0;
	LOG_assert_param( format == NULL );
		

	if(format==NULL)		while(1);
	
	//防止2条指令时间间隔较小，频繁读取时间
	OS_ENTER_CRITICAL();
	LevelTemp = gLOG_LogLevelFlag;
	OS_EXIT_CRITICAL();
	/*begin:yangfei modifed 2013-04-04*/
    #if  0
	//根据日志等级进行过滤记录
	if( !(LevelTemp&Flag) )		{return NO_ERR;}
    #else
    if( LevelTemp<Flag )		{return NO_ERR;}
    #endif
    /*end:yangfei modifed 2013-04-04*/
	ReadDateTime(lTimeDate);
	
	// \r	
	StringBuf[0] = '\r';
	// \n
	StringBuf[1] = '\n';
	//年	
	StringBuf[2] = UINT8_TO_ASCII_H((lTimeDate[ARRAY_YEAR]>>4)&0x0F);
	StringBuf[3] = UINT8_TO_ASCII_L(lTimeDate[ARRAY_YEAR]&0x0F);
	// - 
	StringBuf[4] = '-';
	//月
	StringBuf[5] = UINT8_TO_ASCII_H((lTimeDate[ARRAY_MONTH]>>4)&0x0F);
	StringBuf[6] = UINT8_TO_ASCII_L(lTimeDate[ARRAY_MONTH]&0x0F);
	// - 
	StringBuf[7] = '-';
	//日
	StringBuf[8] = UINT8_TO_ASCII_H((lTimeDate[ARRAY_DAY]>>4)&0x0F);
	StringBuf[9] = UINT8_TO_ASCII_L(lTimeDate[ARRAY_DAY]&0x0F);
	// 空格 
	StringBuf[10] = ' ';
	//时	
	StringBuf[11] = UINT8_TO_ASCII_H((lTimeDate[ARRAY_HOUR]>>4)&0x0F);
	StringBuf[12] = UINT8_TO_ASCII_L(lTimeDate[ARRAY_HOUR]&0x0F);
	// : 
	StringBuf[13] = ':';
	//分
	StringBuf[14] = UINT8_TO_ASCII_H((lTimeDate[ARRAY_MINUTE]>>4)&0x0F);
	StringBuf[15] = UINT8_TO_ASCII_L(lTimeDate[ARRAY_MINUTE]&0x0F);
	// : 
	StringBuf[16] = ':';
	//秒
	StringBuf[17] = UINT8_TO_ASCII_H((lTimeDate[ARRAY_SECOND]>>4)&0x0F);
	StringBuf[18] = UINT8_TO_ASCII_L(lTimeDate[ARRAY_SECOND]&0x0F);
	// -->空格
	StringBuf[19] = '-';
	StringBuf[20] = '-';
	StringBuf[21] = '>';
	StringBuf[22] = ' ';
	
	va_start(argptr, format);
	StringLen = vsnprintf(&StringBuf[23], sizeof(StringBuf), format, argptr);
	if(StringLen)			;						//此处需对复制的长度进行判断
	va_end(argptr);
	LOG_Flag_Level = Flag;
	if(EXTI_ERRO_Happen)
		{
		EXTIhappenIndex = EXTI_ERRO_Happen;
		EXTI_ERRO_Happen =0;
		LOG_HandleEXTIBuf((uint8 *)StringBuf,EXTIhappenIndex);

	    }
	else
		{
         /*begin:yangfei added 2013-02-20*/
         if(LevelTemp>LOG_LEVEL_INFO)
           {
           debug("%s\r\n",StringBuf);
           }
         else
           {
           LOG_WriteSysLog((uint8 *)StringBuf, strlen(StringBuf));
           }
		}
   /*end:yangfei added 2013-02-20*/
	
	return NO_ERR;
}

/****************************************************************************************************
**	任 务  名 称: Log_assert_failed
**	任 务  功 能: 参数断言判断处理函数	
**	输 入  参 数: const char *format -- 格式化输入参数
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void LOG_assert_failed(uint8* file, uint32 line)
{
	LOG_WriteSysLog_Format(LOG_LEVEL_DEBUG, "ERROR: <<%s.c>> Line %5d", file, line);
}

/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
