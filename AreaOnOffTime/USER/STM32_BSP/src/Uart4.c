
/********************************************************************

********************************************************************/
#include <includes.h>


#define UART4_SEND_QUEUE_LENGTH				2072 			//给UART4发送数据队列分配的空间大小,单位：字节
#define UART4_REC_QUEUE_LENGTH			    2072 			//给UART4接收数据队列分配的空间大小,单位：字节

//注意：**QUEUE_LENGTH 包含队列指针及函数指针，实际上用来存数据的是**QUEUE_LENGTH-24 个字节
static uint8 UART4SendQueue[UART4_SEND_QUEUE_LENGTH];
	   uint8 UART4RecQueue[UART4_REC_QUEUE_LENGTH];

static uint8 IRQBuf[32] = {0x00};
static void *Q_IRQBuf[32];
static uint8 IRQBuf_Counter = 0;

static OS_EVENT *UART4QMsg;							//中断到任务的消息邮箱
	   OS_EVENT *UART4RecQueSem; 							//与接收队列绑定的信号量
	   OS_EVENT *UART4SendQueSem;							//发送数据的信号量
	   #ifdef   _MBUS_DETACH_
	   OS_EVENT *UART4SendCompleteSem;            //485发送完成信号量
	   #endif
	   uint8 IRQ4testBuf[32] = {0x00};//muxiaoqing test
	   uint8 IRQ4testBuf_Counter = 0;

uint8 UART4_QueueCreate(void)
{
	//创建发送队列
    if (QueueCreate((void *)UART4SendQueue,
                     sizeof(UART4SendQueue),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
	//创建接收队列
	if (QueueCreate((void *)UART4RecQueue,
                     sizeof(UART4RecQueue),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
    UART4QMsg= OSQCreate(&Q_IRQBuf[0], 32);
	if (UART4QMsg == NULL)
    {
        while (1);
    }
	//创建与接收队列绑定信号量
	UART4RecQueSem = OSSemCreate(0);
    if (UART4RecQueSem == NULL){
        while(1);
    }
    
    //创建与发送队列绑定信号量
	UART4SendQueSem = OSSemCreate(0);
    if (UART4SendQueSem == NULL){
        while(1);
    }
	#ifdef   _MBUS_DETACH_
	UART4SendCompleteSem = OSSemCreate(0);
    if (UART4SendCompleteSem == NULL){
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
static void __UART4Putch(uint8 Data)
{
	QueueWrite_Int((void *)UART4SendQueue, Data); 					//数据入队列
	
	/*
	if(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == SET)			//UART0发送保持寄存器空
	{ 									
		QueueRead_Int(&tmp, UART0SendQueue); 						//发送最初入队的数据
		//说明，这里读队列没有判断返回值，是因为此时是关中断的，而上面刚写队列了。至于在开中断状态下可以不可以不判断返回值，没有仔细考虑
		UART4->DR = tmp;
	}*/
}

/*********************************************************************************************************
** 函数名称: UART4Send
** 功能描述: 发送多个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: __UART0Putch
** 特别说明：本函数不可重入，若多个任务使用本函数，则必须在调用前申请信号量。并且，由于没有使用OSSchedLock(见函数UART0Send_L)，
**           也就没有解决"防止低优先级任务调用本函数过程中被长时间剥夺CPU使用权，而导致发出的数据出现较长时间的间断"
**           的问题
********************************************************************************************************/
void UART4Send(uint8* buf,uint32 n)
{
	OS_CPU_SR cpu_sr;
	uint32 N=3; //为了减少系统关中断时间,并减少开关中断次数，这里采用写N个字节关开一次中断
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=10时，测试得出关中断时间约为63us，
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=6时，测试得出关中断时间约为40us，(见文件末尾说明)
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=3时，测试得出关中断时间约为23us，
				//为了降低系统的中断延迟时间，原则上N 不应大于6。
	uint32 i,j,p,x,y;
	#ifdef   _MBUS_DETACH_
	uint8 err;
	#endif
	
	//======================等待发送队列可用空间>=n
	if(QueueNotEnoughPend((void*)UART4SendQueue,n)){
		return ;
	}
	//======================
	x=n/N;	y=n%N;	p=0;
	for(i=0;i<x;i++){
		OS_ENTER_CRITICAL();
		for(j=0;j<N;j++){
			__UART4Putch(buf[p++]);
		}
		OS_EXIT_CRITICAL();
	}

	OS_ENTER_CRITICAL();
	for(j=0;j<y;j++){
		__UART4Putch(buf[p++]);
	}
	OS_EXIT_CRITICAL();
#ifdef   _MBUS_DETACH_
    OSTimeDly(5);
    RS485_USART4_SEND_DIR(); 
	OSTimeDly(5);
#endif
	
	//以上均为 将需要发送的数据，写入队列，写入完成后开启发送缓冲寄存器为空中断，开始中断发送
	USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
#ifdef   _MBUS_DETACH_	
	OSSemPend(UART4SendCompleteSem, 0, &err); //等待接收数据
    OSTimeDly(5);
    RS485_USART4_RCV_DIR();
#endif
}

/*********************************************************************************************************
** 函数名称: UART0Putch
** 功能描述: 发送一个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: Uart0Send
********************************************************************************************************/
void UART4Putch(uint8 Data)
{
	UART4Send(&Data,1);
}

/*********************************************************************************************************
** 函数名称: UART4Getch
** 功能描述: 从UART4接收队列读一个字节，若队列内无数据则按OutTime做超时等待. 供上层调用
** 输　入: OutTime,等待超时时间
** 输　出: data 接收到的数据
** 全局变量: 无
** 调用模块:
** 特别说明： ！！！！！！！！！！此函数不可重入！！！！！！！！！！
********************************************************************************************************/

uint8 UART4Getch(uint8* data, uint16 OutTime)
{
	uint8 err;
	
	while (QueueRead(data,(void*)UART4RecQueue)!=QUEUE_OK)
	{
        OSSemPend(UART4RecQueSem, OutTime, &err); //等待接收数据
        FeedTaskDog();
    	if(err!=OS_ERR_NONE)
    		{
    			return err;
			}
	}
	return 0;
}

/*********************************************************************************************************
** 函数名称: TaskUART4Rec
** 功能描述: UART4接收任务，将收到的数据写入UART0接收队列，此任务应具有较高的优先级
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
********************************************************************************************************/
//消息队列方式
void TaskUART4Rec(void *pdata)
{
	uint8 err;
	uint8 *pReadData = NULL;	

	pdata = pdata;
	IRQBuf_Counter = 0x00;
	QueueFlush((void*)UART4RecQueue); //清空接收队列
	memset(IRQBuf, 0x00, 32);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	
	while(1)
	{
		do{
			FeedTaskDog();
			pReadData = (uint8 *)OSQPend(UART4QMsg, OS_TICKS_PER_SEC*6, &err);
		}while( (err != OS_ERR_NONE) || (pReadData == NULL) );
		
		QueueWrite((void*)UART4RecQueue, *pReadData); //将收到的数据写入队列
		OSSemPost(UART4RecQueSem);	
	}
}

/*********************************************************************************************************
** 函数名称: UART4_IRQHandler
** 功能描述: UART4接收中断
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
********************************************************************************************************/
void UART4_IRQHandler(void)
{
    CPU_SR		cpu_sr;
    uint8 Err = 0x00;
	uint8 tmp = 0x00;
	
	
	OS_ENTER_CRITICAL();
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	//有产生 Tansmit Data Register empty interrupt
	if(USART_GetITStatus(UART4, USART_IT_TXE) == SET)
		{
			Err = QueueRead(&tmp, UART4SendQueue);
			if (Err == QUEUE_OK)
				{
					UART4->DR = tmp;
				}
			else if(Err == QUEUE_EMPTY)
				{
					USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
					#ifdef   _MBUS_DETACH_
				    OSSemPost(UART4SendCompleteSem);
					#endif
				}
			
		}
	else if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
			{ //产生 Receive Data register not empty interrupt
				IRQBuf[IRQBuf_Counter] = UART4->DR;	
				OSQPost(UART4QMsg, &IRQBuf[IRQBuf_Counter]);
				if(IRQBuf_Counter>=30)	IRQBuf_Counter = 0;
				IRQBuf_Counter++;
				
	           // {
				//	IRQ4testBuf[IRQ4testBuf_Counter]=IRQBuf[IRQBuf_Counter];
				//	IRQ4testBuf_Counter ++;
				//	if(IRQ4testBuf_Counter>=30)	
         		//	{
         		//	IRQ4testBuf_Counter = 0;
         		//	}
				//}

				
			}
		else if( (USART_GetFlagStatus(UART4,USART_FLAG_ORE)==SET) || 
				 (USART_GetFlagStatus(UART4,USART_FLAG_NE)==SET) ||
				 (USART_GetFlagStatus(UART4,USART_FLAG_FE)==SET) ||
				 (USART_GetFlagStatus(UART4,USART_FLAG_PE)==SET) )
    			{//溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题
        			USART_ClearFlag(UART4, USART_FLAG_ORE);    	//读SR
        			tmp = UART4->DR;							//读DR
    			}
			else
				{
					//清除中断标志
					USART_ClearITPendingBit(UART4, USART_IT_TC);
				}
	OSIntExit();
}

