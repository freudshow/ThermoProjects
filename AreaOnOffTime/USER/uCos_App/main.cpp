/*----------------------------------------------------------------------------
 *      
 *----------------------------------------------------------------------------
 *      Name:    app.C
 *      Purpose: 
 *----------------------------------------------------------------------------
 *      
 *      Copyright (c) 2004-2009          All rights reserved.
 *---------------------------------------------------------------------------*/


/**********************************************************************************************************
*                                             INCLUDE FILES
**********************************************************************************************************/
	
#include <includes.h>

#include "tasks.h"
#include "tasks_up.h"
#include "app_up.h"
#include "lcd_menu.h"
/**********************************************************************************************************
*                                      extern task create FUNCTION
**********************************************************************************************************/

#if (OS_VIEW_MODULE == DEF_ENABLED)
extern void  App_OSViewTaskCreate (void);
#endif

/* extern GLOBAL VARIABLES*/
extern uint16 gVersion;
/**********************************************************************************************************
*                                            LOCAL DEFINES
********************************************************************************************************* */



/**********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
**********************************************************************************************************/

static  OS_STK         App_TaskStartStk[APP_TASK_START_STK_SIZE];

uint8 gTASK_DeadTask;
uint8 gDebugLevel = 4;

uint8 gRestartFlag = FALSE;

/**********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
**********************************************************************************************************/

static  void  App_TaskStart			(void		*p_arg);  

/**********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none.
*
* Return(s)   : none.
**********************************************************************************************************/

INT32S main (void)
{
    CPU_INT08U  os_err;
	os_err = os_err; /* prevent warning... */

	/* Note:  由于使用UCOS, 在OS运行之前运行,注意别使能任何中断. */
	CPU_IntDis();                    /* Disable all ints until we are ready to accept them.  */

    OSInit();                        /* Initialize "uC/OS-II, The Real-Time Kernel".         */


	os_err = OSTaskCreateExt((void (*)(void *)) App_TaskStart,  /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
                             (INT8U           ) APP_TASK_START_PRIO,
                             (INT16U          ) APP_TASK_START_PRIO,
                             (OS_STK        * )&App_TaskStartStk[0],
                             (INT32U          ) APP_TASK_START_STK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
	
	BSP_FSMC_IO_Init();
	BSP_SRAM_Init();
	LOG_QueueCreate();
	Createtasks_up();
    CreateMboxs_up();
    
    CreateTasks_Tasks();   //创建管理Tasks1()、Tasks2()和Tasks4()的任务
    
    CreateMboxs_Tasks();   //创建管理Mbox的任务
	CreateMutexs_Tasks();  //创建管理Mutexs的任务

	OSStart();                               	/* Start multitasking (i.e. give control to uC/OS-II).  */

	return (0);
}

/********************************************************************************************************
**  函 数  名 称: App_TaskStart  									                                   **
**	函 数  功 能: 任务开始                                                                             **			
**	输 入  参 数: void 													                               **
**	输 出  参 数: none											                                       **
**  返   回   值: none																			   	   **
**	备		  注: 本任务具有最好优先级，对硬件进行及全局变量等进行初始化，监控任务看门狗						                                                               **
********************************************************************************************************/	  
static  void  App_TaskStart ( void *p_arg )
{
	uint32 i = 0x00;
	uint8 num = 0x00;
	uint8 rest = 0;
	p_arg = p_arg;  			// 避免编译警告

	//uint8 chtest[] = {0x55,0x55,0x55,0x55,0x55,0xaa} ;
	/******************  Init hardware *****************/
	BSP_Init();
	//本系统内，由于大多数全局变量存放在外部SRAM中，没有进行编译器的自动赋0x00为初始值，
	//所以在此进行全部参数的设置初始值
	READ_ParaInit();
	PRO_InitGlobalVar();
	//PARA_InitGlobalVar();
	InitFeedDogFuns();
	/***************************************************/
	
    OS_CPU_SysTickInit();                                    /* Initialize the SysTick.                              */
#if (OS_TASK_STAT_EN > 0)
    OSStatInit();                                            /* Determine CPU capacity.                              */
#endif

//	OSTimeDly(OS_TICKS_PER_SEC*2);
//	PARA_InitGlobalVar();
	
	//任务开始前初始化看门狗
	IWDG_Configuration();
	/*begin:yangfei added 2013-01-24 for start log*/
	Uart_Printf("\r\n===============================================");
	Uart_Printf("\r\n=         (C) COPYRIGHT 2013 513所");
	Uart_Printf("\r\n=                                      ");
	Uart_Printf("\r\n=           app (Version %d.%d%d)          ",(gVersion>>8)&0x0f,(gVersion>>4)&0x0f,gVersion&0x0f);
	Uart_Printf("\r\n=                                      ");
	Uart_Printf("\r\n=        By 513 xi'an Application Team ");
	Uart_Printf("\r\n===============================================");
	Uart_Printf("\r\n*****Compile time:("__TIME__","__DATE__")*********\r\n");
	UartCmdtime(0,0);
	Uart_Printf("***********************************************\r\n");

	while(1)
    {
    	OSTimeDly(OS_TICKS_PER_SEC);
		//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [App_TaskStart] Per Second Check OK!");
		/*begin:yangfei added test*/
			{
			//uint8 SendBuff[]={0xaa,0x55};
		       //uint8 SendBuff[]={0xfe, 0xfe, 0x68, 0x20, 0x20, 0x58, 0x35, 0x72, 0x69, 0x55, 0x66, 0x04, 0x04, 0xa0, 0x17, 00, 0x99, 0x23, 0x16};
                     //UpDevSend(UP_COMMU_DEV_485,  SendBuff,  sizeof(SendBuff)); 
			}
	
		/*end:yangfei added test*/

		
		ProBltStaDog();
		
        #if WDT_TYPE ==1 
		IWDG_ReloadCounter();
        #endif
			
		
		if(gRestartFlag==TRUE)
		{
			rest++;
			if(rest>4)						//5秒后复位
			{
				NVIC_SETFAULTMASK();
				NVIC_GenerateSystemReset();
			}
		}
		else
		{
			rest=0;
		}
		

        num=(num+1)%64;  //本任务每64秒检查其他各任务工作是否正常 
        if(num)	continue;      

		for(i=0; i<OS_LOWEST_PRIO+1; i++)
			{
				if(gTasksWatchDog[i].Monitor==FALSE)
					{
						continue;
					}

				if(gTasksWatchDog[i].CounterPre==gTasksWatchDog[i].CounterCur)
					{
						gTASK_DeadTask=i;
						#if WDT_TASK_MONIT_EN == 1
						while(1);
						#endif 
					}
				else{
						gTasksWatchDog[i].CounterPre=gTasksWatchDog[i].CounterCur;
					}
			}//由于本任务优先级最高，所以这里对gTasksWatchDog的访问不需要关中断，各任务喂狗时要注意关中断
	}
	
}

/*********************************************************************************************************
  												END FILE
*********************************************************************************************************/
