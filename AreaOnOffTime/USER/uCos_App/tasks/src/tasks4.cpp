/*******************************************Copyright (c)*******************************************
**									山东华宇空间技术公司(西安分部)                                **                            
**  文   件   名: tasks4.cpp																      **
**  创   建   人: 勾江涛																		  **
**	版   本   号: 0.1																		      **
**  创 建  日 期: 2012年9月20日 													     	      **
**  描        述: 设定键盘操作        			      						                      **
**	修 改  记 录:   																			  **
****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>
#include "app_flashmem.h"
#include "app_down.h"
#include "lcd_menu.h"
#include "tasks4.h"

/********************************************** static **********************************************/
static OS_STK  TaskKeyStk[256];       //无符号整形
static OS_STK  TaskLcmStk[TASK_STK_SIZE];	//TASK_STK_SIZE = 1024
static OS_STK  TaskLcmTopStk[TASK_STK_SIZE];
#ifdef   _MBUS_DETACH_
static OS_STK  TaskPwrStk[TASK_STK_SIZE];
#endif
static void* KeyMessageGrp[KEY_MSG_SIZE]; //  KEY_MSG_SIZE = 6 键盘消息队列的指针数组的大小
uint8  EXTI_ERRO_Happen =0;
uint8  Powerdown =0;

extern uint8 gRestartFlag ;

/************************************************extern***********************************************/
extern uint8 KeyMessageFlag;

extern uint8 autotestIsfromExitKey;//muxiaoqing test

/********************************************************************************************************
**  函 数  名 称: TaskKey       									                                   **
**	函 数  功 能: 键盘设置，分为上下左右、取消和进入操作                                               **			
**	输 入  参 数: pdata												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void  TaskKey ( void * pdata )
{
	uint8 err,i;
	uint8 KeyVal[2];
	pdata = pdata;        
//    uint8 test =0;
	for(;;)
	{
	 	
	 	OSMboxPend(GpioIntMbox,0,&err);
		
		for(i=0;i<2;i++)
		{
			OSTimeDly(OS_TICKS_PER_SEC/20);
			if(STA_KEY_CANCEL()==0){
				if(autotestIsfromExitKey ==1)
				{
				autotestIsfromExitKey=2;
			
				KeyVal[i]=KEY_EXIT;
				}
				else if((autotestIsfromExitKey ==2 )||(autotestIsfromExitKey ==0))
				KeyVal[i]=KEY_EXIT;
			}
			else if(STA_KEY_UP()==0){
				if(autotestIsfromExitKey ==1)//防止自动测试返回键退出后再按上下左右键重复运行函数
					{
					
					LCM_LIGHT_ENABLE; //打开液晶背光
					}
				else
				KeyVal[i]=KEY_UP;
			}
			else if(STA_KEY_DOWN()==0){
				
				if(autotestIsfromExitKey ==1)//防止自动测试返回键退出后再按上下左右键重复运行函数
					{
					LCM_LIGHT_ENABLE; //打开液晶背光

				}
				else
				KeyVal[i]=KEY_DOWN;
			}
			else if(STA_KEY_LEFT()==0){
				
				if(autotestIsfromExitKey ==1)//防止自动测试返回键退出后再按上下左右键重复运行函数
					{
					LCM_LIGHT_ENABLE; //打开液晶背光

				}
				else
				KeyVal[i]=KEY_LEFT;
			}
			
			else if(STA_KEY_RIGHT()==0){
				
				if(autotestIsfromExitKey ==1)//防止自动测试返回键退出后再按上下左右键重复运行函数
					{
					LCM_LIGHT_ENABLE; //打开液晶背光

				}
				else
				KeyVal[i]=KEY_RIGHT;
			} 
			else if(STA_KEY_OK()==0){
				KeyVal[i]=KEY_ENTER;
			}
			else{
				KeyVal[i]=NO_KEY;
				break;
			}
		}

		if(KeyVal[0]==NO_KEY)
			{
				OSTimeDly(OS_TICKS_PER_SEC/10);
			}
		else if(KeyVal[0]==KeyVal[1])
			{
				//OpenBell();
				//OSTimeDly(OS_TICKS_PER_SEC/10);
				//CloseBell();
				//此处暂时先不判断液晶屏亮灭
				if(ProBltSta()==TRUE)	//此次判断液晶的亮灭状态，若为亮则进行液晶操作，若灭，则不操作
					{
						OSQPost(KeyMessageQ,(void *)(KeyVal[0]));
					}
			}
			else
				{
					OSTimeDly(OS_TICKS_PER_SEC/10);
				} 			
   	}
}


/********************************************************************************************************
**  函 数  名 称: TaskPwr       									                                   **
**	函 数  功 能: 电源监控任务，进行断电保护
                                1.挂起一些任务；
                                2.打印power off消息
                                2.刷新log队列，将消息保存到日志并且上报给上位机。**			
**	输 入  参 数: pdata												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
#ifdef HW_VERSION_1_01
#ifdef   _MBUS_DETACH_

void  TaskGpioPolling(void *pdata)   
{
	uint8 err;	
	uint8 tmpmid;
	uint8 i = 0;
	pdata = pdata; 

	SDCardTest(); //在这里测试SD卡写入读出是否一致,放在循环外，只运行一次。
	
//   GPIOPwrInit();
	for(;;)
	{ 	
	 	OSMboxPend(GpioPwrMbox,0,&err);
		OSTimeDly(OS_TICKS_PER_SEC/20);
		#if 1
		if(EXTI_ERRO_Happen ==3)
			{
               
			   	//此处表示断电后系统重新获得上电
			   Powerdown =0;
			   LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: <TaskGpioPolling> Power on! ");
		    }
		else if(STA_PWR_OFF())
		{
		   // SuspendTask();
		   if(!Powerdown)//仅写一次日志，断电后该管脚不稳定有时候会在降压过程中产生多次中断
		   	{
    		   EXTI_ERRO_Happen =1;
    		   Powerdown =1;
    		   LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: <TaskGpioPolling> Power Off! ");
		   	}
		}
		else if(STA_MBUS_OFF())
		{
			// SuspendTask();
			
		//	EXTI_ERRO_Happen =2;
		//    MBUSOff = 1;
		
		//	LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: <TaskGpioPolling> MBUS Off! ");
            
		//    DISABLE_MBUS();
		}
		else if (GPIO_GPRS_CHECK_STATUS())
			{
			// SuspendTask();
			
			//EXTI_ERRO_Happen =1;
			//LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: <TaskGpioPolling> Power Off! ");
			
			BSP_Gprs_CtrInit_Xmz();
		//	OSTaskResume(PRIO_TASK_GPRS_IPD);//恢复GPRS接收任务
		UpdGprsRunSta_ModuId(MODU_ID_UNKNOWN);
		InitGprsRunSta();//GPRS没有就绪
		tmpmid=ModelIdentify();
		if(tmpmid == MODU_ID_UNKNOWN)
		{
		//	SuspendGprsRecTask();
		//	OSTaskSuspend(OS_PRIO_SELF);
		}
		else
			{
        		    UpdGprsRunSta_ModuId(tmpmid);
        			
        			OSTaskResume(PRIO_TASK_GPRS_MANA);//恢复GPRS 管理任务
			}

		    }
		
		else if (GPIO_SD_CHECK_STATUS())
			{
			// SuspendTask();
			
			//EXTI_ERRO_Happen =1;
			//LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: <TaskGpioPolling> Power Off! ");
			//MSD_Init();
			//mountSd();
			
			//PARA_InitGlobalVar();

		    }
		#endif


		


		
	}
}

//#undef STA_PWR_OFF()
#endif
#endif
/********************************************************************************************************
**  函 数  名 称: TaskLcm       									                                   **
**	函 数  功 能: 液晶显示任务                                                                         **			
**	输 入  参 数: pdata												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void  TaskLcm(void *pdata)
{
	pdata = pdata;        
	
	OSTimeDly(OS_TICKS_PER_SEC);
	
	RegisterMenu();
	
	PARA_InitGlobalVar();
//	gRestartFlag = FALSE;
    OSTimeDly(OS_TICKS_PER_SEC*1);
	//OSTaskSuspend(OS_PRIO_SELF);
	for(;;)
	{
	 	//OSTimeDly(OS_TICKS_PER_SEC);

	 	MenuTest();
	 	/*
	 	KeyVal = (uint32)OSQPend(KeyMessageQ, 0, &err);
	 	
	 	
	 	OSTimeDly(OS_TICKS_PER_SEC/5);
	 	//在这里响应键盘
	 	OpenTestLed2();
		OSTimeDly(OS_TICKS_PER_SEC/5);
		CloseTestLed2();*/
		
   	}
}
/********************************************************************************************************
**  函 数  名 称: TaskLcmTop       									                                   **
**	函 数  功 能: 循显任务                                                                             **			
**	输 入  参 数: pdata												                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void  TaskLcmTop(void *pdata)   
{
	uint8 err;
	pdata = pdata;   
	
	//#if TEST_SELF_CHECK ==1
	//OSTaskSuspend(OS_PRIO_SELF);
	//#endif
		
	for(;;)
	{
 		OSTimeDly(OS_TICKS_PER_SEC*3);
	 	OSSemPend(LcmSem, 0, &err);
	 	ShowMenuTop();

		ShowCommunication();//显示 "已登录"或"未登录"
		
	 	OSSemPost(LcmSem);

	 	KeyMessageFlag = 0;	

		  
   	}
}
/********************************************************************************************************
**  函 数  名 称: CreateTasks4     									                                   **
**	函 数  功 能: 创建TASKS4                                                                           **			
**	输 入  参 数: none								    			                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void CreateTasks4(void)
{
	uint8 err;
	
    err=OSTaskCreate(TaskKey, (void *)0, &TaskKeyStk[256 - 1],PRIO_TASK_KEY);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
    
    err=OSTaskCreate(TaskLcm, (void *)0, &TaskLcmStk[TASK_STK_SIZE - 1],PRIO_TASK_LCM);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
    
    err=OSTaskCreate(TaskLcmTop, (void *)0, &TaskLcmTopStk[TASK_STK_SIZE - 1],PRIO_TASK_LCM_TOP);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
#if 1//def HW_VERSION_1_01
	
       err=OSTaskCreate(TaskGpioPolling, (void *)0, &TaskPwrStk[256 - 1],35);
	if(err!=OS_ERR_NONE){
    	while(1);
    }
#endif   
}
/********************************************************************************************************
**  函 数  名 称: CreateMboxs4     									                                   **
**	函 数  功 能: 创建mboxs4                                                                          **			
**	输 入  参 数: none								    			                                   **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 						                                                               **
********************************************************************************************************/
void CreateMboxs4(void)
{

	LcmSem = OSSemCreate(1);
    if (LcmSem == NULL)
    {
       	while (1);
    }
		
	KeyMessageQ = OSQCreate(KeyMessageGrp, KEY_MSG_SIZE); 
	
	if (KeyMessageQ == NULL)
    {
        while (1);
    }
    
}
void CreateMutexs4(void)
{
	
}		


