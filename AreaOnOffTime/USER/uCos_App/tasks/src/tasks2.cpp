/*******************************************Copyright (c)*******************************************
**									山东华宇空间技术公司(西安分部)                                **                            
**  文   件   名: tasks2.cpp																      **
**  创   建   人: 勾江涛																		  **
**	版   本   号: 0.1																		      **
**  创 建  日 期: 2012年9月20日 													     	      **
**  描        述: 根据设定的时间，开始读取流量表的数据			      						      **
**	修 改  记 录:   																			  **
****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>
#include "app_flashmem.h"
#include "app_down.h"
#include "tasks_up.h"
#include "tasks2.h"
#include "app_up.h"

/****************************************************************************************************/
/********************************************** global **********************************************/
OS_EVENT * ReadAllCurMbox;

OS_EVENT* MbusChannleOnMbox;
OS_EVENT* MbusChannleOFFMbox;

uint8 gu8CreateNewDay = 1; //新一天开始标记，1-新一天。当刚开机或新一天开始时标记为1.

/****************************************************************************************************/
/********************************************** static **********************************************/
//各任务堆栈定义
static OS_STK	TaskClockStk[256];
static OS_STK	TaskReadAllCurStk[TASK_STK_SIZE];
static OS_STK	TaskLogStk[TASK_STK_SIZE];
static OS_STK	TaskMbusShortCStk[TASK_STK_SIZE];
static OS_STK	TaskMbusShortCTimerStk[TASK_STK_SIZE];

static uint8 counter1=0;
static 	 uint8 PreTime[8] = {0x00};
extern uint8  EXTI_ERRO_Happen ;//muxiaoqing
vs32  Timer_counter =0;
vs32  Channel1_Timer_counter =0;
vs32  Channel2_Timer_counter =0;
vs32  Channel3_Timer_counter =0;
vs32  Channel4_Timer_counter =0;
#define	FIRST_COUNTER_TIME 30
#define	SECOND_COUNTER_TIME 35
#define	THIRD_COUNTER_TIME 50



vs32 Channel1_Start_Counter =0;

vs32 Channel2_Start_Counter =0;
vs32 Channel3_Start_Counter =0;
vs32 Channel4_Start_Counter =0;


uint8  BinaryDA1[32]={0};

void DecimalToBinary1(int num,uint8 *BinaryArry)
	{   
	uint8 *Binary;
	Binary =BinaryArry;
	if(num ==0)    
		{
		return;   
	    }  
	else   
		{    
		if(num/2)
			{			    
        		counter1++;
        		DecimalToBinary1(num/2,Binary);           
        		Binary[31-(--counter1)]= num%2;
			}
		else
			{
                Binary[31-counter1]=num%2;
			
		    }
		}
	}

/****************************************************************************************************/
/********************************************************************************************************
**  函 数  名 称: TaskClock     									                                   **
**	函 数  功 能: 设置抄表时间                                                                         **			
**	输 入  参 数: pdata													                               **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void  TaskClock( void *pdata )
{
	uint8 Err = 0;
	uint8 TNum;
	uint8 ReadDate[6];
	uint8 CmdSenddFlag		= 0;					//抄表节点发送抄表信号的标志
	uint8 TimeNodeFlag		= 0;					//抄表时间点到，发送抄表命令
	uint8 NodeIndex			= 0;
	int8  TimeOffset		= 0;
	uint8 SendMsg[2]		= {0x00};
	uint16 CurrentTime		= 0;
//	uint32 LogicAddr		= 0;	
	/*begin:yangfei added 2012-12-26*/
	uint8  OldMonth          =5;
	/*end   :yangfei added 2012-12-26*/

	uint8 OldDay = 0;
	char lcYesterdayPath[] = "/2015/06/09/1200";
	uint8 SystemTime[6] = {0x00};


//	uint16 ReadingMeterSn	= 0x00;
	/*begin:yangfei added 2012-12-24 find path*/
//	uint8 SystemTime[6] = {0x00};
//	char  DataPath[] = "/2012/12/24/1530";
//	uint8 Res = 0;
//	CPU_SR			cpu_sr;	

	pdata = pdata;

	TNum=0;
	
	memset(PreTime, 0x00, 8);
 	ReadDateTime(ReadDate);
	memcpy(PreTime, ReadDate, 6);

//"昨天"初始化
	memcpy(SystemTime, gSystemTime, 6);
	if(SystemTime[ARRAY_DAY] == 1)   //昨天，不严谨。
		SystemTime[ARRAY_DAY] = 30;
	else
		SystemTime[ARRAY_DAY] -= 1;
	
	GetFilePath(lcYesterdayPath, SystemTime,ARRAY_DAY);
	memcpy(gcYesterdayPath,lcYesterdayPath,sizeof(lcYesterdayPath));

	
	/*begin:yangfei added 2012-12-26*/
	//MakeDirInit(1);
	/*end   :yangfei added 2012-12-26*/
//	LogicAddr = 0x4C800000;	
	while(1)
	{
		FeedTaskDog(); 
	 	OSTimeDly(15*OS_TICKS_PER_SEC);

		TNum=(TNum+1)%4; //60 second
	 	if(TNum == 0)											//每分钟检查一次时钟
	 	{
	 			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskClock] One Minute Check OK!"); 
	 			//检查是否到了抄表时间点
	 			ReadDateTime(ReadDate);
				if( (ReadDate[ARRAY_HOUR]==0x00)&&(ReadDate[ARRAY_MINUTE]==0x00) )
				{
					gu8CreateNewDay = 1; //新一天开始，标记创建新一天的基础信息。
					TIME_UpdateSystemTime();  					//更新系统时钟
				}
				
				if(gu8CreateNewDay == 1){
					STORE_InitTimeNodePara(ReadDate, 0); //在这里主要作用是每天0点0分准时创建新一天的基础信息。
					gu8CreateNewDay = 0;
				}
								
				CurrentTime = (ReadDate[ARRAY_HOUR]<<8) | ReadDate[ARRAY_MINUTE];
	 			
	 			Err = STORE_GetTimeNodeInfo(CurrentTime, &NodeIndex, &TimeOffset);
				if(Err == NO_ERR)
				{
					if( (0<TimeOffset) && (TimeOffset<20) )		//5分钟内 一定要将定时抄表命令发出去
	 					{
	 						TimeNodeFlag = 1;
	 						//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskClock] Ready To Read Meter!"); 
	 					}
	 				else
	 					{
							TimeNodeFlag = 0;
	 						CmdSenddFlag = 0;
	 						//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskClock] Not Ready To Read Meter!");
	 					}
				}/*
				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskClock] Time Check Info: NodeIdex is %2XH, TimeOffset is %2XH,"
									   "The Function <STORE_GetTimeNodeInfo> Return Value is %2XH",
									   NodeIndex, TimeOffset, Err);*/
				/*
				uint8 test = 0xA1;
				USART1Send(&test, 1);
				USART1Send(ReadDate, 6);
				USART1Send(&NodeIndex, 1);
				USART1Send((uint8 *)&TimeOffset, 1);
				USART1Send(&Err, 1);
				*/
				/*  这里时间检测与时钟设置之间的冲突该如何解决
				//这里时间判定要严格
				testcmp = CmpTime(ReadDate, PreTime);
				if(testcmp > 0)		//时间正确
				{
					memcpy(PreTime, ReadDate, 6);
				}
				else				//时间不正确
				{
					PreTime[7]++;
					OSMutexPend (FlashMutex,0,&Err);
    				Err =AppSdWrRdBytes(LogicAddr, 8, PreTime, SD_WR);
    				OSMutexPost (FlashMutex);
					LogicAddr += 8;

					AddMin(PreTime, 1);		//修正错误时间	
					UserSetDateTime(PreTime);

				}
				*/
				if(OldMonth != gSystemTime[ARRAY_MONTH])
	 			{
	 					OldMonth = gSystemTime[ARRAY_MONTH];
				
						MakeDirInit(OldMonth);
	 			}

				if(OldDay != gSystemTime[ARRAY_DAY]){
					StoreYesterday();
					OldDay = gSystemTime[ARRAY_DAY];
				}


			
	 		}
	 	
	 	if( (CmdSenddFlag==0) && (TimeNodeFlag==1) )		//每个时间节点 只发送一个抄表请求
		{	
	 			STORE_InitTimeNodePara(ReadDate, NodeIndex);
	 			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskClock]Init The Time Node Para!");
	 			CmdSenddFlag = 1;							//防止5分钟内每分钟都发送一个抄表请求

				if((gu8ReadAllMeterFlag!=0) && (gu8ReadAllMeterFlag!=1)){  //防止出错。
					gu8ReadAllMeterFlag = 0;
				}
				
				if(gu8ReadAllMeterFlag == 0){  //只有在不抄表时才发送。
					SendMsg[0] =  TIMING_ALL;
	 				OSQPost(CMD_Q, (void*)SendMsg);
	 				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskClock] Post Msg Queue For Reading all of HeatMeters!");
				}
				else{
					LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: Ignore this time Reading all of HeatMeters!");
				}
				
			}

   	}
}

/********************************************************************************************************
**  函 数  名 称: TaskReadAllCur     									                               **
**	函 数  功 能: 读取所有流量表的数据                                                                 **			
**	输 入  参 数: pdata													                               **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void  TaskReadAllCur(void *pdata)
{
	uint8 err;
	uint8 *pMsg = NULL;
	
	
	pdata = pdata;
	while(1)
	{
		
	 	do{
	 		FeedTaskDog();
	 		pMsg = (uint8 *)OSQPend(CMD_Q, 10*OS_TICKS_PER_SEC, &err);
	 	}while( (err!=OS_ERR_NONE) || (pMsg == NULL) );
	 	
	 	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskReadAllCur] Recive a Message, The Msg ID is 0x%X", *pMsg);
	 	switch(*pMsg)
	 	{
			case MSG_1D:
			case TIMING_ALL:
	 			{
					gu8ReadAllMeterFlag = 1;//标记开始抄全表。
	 				READ_ReadAllMeters();
					gu8ReadAllMeterFlag = 0;//标记抄全表结束。
	 				break;
	 			}
	 		case MSG_18:
	 			{
	 				DEAL_MSG18_ForAll(++pMsg);
	 				break;
	 			}
	 		case MSG_1C:
	 			{
	 				DEAL_MSG1C_ForAll(++pMsg);
	 				break;
	 			}
			case ISSUE_HEATVALUE:  //通断时间面积法中，向用户发送上位机发来的热量分摊值。
			{
				IssueAllHeatValue();//下发所有用户的分摊热量值。
				break;
			}

				
	 		default:
	 			break;
	 	}
	 	
   		OSTimeDly(2*OS_TICKS_PER_SEC); 

   	}
}


/********************************************************************************************************
**  函 数  名 称: TaskMbusShortC     									                               **
**	函 数  功 能: Mbus 短路管理                 **			
**	输 入  参 数: pdata													                               **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void  TaskMbusShortC(void *pdata)
{
	uint8 i = 0;
	uint8 lu8MBusState = 0;

	while(1){
				//begin: 蜂鸣器.PB5,高电平响，低电平停止响。
		
		
				if((gu8ErrFlag==1) && (gu16ErrOverTime<4000)){
					GPIO_SetBits(GPIOB,GPIO_Pin_5);
					OSTimeDlyHMSM(0,0,0,100);
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
					OSTimeDlyHMSM(0,0,0,200);
					GPIO_SetBits(GPIOB,GPIO_Pin_5);
					OSTimeDlyHMSM(0,0,0,100);
					GPIO_ResetBits(GPIOB,GPIO_Pin_5);
					
					gu16ErrOverTime++;
				}
				else{
					OSTimeDlyHMSM(0,0,0,500);

				}
		
		
		
				//end: 蜂鸣器。

				FeedTaskDog();
		
				//begin:检测判断MBUS通道是否短路。最长等待3秒。
				if(1 == gu8MBusShortRising){
					for(i=0;i<60;i++){
						OSTimeDlyHMSM(0,0,0,50);
						lu8MBusState = STA_MBUS_OFF();
						if(lu8MBusState == 0){	//如果MBUS短路检测引脚为低电平，则说明没有真正短路。
							break;
						}
						else{
							gu8MBusShortTime++;
		
						}
		
					}
		
					if(i >= 59){
						DisableAllMBusChannel();  //进入MBUS中断，切断所有通道。
						gu8MBusShortFlag[gu8NowMBUSChannel-1] = 1;	//标记对应MBUS通道通道短路
		
					}
					
					gu8MBusShortRising = 0; //无论是否真短路，判断完后都要清除标记。
		
					
		
					
		
				}
				else{
					OSTimeDlyHMSM(0,0,0,500);

				}
		
				//end:检测判断MBUS通道是否短路。


	}

}
/********************************************************************************************************
**  函 数  名 称: TaskMbusShortC_Timer     									                               **
**	函 数  功 能: Mbus 短路时间管理                 **			
**	输 入  参 数: pdata													                               **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void  TaskMbusShortC_Timer(void *pdata)
{

}
/********************************************************************************************************
**  函 数  名 称: CreateTasks2      									                               **
**	函 数  功 能: 创建Tasks2                                                                           **			
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void CreateTasks2(void)
{
	uint8 err=0;
    
    err=OSTaskCreate(TaskClock, (void *)0,&TaskClockStk[256 - 1], PRIO_TASK_CLOCK);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
    
    err=OSTaskCreate(TaskReadAllCur, (void *)0,&TaskReadAllCurStk[(TASK_STK_SIZE) - 1],PRIO_TASK_READ_ALL_CUR);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
	
	err=OSTaskCreate(TaskLog, (void *)0,&TaskLogStk[(TASK_STK_SIZE) - 1],PRIO_TASK_LOG);
	if(err!=OS_ERR_NONE){
    	while(1);
    } 

	err=OSTaskCreate(TaskMbusShortC, (void *)0,&TaskMbusShortCStk[(TASK_STK_SIZE) - 1],PRIO_TASK_MBUSSHORTC);
	if(err!=OS_ERR_NONE){
    	while(1);
		}

//	err=OSTaskCreate(TaskMbusShortC_Timer, (void *)0,&TaskMbusShortCTimerStk[(TASK_STK_SIZE) - 1],37);
//		if(err!=OS_ERR_NONE){
//			while(1);		
//    } 

}

/********************************************************************************************************
**  函 数  名 称: CreateMboxs2       									                               **
**	函 数  功 能: 创建Mboxs2                                                                           **			
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void CreateMboxs2(void)
{
	UpAskMeterSem = OSSemCreate(1);
    if (UpAskMeterSem == NULL)
    {
        while(1);
    }

	
	METERChangeChannelSem = OSSemCreate(1);
    if (METERChangeChannelSem == NULL)
    {
        while(1);
    }
    
    CMD_Q = OSQCreate(&CmdMsgQ[0], 10);
	if (Str_Q == NULL)
    {
        while (1);
    }
  	MbusChannleOnMbox= OSMboxCreate((void*)0);
	if (MbusChannleOnMbox == NULL)
    {
        while (1);
    } 
	MbusChannleOFFMbox= OSMboxCreate((void*)0);
	if (MbusChannleOFFMbox == NULL)
    {
        while (1);
    }  
}
/********************************************************************************************************
**  函 数  名 称: CreateMutexs2       									                               **
**	函 数  功 能: 创建Mutexs2                                                                          **			
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void CreateMutexs2(void)
{
    ;
}
/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
