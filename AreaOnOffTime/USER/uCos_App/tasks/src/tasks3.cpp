/*******************************************Copyright (c)*******************************************
**									山东华宇空间技术公司(西安分部)                                **                            
**  文   件   名: tasks3.cpp																      **
**  创   建   人: 勾江涛																		  **
**	版   本   号: 0.1																		      **
**  创 建  日 期: 2012年12月07日 													     	      **
**  描        述: GPRS管理任务        			      						                      **
**	修 改  记 录:   																			  **
****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>
#include "app_flashmem.h"
#include "app_down.h"
#include "tasks3.h"
#include "tasks_up.h"


//#define TEST_SELF_CHECK 2
/********************************************** static **********************************************/
static OS_STK  TaskGprsIpdStk[TASK_STK_SIZE];
static OS_STK  TaskGprsManaStk[TASK_STK_SIZE];
//static char  gGprsRecBuf[MAX_IPSEND_MC52i_BYTENUM*2+100];
static char gRecBuf_xmz[MAX_REC_MC52i_BYTENUM+100];//这里+100是冗余

static  uint32 tstn1,tstn2,tstn3,tstn4,tstn5;

/************************************************ gobale ***********************************************/
OS_EVENT* GprsManaMbox;
OS_EVENT* UpLandAnswMbox;
OS_EVENT* HeartFrmMbox;

/************************************************ extern ***********************************************/
extern uint16 gUpdateBegin ;
/********************************************************************************************************
**  函 数  名 称: Fun_GprsIpd_xmz       									                           **
**	函 数  功 能: 西门子模块 								                                           **			
**	输 入  参 数: pdata												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void Fun_GprsIpd_xmz(void)
{
	//本任务没有任务狗,原因之一是它可能在一段时间处在挂起状态
	uint8 err;
	uint16 len;
	uint8 Count = 0;

	for(;;)
	{
        //OSTimeDly(10*OS_TICKS_PER_SEC);
        /*begin:yangfei added 2013-02-28 for 
        大数据量主动去读，防止GPRS模块缓存满而掉线问题*/
        if(gUpdateBegin == 1)
        	{
        	err = 0;
        	}
		else
			{
			err=GprsGetIPDATA_xmz(gRecBuf_xmz,0,&len);
			}
		/*end:yangfei added 2013-02-28 */
		if(err==0)
			{
			debug_debug(gDebugModule[GPRS_MODULE],"enter jh");
			do{
	  			err=GprsGetIPDATA_jh(gRecBuf_xmz,0,&len);
	  			if(err)
				{
				//重试一次 
				//可能对调试过程中断点(或本地口来的数据)导致错误而彻底中断西门子模块发的^SISR: 1, 1(数据还没有完全读空时)
					debug_err(gDebugModule[GPRS_MODULE],"%s %d err=%d",__FUNCTION__,__LINE__,err);	
					/*begin:yangfei added 2013-02-28 needed deleted*/
					#if  0
					OSTimeDly(OS_TICKS_PER_SEC);
					#endif
					OSTimeDly(OS_TICKS_PER_SEC/8);
					/*end:yangfei added 2013-02-28 needed deleted*/
	  				err=GprsGetIPDATA_jh(gRecBuf_xmz,0,&len);
					if(err)
						{
						debug_err(gDebugModule[GPRS_MODULE],"%s %d err=%d",__FUNCTION__,__LINE__,err);
						}
	  			}
	  		/*begin:yangfei added 2013-02-28 needed deleted*/
				debug_info(gDebugModule[GPRS_MODULE],"GprsIPREC len=%d ok ",len);	
				if(len>400){gUpdateBegin = 1;}/*进入主动读取数据模式*/
				if(len == 0)
					{Count++;}
				else
					{Count = 0;}
				if(Count > 10){gUpdateBegin = 0;}/*连续10次无数据进入查询接受数据模式*/
			/*end:yangfei added 2013-02-28 needed deleted*/
	  		UpdGprsRunSta_FeedRecDog();	
	  		UpdGprsRunSta_AddFrmRecTimes(len);	
	  		UpRecQueueWrite(UP_COMMU_DEV_GPRS,(uint8*)gRecBuf_xmz,len);
	  		OSTimeDly(OS_TICKS_PER_SEC/4);
	  		}while(err==0&&len>0);	
			debug_debug(gDebugModule[GPRS_MODULE],"leave jh");
		}
	else
		{
		//debug_err(gDebugModule[GPRS_MODULE],"%s %d err=%d",__FUNCTION__,__LINE__,err);
		}
	} 
}

/********************************************************************************************************
**  函 数  名 称: TaskGprsIpd       									                               **
**	函 数  功 能: 西门子模块 								                                           **
**	输 入  参 数: pdata												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 为兼容多种模块设计	                                                               **
********************************************************************************************************/
void  TaskGprsIpd(void *pdata)
{
	//本任务没有任务狗,原因之一是它可能在一段时间处在挂起状态

	pdata = pdata;  
	OSTaskSuspend(OS_PRIO_SELF);	
	
	if(GetGprsRunSta_ModuId()==MODU_ID_XMZ)
		{
			Fun_GprsIpd_xmz();
		}

}

/********************************************************************************************************
**  函 数  名 称: DlyLandFail       									                               **
**	函 数  功 能: 登录失败延时 								                                           **			
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void DlyLandFail(void)
{
	uint8 i;
	for(i=0;i<6;i++){
		FeedTaskDog();
		OSTimeDly(10*OS_TICKS_PER_SEC);
	}
}

/********************************************************************************************************
**  函 数  名 称: TaskGprsIpd       									                               **
**	函 数  功 能: 登录失败延时检测							                                           **
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void DlyGprsCheck(void)
{
	uint8 n,m;
	n=GPRS_CHECK_CYCLE/10;
	m=GPRS_CHECK_CYCLE%10;
	
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <DlyGprsCheck> Wait for GPRS Check Time Delay!");
	while(n--){
		FeedTaskDog();
		OSTimeDly(10*OS_TICKS_PER_SEC);
	}
	if(m){
		OSTimeDly(m*OS_TICKS_PER_SEC);
		FeedTaskDog();
	}
}

/********************************************************************************************************
**  函 数  名 称: ConnectConfirm       									                               **
**	函 数  功 能: 登录确认，失败并重试						                                           **
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注:    					                                                               **
********************************************************************************************************/
uint8 ConnectConfirm(void)
{
	uint8 err,TryT;
  /*	
	uint8 CurSocketType;

	CurSocketType=ReadCurSType();
  
	if(CurSocketType==0){//TCP
		OSMboxPost(HeartFrmMbox,(void*)1);//通知主动上报任务发登陆帧
		return TRUE;
	}
  */
	//UDP
	TryT=0;
	while(1)
	{
		FeedTaskDog();
		OSTimeDly(2*OS_TICKS_PER_SEC);
		OSMboxPost(HeartFrmMbox,(void*)1);//通知主动上报任务发登陆帧
		OSMboxAccept(UpLandAnswMbox);//读空非预期的消息
		TryT++;
		OSMboxPend(UpLandAnswMbox,OS_TICKS_PER_SEC*10,&err);
		//err = OS_ERR_NONE;		//for test
		if(err==OS_ERR_NONE)
			{//收到回应
				LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <ConnectConfirm> Recvie GPRS UpLand Confirm Frame OK!");
				break;
			}
		if(TryT>=5)
			{
				LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: <ConnectConfirm> Retry five Times! GPRS UpLand Failure!");
				return FALSE; //5次没有收到登陆帧回应
			}
		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <ConnectConfirm> Retry Times is %d", TryT);
	}
	return TRUE;
	
}

/********************************************************************************************************
**  函 数  名 称: SuspendGprsRecTask       									                           **
**	函 数  功 能: 挂起GPRS接收任务							                                           **
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注:                                                                                      **
********************************************************************************************************/
void SuspendGprsRecTask(void)
{
	uint8 err;
	//在挂起GPRS接收任务前获取它可能正在占用的信号量GprsXmzSem,此信号量上行发送任务也可能申请,
	//如果不做此处理,可能导致上行发送任务死掉
	OSSemPend(GprsXmzSem, 0, &err);
	OSTaskSuspend(PRIO_TASK_GPRS_IPD);//挂起GPRS接收任务
	OSSemPost(GprsXmzSem);
}

/********************************************************************************************************
**  函 数  名 称: SuspendGprsRecTask       									                           **
**	函 数  功 能: 挂起GPRS接收任务							                                           **
**	输 入  参 数: none												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注:                                                                                      **
********************************************************************************************************/
uint8 gGprsFirst;
void  TaskGprsMana(void *pdata)
{
	uint8 tmpmid;
	uint8 err;
	uint32 ReStartCounter = 0x00;
	uint32 HeartFrmSndCycles;
	uint32 Cycles;
	uint32 selfchecknum;
	uint8 lu8csq = 0;
	
	GPRS_RUN_STA GprsRunSta;
	UP_COMM_MSG UpCommMsg;
	CPU_SR	cpu_sr;
	
	pdata = pdata;  
	#if TEST_SELF_CHECK ==0
	uint32 heartcy_sec;//心跳周期，单位秒
	#endif
	#if TEST_SELF_CHECK ==1
	Dog_OSTaskSuspend(OS_PRIO_SELF);
	#endif
	
	gGprsFirst=TRUE;

/*	#if TEST_SELF_CHECK ==1
	tmpmid=GetGprsRunSta_ModuId();
	if(tmpmid==MODU_ID_XMZ){
		err=SetXmzBsp();
		if(err){
			Dog_OSTaskSuspend(OS_PRIO_SELF);
		}
	}
	#endif*/
	
	#if GPRS_HEART_FRM_TIME%GPRS_CHECK_CYCLE == 0
		HeartFrmSndCycles=GPRS_HEART_FRM_TIME/GPRS_CHECK_CYCLE;
	#else
		HeartFrmSndCycles=GPRS_HEART_FRM_TIME/GPRS_CHECK_CYCLE+1;
	#endif
	
	UpdGprsRunSta_ModuId(MODU_ID_UNKNOWN);
	InitGprsRunSta();//GPRS没有就绪
	tmpmid=ModelIdentify();
	if(tmpmid == MODU_ID_UNKNOWN)
	{
		SuspendGprsRecTask();
		OSTaskSuspend(OS_PRIO_SELF);
	}
	UpdGprsRunSta_ModuId(tmpmid);
	for(;;)
	{
	  	//OSTaskSuspend(OS_PRIO_SELF);	//GPRS任务挂起
	    //OSTimeDly(10*OS_TICKS_PER_SEC);
		ReStartCounter++;
		if(ReStartCounter > 1)
			{
			LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: [TaskGprsMana] GPRS Modul Restart, Retry Times is %2d", ReStartCounter);
			debug_info(gDebugModule[GPRS_MODULE],"WARNING: [TaskGprsMana] GPRS Modul Restart, Retry Times is %2d", ReStartCounter);
			}
		else
			{
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskGprsMana] GPRS Modul Start!");
			debug_info(gDebugModule[GPRS_MODULE],"INFO: [TaskGprsMana] GPRS Modul Start!");
			}
		/*begin:yangfei 2013-01-14 modified for uart1*/
		#if 1
		if(ReStartCounter >= 5)
		{
			 //SuspendGprsRecTask();//挂起GPRS接收任务
			 //此处可切换到短信模式
			 //OSTaskSuspend(OS_PRIO_SELF);	//GPRS任务挂起
		}
		#else
		if(ReStartCounter >= 1)
		{
			 SuspendGprsRecTask();//挂起GPRS接收任务
			 //此处可切换到短信模式
			 OSTaskSuspend(OS_PRIO_SELF);	//GPRS任务挂起
		}
		#endif
		/*end:yangfei 2013-01-14 modified for uart1*/
	    FeedTaskDog();		
		InitGprsRunSta();//GPRS没有就绪时 ，TaskUpSend不会使用设备 UP_COMMU_DEV_AT
		
		SuspendGprsRecTask();//挂起GPRS接收任务 
		
		tmpmid=GetGprsRunSta_ModuId();
		
		if(tmpmid==MODU_ID_XMZ)
			{
				GprsInit_xmz(); //使用设备 UP_COMMU_DEV_AT
			}
	    
	  	UpdGprsRunSta_Ready(TRUE);
		//gprs ready后，设备 UP_COMMU_DEV_AT的使用权交给TaskUpSend，下面本任务将
		//定时检查GPRS状态，确定是否需要重新启动，查询GPRS状态使用的是设备UP_COMMU_DEV_AT，故本任务不直接使用，
	  	//而是向TaskUpSend发出请求，TaskUpSend将结果写入全局状态信息
	  	//是否本地口也限制？也就是显示写串口1
	  	
	  	//OSTimeDly(2*OS_TICKS_PER_SEC);
	  	
	  	OSTaskResume(PRIO_TASK_GPRS_IPD);//恢复GPRS接收任务

	  	if(ConnectConfirm()==FALSE){
	  		DlyLandFail();
	  		continue;
	  	}
	
	  	//OSMboxPost(HeartFrmMbox,(void*)1);//通知主动上报任务发心跳帧,第1帧（登陆帧）
	  	//==========如果发完登陆帧后马上还要发1个心跳帧,则应加下面两句,否则,在1个心跳周期后才开始发心跳帧
	  	//OSTimeDly(OS_TICKS_PER_SEC);
	  	//OSMboxPost(HeartFrmMbox,(void*)3);////通知主动上报任务发心跳帧
	  	//==========
	  	////
	  	UpdGprsRunSta_Cont(TRUE);
	  	selfchecknum=0;
	  	FeedTaskDog();
	  	while(1){
	  		//OSTimeDly(GPRS_CHECK_CYCLE*OS_TICKS_PER_SEC);//挂起一个GPRS检查周期
	  		DlyGprsCheck();

			if(CMD_Csq(&lu8csq)==0)  //获取信号强度。
           	{
            	UpdGprsRunSta_Csq(lu8csq);
            }

			/*begin:yangfei added 2013-02-28 for test needed modified*/	
			#if 1
	  		UpdGprsRunSta_AddSndDog();
	  		UpdGprsRunSta_AddRecDog();
			#endif
	  		/*end:yangfei added 2013-02-28 */	  			  		
	  		selfchecknum=(selfchecknum+1)%(GPRS_SELF_CHECK_CYCLE/GPRS_CHECK_CYCLE);
	  		if(selfchecknum==0){
	  			//========发送消息，通知发送任务检查GPRS状态，结果填入gGprsRunSta
	  			UpCommMsg.GprsGvc=TRUE;
	  			OSQPost(UpSend_Q,(void*)(&UpCommMsg));
	  			OSMboxPend(GprsManaMbox, 0, &err);
	  			//==========
	  		}
	  		
	  		ReadGprsRunSta(&GprsRunSta);
	  		
	  		if(GprsRunSta.IpCloseNum>=2){
	  			tstn1++;
	  			
	  			LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: [TaskGprsMana] GprsRunSta.IpCloseNum>=2! Modul Restart!");
				debug_info(gDebugModule[GPRS_MODULE],"WARNING: [TaskGprsMana] GprsRunSta.IpCloseNum>=2! Modul Restart!");
	  			#if   TEST_SELF_CHECK == 0
	  			break;//重新启动GPRS
	  			#endif
	  		}

	  		if(GprsRunSta.IpSendFailNum>=10){
	  			tstn2++;
	  			;
	  		}
	  		
	  		if(GprsRunSta.IpSendRemnBufNum<=3){
	  			tstn3++;
	  			;
	  		}
	  		
	  		#if   TEST_SELF_CHECK == 0
	  			OS_ENTER_CRITICAL();
	  			//heartcy_sec=GlobalVariable.HeartCycle;
				heartcy_sec = 60;
	  			OS_EXIT_CRITICAL();
	  			if(heartcy_sec!=0&&heartcy_sec<60){
	  				HeartFrmSndCycles=(heartcy_sec*60)/GPRS_CHECK_CYCLE;
					
				}
	  			else{
	  				HeartFrmSndCycles=GPRS_HEART_FRM_TIME/GPRS_CHECK_CYCLE;
	  			}
	  		
	  		#endif
	  		
	  		
	  		Cycles=GPRS_HEART_FRM_REC_OUTTIME/GPRS_CHECK_CYCLE;
	  		Cycles+=HeartFrmSndCycles;
	  		if(GprsRunSta.RecDog>Cycles){
	  			tstn4++;
	  			
	  			LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: [TaskGprsMana] Heart Frame Recive Out Time! Module will Restart!");
				debug_info(gDebugModule[GPRS_MODULE],"WARNING: [TaskGprsMana] Heart Frame Recive Out Time! Module will Restart!");
	  			break;//重新启动GPRS
	  		}
	  		
	  		//不一定需要 当模块返回发送失败时，不会喂狗，这个值就会超过HeartFrmSndCycles，越来越大
	  		if(GprsRunSta.SndDog>HeartFrmSndCycles+5){ 
	  			tstn5++;
	  			
	  			LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: [TaskGprsMana] Heart Frame Send Failure! Modul Restart!");
				debug_info(gDebugModule[GPRS_MODULE],"WARNING: [TaskGprsMana] Heart Frame Send Failure! Modul Restart!");
	  			break;//重新启动GPRS
	  		
	  		}
	  		//不一定需要
	  		
	  		if(GprsRunSta.SndDog>=HeartFrmSndCycles){
	  			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskGprsMana] Post HeartFrmMbox to [TaskReportUp]!");
	  			OSMboxPost(HeartFrmMbox,(void*)3);//通知主动上报任务发心跳帧
				ClearCommunication();

	  		}

	  	} 
	} 	
}

void CreateTasks3(void)
{
	uint8 err;
	
    err=OSTaskCreate(TaskGprsIpd, (void *)0, &TaskGprsIpdStk[TASK_STK_SIZE - 1],PRIO_TASK_GPRS_IPD);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
    err=OSTaskCreate(TaskGprsMana, (void *)0, &TaskGprsManaStk[TASK_STK_SIZE - 1],PRIO_TASK_GPRS_MANA);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
}
void CreateMboxs3(void)
{
	GprsManaMbox= OSMboxCreate((void*)0);
	if (GprsManaMbox == NULL)
    {
        while (1);
    }
    UpLandAnswMbox= OSMboxCreate((void*)0);
	if (UpLandAnswMbox == NULL)
    {
        while (1);
    }
    
	HeartFrmMbox = OSMboxCreate(NULL);
    if (HeartFrmMbox == NULL){
		while(1);
    }

	GprsXmzSem = OSSemCreate(1);
    if (GprsXmzSem == NULL)
    {
        while(1);
	}

}
void CreateMutexs3(void)
{
	
}		
