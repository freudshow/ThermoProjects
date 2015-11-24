/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: uart0pro.c
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年8月10日 
**  描        述: 下行串口抄读数据处理
**	修 改  记 录:   	
*****************************************************************************************************/

/********************************************** include *********************************************/
#include <includes.h>
#include "app_down.h"
#include "tasks.h"

/********************************************** define *********************************************/

/********************************************** global *********************************************/
OS_EVENT *DownUartRequestQ;												//申请使用DownUart的消息队列
OS_EVENT *DownUartAnswerMbox_UAF;
OS_EVENT *DownUartAnswerMbox_CLK;
OS_EVENT *DownUartAnswerMbox_MBUS;
OS_EVENT *DownUartAnswerMbox_LCM;

/********************************************** static *********************************************/

/****************************************************************************************************
**	函 数  名 称: GetDownUartAnswMbox
**	函 数  功 能: 查找是哪个任务的消息邮箱消息
**	输 入  参 数: uint8 TaskPrio -- 任务优先级
**	输 出  参 数:  
**  返   回   值: OS_EVENT* -- 返回消息邮箱
**	备		  注: 为了通过一个任务管理下行抄表，并将抄表结果返回给各个不同的任务
*****************************************************************************************************/
OS_EVENT* GetDownUartAnswMbox(uint8 TaskPrio)
{
	LOG_assert_param(TaskPrio > OS_LOWEST_PRIO);
	
	switch (TaskPrio)
    {
        case PRIO_TASK_UP_ANAL_FR:
              return (DownUartAnswerMbox_UAF);
        
        case PRIO_TASK_CLOCK:             
            return (DownUartAnswerMbox_CLK);

        case PRIO_TASK_READ_ALL_CUR:
              return (DownUartAnswerMbox_MBUS);

        case PRIO_TASK_LCM:
              return (DownUartAnswerMbox_LCM);
          
        default :
            return ((OS_EVENT*)0);
    }	
}

/****************************************************************************************************
**	函 数  名 称: Uart0SendThenReceive_PostPend
**	函 数  功 能: 下行抄表，发送之后等待接收数据
**	输 入  参 数: uint8 *buf -- 需发送的数据;	uint8 *n -- 需发送的字节数
**	输 出  参 数: uint8 *buf -- 该空间复用，输出接收到的数据
**  返   回   值: uint8* -- 返回接收到的消息邮箱数据地址
**	备		  注: 本函数先发送数据，之后等待接收数据，并返回接收到的数据
*****************************************************************************************************/
uint8* Uart0SendThenReceive_PostPend(uint8 *buf, uint8 *n)
{
	
	uint8 Err			= 0x00;
	uint8 TaskPrio;
	uint8 *cp 			= NULL;
	uint8 MsgBuf[METER_FRAME_LEN_MAX] = {0x00};
	OS_EVENT *Mbox_Answ	= NULL;
	CPU_SR	cpu_sr;
	
	LOG_assert_param(buf == NULL);
	LOG_assert_param(n == NULL);
	LOG_assert_param(*n >= METER_FRAME_LEN_MAX);

	OS_ENTER_CRITICAL();
	TaskPrio = OSPrioCur;
	OS_EXIT_CRITICAL();
	
   	Mbox_Answ = GetDownUartAnswMbox(TaskPrio);
	if(Mbox_Answ == (OS_EVENT*)0)
		{
			//LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: <Uart0SendThenReceive_PostPend> GetDownUartAnswMbox Error!");
			while(1);
		}
	
	MsgBuf[0] = TaskPrio;								//发消息的任务
	MsgBuf[1] = *n;										//发送的字节数
	
	memcpy(&MsgBuf[2], buf, MsgBuf[1]);					//复制消息的数据域

    OSQPost(DownUartRequestQ, (void *)MsgBuf);
    //LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <Uart0SendThenReceive_PostPend> Send Data From DownUart!");
    
	cp = (uint8 *)OSMboxPend(Mbox_Answ, 0, &Err);
	if(cp[0])
		{												//出错
			//LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: <Uart0SendThenReceive_PostPend> Recive Data Time Out, The Err is %d!", cp[0]);
			return ((uint8*)cp);
		} 
	else
	{
		//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <Uart0SendThenReceive_PostPend> Recive Data Successful!"); 
	    memcpy(&buf[0], &cp[2], cp[1]);
	    *n = cp[1];
		return ((uint8*)cp);
	}
}

/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
