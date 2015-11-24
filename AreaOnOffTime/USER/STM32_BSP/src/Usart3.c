
/********************************************************************

********************************************************************/
#include <includes.h>


#define USART3_SEND_QUEUE_LENGTH			2072 			//给UART2发送数据队列分配的空间大小,单位：字节

#define USART3_REC_QUEUE_AT_LENGTH   		4000
#define USART3_REC_QUEUE_ATIPD_LENGTH   		10000

//注意：**QUEUE_LENGTH 包含队列指针及函数指针，实际上用来存数据的是**QUEUE_LENGTH-24 个字节
static 	uint8 USART3SendQueue[USART3_SEND_QUEUE_LENGTH];
/*begin:yangfei modified 2013-03-01*/
#if  1
uint8 USART3RecQueue_At[USART3_REC_QUEUE_AT_LENGTH];
#else
uint8 USART3RecQueue_At[10000];
#endif		
uint8 IRQ3testBuf[32] = {0x00};//muxiaoqing test
uint8 IRQ3testBuf_Counter = 0;

/*end:yangfei modified*/
uint8 USART3RecQueue_AtIPD[USART3_REC_QUEUE_ATIPD_LENGTH];
#if TEST_TASK_TEST_EN == 1
//int USART3RecQueue_Test[(USART3_REC_QUEUE_ATIPD_LENGTH + sizeof(int) - 1) / sizeof(int)];
uint8 USART3RecQueue_Test[USART3_REC_QUEUE_ATIPD_LENGTH];
#endif
/*begin:yangfei modified*/
//static uint8 IRQBuf[64] = {0x00};
static uint8 IRQBuf[128] = {0x00};
/*end:yangfei modified*/
static void *Q_IRQBuf[64];
static uint8 IRQBuf_Counter = 0;

static OS_EVENT *USART3QMsg;									//中断到任务的消息邮箱
	   OS_EVENT *USART3SendQueSem;								//发送数据的信号量
	   
OS_EVENT *USART3RecQueSem_At; //与接收队列绑定的信号量
OS_EVENT *USART3RecQueSem_AtIPD; //与接收队列绑定的信号量
#if TEST_TASK_TEST_EN == 1
OS_EVENT *USART3RecQueSem_Test;
#endif
/*begin:yangfei added 2013-02-27*/
uint8 write_USART3RecQueue_Test(DataQueue *Queue,QUEUE_DATA_TYPE Data,uint8 mode)
{
   Data = Data;
   mode = mode;
   QueueFlush(Queue);
   return 0;
}
/*end:yangfei added 2013-02-27*/
uint8 USART3_QueueCreate(void)
{
	//创建发送队列
    if (QueueCreate((void *)USART3SendQueue,
                     sizeof(USART3SendQueue),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
    
	if (QueueCreate((void *)USART3RecQueue_At,
                     sizeof(USART3RecQueue_At),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
    
	if (QueueCreate((void *)USART3RecQueue_AtIPD,
                     sizeof(USART3RecQueue_AtIPD),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
    
    #if TEST_TASK_TEST_EN == 1
	if (QueueCreate((void *)USART3RecQueue_Test,
                     sizeof(USART3RecQueue_Test),
                     NULL,
                     /*NULL*/write_USART3RecQueue_Test)
                     == NOT_OK){
		while(1);
    }
	#endif
    
    USART3QMsg= OSQCreate(&Q_IRQBuf[0], 32);
	if (USART3QMsg == NULL)
    {
        while (1);
    }
    
    //创建与发送队列绑定信号量
	USART3SendQueSem = OSSemCreate(0);
    if (USART3SendQueSem == NULL){
        while(1);
    }
    
    //创建与接收队列绑定信号量
	USART3RecQueSem_At = OSSemCreate(0);
    if (USART3RecQueSem_At == NULL){
        while(1);
    }
    
	USART3RecQueSem_AtIPD = OSSemCreate(0);
    if (USART3RecQueSem_AtIPD == NULL){
        while(1);
    }
    
	#if TEST_TASK_TEST_EN == 1
    USART3RecQueSem_Test = OSSemCreate(0);
    if (USART3RecQueSem_Test == NULL){
        while(1);
    }
	#endif
    
	return 0;
}

/*********************************************************************************************************
** 函数名称: __UART0Putch
** 功能描述: 发送一个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: 无
** 说明： 调用本函数前必须关中断！内部使用
********************************************************************************************************/
static void __USART3Putch(uint8 Data)
{
	QueueWrite_Int((void *)USART3SendQueue, Data); 					//数据入队列
	
	/*
	if(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == SET)			//UART0发送保持寄存器空
	{ 									
		QueueRead_Int(&tmp, UART0SendQueue); 						//发送最初入队的数据
		//说明，这里读队列没有判断返回值，是因为此时是关中断的，而上面刚写队列了。至于在开中断状态下可以不可以不判断返回值，没有仔细考虑
		USART3->DR = tmp;
	}*/
}

/*********************************************************************************************************
** 函数名称: USART3Send
** 功能描述: 发送多个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: __UART0Putch
** 特别说明：本函数不可重入，若多个任务使用本函数，则必须在调用前申请信号量。并且，由于没有使用OSSchedLock(见函数UART0Send_L)，
**           也就没有解决"防止低优先级任务调用本函数过程中被长时间剥夺CPU使用权，而导致发出的数据出现较长时间的间断"
**           的问题
********************************************************************************************************/
void USART3Send(uint8* buf,uint32 n)
{
	OS_CPU_SR cpu_sr;
	uint32 N=3; //为了减少系统关中断时间,并减少开关中断次数，这里采用写N个字节关开一次中断
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=10时，测试得出关中断时间约为63us，
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=6时，测试得出关中断时间约为40us，(见文件末尾说明)
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=3时，测试得出关中断时间约为23us，
				//为了降低系统的中断延迟时间，原则上N 不应大于6。
	uint32 i,j,p,x,y;
	
	//======================等待发送队列可用空间>=n
	if(QueueNotEnoughPend((void*)USART3SendQueue,n)){
		return ;
	}
	//======================
	x=n/N;	y=n%N;	p=0;
	for(i=0;i<x;i++){
		OS_ENTER_CRITICAL();
		for(j=0;j<N;j++){
			__USART3Putch(buf[p++]);
		}
		OS_EXIT_CRITICAL();
	}

	OS_ENTER_CRITICAL();
	for(j=0;j<y;j++){
		__USART3Putch(buf[p++]);
	}
	OS_EXIT_CRITICAL();
	
	//以上均为 将需要发送的数据，写入队列，写入完成后开启发送缓冲寄存器为空中断，开始中断发送
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	
}

/*********************************************************************************************************
** 函数名称: UART0Putch
** 功能描述: 发送一个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: Uart0Send
********************************************************************************************************/
void USART3Putch(uint8 Data)
{
	USART3Send(&Data,1);
}


/*********************************************************************************************************
** 函数名称: USART3Getch
** 功能描述: 从USART3接收队列读一个字节，若队列内无数据则按OutTime做超时等待. 供上层调用
** 输　入: OutTime,等待超时时间
** 输　出: data 接收到的数据
** 全局变量: 无
** 调用模块:
** 特别说明： ！！！！！！！！！！此函数不可重入！！！！！！！！！！
********************************************************************************************************/

uint8 USART3Getch(uint8* data, uint16 OutTime)
{
	uint8 err;
	
	while (QueueRead(data,(void*)USART3RecQueue_At)!=QUEUE_OK)
	{
        OSSemPend(USART3RecQueSem_At, OutTime, &err); //等待接收数据
        FeedTaskDog();
    	if(err!=OS_ERR_NONE)
    		{
    			return err;
			}
	}
	return 0;
}

/*********************************************************************************************************
** 函数名称: TaskUSART3Rec
** 功能描述: UART2接收任务，将收到的数据写入UART0接收队列，此任务应具有较高的优先级
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
********************************************************************************************************/

//消息队列方式
void TaskUSART3Rec(void *pdata)
{
	uint8 err;
	uint8 *pReadData = NULL;	

	pdata = pdata;
	IRQBuf_Counter = 0x00;
	QueueFlush((void*)USART3RecQueue_At); 				//清空接收队列
	QueueFlush((void*)USART3RecQueue_AtIPD); 			//清空接收队列
	#if TEST_TASK_TEST_EN == 1
	QueueFlush((void*)USART3RecQueue_Test); 				//清空接收队列
	#endif
	memset(IRQBuf, 0x00, 128);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	
	while(1)
	{
		do{
			FeedTaskDog();
			pReadData = (uint8 *)OSQPend(USART3QMsg, OS_TICKS_PER_SEC*6, &err);
		}while( (err != OS_ERR_NONE) || (pReadData == NULL) );
		/*begin:yangfei added 2013-02-22*/
        //debug("%c",*pReadData);//Undervoltage
        /*end:yangfei added */
		QueueWrite((void*)USART3RecQueue_At, *pReadData); 		//将收到的数据写入队列
		QueueWrite((void*)USART3RecQueue_AtIPD, *pReadData); 	//将收到的数据写入队列
		#if TEST_TASK_TEST_EN == 1
		QueueWrite((void*)USART3RecQueue_Test, *pReadData); 	//将收到的数据写入队列
		#endif
		
		OSSemPost(USART3RecQueSem_At);
		OSSemPost(USART3RecQueSem_AtIPD);
		#if TEST_TASK_TEST_EN == 1
		OSSemPost(USART3RecQueSem_Test);
		#endif
	}
}


/*********************************************************************************************************
** 函数名称: USART3_IRQHandler
** 功能描述: UART2接收中断
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
********************************************************************************************************/

void USART3_IRQHandler(void)
{
    CPU_SR		cpu_sr;
    uint8 Err = 0x00;
	uint8 tmp = 0x00;
	
	
	OS_ENTER_CRITICAL();
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	//有产生 Tansmit Data Register empty interrupt
	if(USART_GetITStatus(USART3, USART_IT_TXE) == SET)
		{
 			Err = QueueRead(&tmp, USART3SendQueue);
			if (Err == QUEUE_OK)
				{
					USART3->DR = tmp;
				}
			else if(Err == QUEUE_EMPTY)
				{
					USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
				}
			
		}
	else if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
			{ //产生 Receive Data register not empty interrupt
				IRQBuf[IRQBuf_Counter] = USART3->DR;
				OSQPost(USART3QMsg, &IRQBuf[IRQBuf_Counter]);
				if(IRQBuf_Counter>=126)	IRQBuf_Counter = 0;
				IRQBuf_Counter++;
				
				//debug("%c",IRQBuf[IRQBuf_Counter]);
			       //debug_debug(gDebugModule[GPRS_MODULE],"%c",IRQBuf[IRQBuf_Counter]);
			    //{
				//	IRQ3testBuf[IRQ3testBuf_Counter]=IRQBuf[IRQBuf_Counter];
				//	IRQ3testBuf_Counter ++;
				//	if(IRQ3testBuf_Counter>=18)	
         		//	{
         		//	IRQ3testBuf_Counter = 0;
         		//	}
				//}   

			}
		else if( (USART_GetFlagStatus(USART3,USART_FLAG_ORE)==SET) || 
				 (USART_GetFlagStatus(USART3,USART_FLAG_NE)==SET) ||
				 (USART_GetFlagStatus(USART3,USART_FLAG_FE)==SET) ||
				 (USART_GetFlagStatus(USART3,USART_FLAG_PE)==SET) )
    			{//溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题
        			USART_ClearFlag(USART3, USART_FLAG_ORE);    //读SR
        			tmp = USART3->DR;							//读DR
    			}
			else
				{
					//清除中断标志
					USART_ClearITPendingBit(USART3, USART_IT_TC);
				}
	OSIntExit();
}

