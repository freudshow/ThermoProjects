#define   TASKDOG_GLOBALS
#include "includes.h"

void InitFeedDogFuns(void)
{
	uint8 i;

	for(i=0;i<OS_LOWEST_PRIO+1;i++){ 
	 	gTasksWatchDog[i].CounterPre=0;
	 	gTasksWatchDog[i].CounterCur=0;
	 	gTasksWatchDog[i].Monitor=FALSE;
	} 
	
	//===注册要监视的任务
	gTasksWatchDog[PRIO_TASK_I2C_MANAGEMENT].Monitor		=TRUE;
//	gTasksWatchDog[PRIO_TASK_UART5_REC].Monitor				=TRUE;
	gTasksWatchDog[PRIO_TASK_UART4_REC].Monitor				=TRUE;
	gTasksWatchDog[PRIO_TASK_USART1_REC].Monitor			=TRUE;
	gTasksWatchDog[PRIO_TASK_USART2_REC].Monitor			=TRUE;
	gTasksWatchDog[PRIO_TASK_DOWN_UATR_MANAGEMENT].Monitor	=TRUE;
	gTasksWatchDog[PRIO_TASK_UP_SEND].Monitor				=TRUE;
	gTasksWatchDog[PRIO_TASK_UP_ANAL_FR].Monitor			=TRUE;
	gTasksWatchDog[PRIO_TASK_UP_REC_ZIGBEE].Monitor			=TRUE;
	gTasksWatchDog[PRIO_TASK_CLOCK].Monitor					=TRUE;
	gTasksWatchDog[PRIO_TASK_READ_ALL_CUR].Monitor			=TRUE;
	gTasksWatchDog[PRIO_TASK_LOG].Monitor					=TRUE;

	#if   TEST_SELF_CHECK > 0
	//gTasksWatchDog[PRIO_TASKTEST].Monitor=TRUE;
	#endif
}

void SetTaskDogMon(uint8 prio,uint8 Monitor)
{
	CPU_SR		cpu_sr;
	OS_ENTER_CRITICAL();
	if(prio==OS_PRIO_SELF){
		prio=OSPrioCur;
	}
	gTasksWatchDog[prio].Monitor=Monitor;
	gTasksWatchDog[prio].CounterCur=gTasksWatchDog[prio].CounterPre+1;//，Monitor==TRUE时：恢复挂起后的喂狗，Monitor==FLASE时无意义
	OS_EXIT_CRITICAL();
}
void Dog_OSTaskSuspend(uint8 prio)
{
	SetTaskDogMon(prio,FALSE);
	OSTaskSuspend(prio);
	if(prio==OS_PRIO_SELF){//如果是自己将自己挂起的
		SetTaskDogMon(prio,TRUE);
	}
}

//注意：在占用MUTEX的时候调用本函数及调用本函数的函数，喂狗作用可能无效，因为此时任务的优先级可能被提升而改变
void FeedTaskDog(void)
{
	#if WDT_TASK_FEED_EN == 1
	CPU_SR		cpu_sr;
	
	OS_ENTER_CRITICAL();
	gTasksWatchDog[OSPrioCur].CounterCur++;
	OS_EXIT_CRITICAL();
	
	#endif
}

void FeedTaskDog_Int(void)//说明：此函数只能在临界代码内调用
{
	#if WDT_TASK_FEED_EN == 1
	
	//OS_ENTER_CRITICAL();
	gTasksWatchDog[OSPrioCur].CounterCur++;
	//OS_EXIT_CRITICAL();
	
	#endif
}
