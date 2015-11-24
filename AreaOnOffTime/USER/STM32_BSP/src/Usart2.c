
/********************************************************************

********************************************************************/
#include <includes.h>


#define USART2_SEND_QUEUE_LENGTH			2072 			//给UART2发送数据队列分配的空间大小,单位：字节
#define USART2_REC_QUEUE_LENGTH			    2072 			//给UART2接收数据队列分配的空间大小,单位：字节

//注意：**QUEUE_LENGTH 包含队列指针及函数指针，实际上用来存数据的是**QUEUE_LENGTH-24 个字节
static uint8 USART2SendQueue[USART2_SEND_QUEUE_LENGTH];
	   uint8 USART2RecQueue[USART2_REC_QUEUE_LENGTH];
	   uint8 IRQ2testBuf[32] = {0x00};//muxiaoqing test
	   uint8 IRQ2testBuf_Counter = 0;

static uint8 IRQBuf[32] = {0x00};
static void *Q_IRQBuf[32];
static uint8 IRQBuf_Counter = 0;

static OS_EVENT *Usart2QMsg;									//中断到任务的消息邮箱
	   OS_EVENT *Usart2RecQueSem; 							//与接收队列绑定的信号量
	   OS_EVENT *Usart2SendQueSem;							//发送数据的信号量

uint8 USART2_QueueCreate(void)
{
	//创建发送队列
    if (QueueCreate((void *)USART2SendQueue,
                     sizeof(USART2SendQueue),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
	//创建接收队列
	if (QueueCreate((void *)USART2RecQueue,
                     sizeof(USART2RecQueue),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
    
    Usart2QMsg= OSQCreate(&Q_IRQBuf[0], 32);
	if (Usart2QMsg == NULL)
    {
        while (1);
    }
	//创建与接收队列绑定信号量
	Usart2RecQueSem = OSSemCreate(0);
    if (Usart2RecQueSem == NULL){
        while(1);
    }
    
    //创建与发送队列绑定信号量
	Usart2SendQueSem = OSSemCreate(0);
    if (Usart2SendQueSem == NULL){
        while(1);
    }
    
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
static void __USART2Putch(uint8 Data)
{
	QueueWrite_Int((void *)USART2SendQueue, Data); 					//数据入队列
	
	/*
	if(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == SET)			//UART0发送保持寄存器空
	{ 									
		QueueRead_Int(&tmp, UART0SendQueue); 						//发送最初入队的数据
		//说明，这里读队列没有判断返回值，是因为此时是关中断的，而上面刚写队列了。至于在开中断状态下可以不可以不判断返回值，没有仔细考虑
		USART2->DR = tmp;
	}*/
}

/*********************************************************************************************************
** 函数名称: USART2Send
** 功能描述: 发送多个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: __UART0Putch
** 特别说明：本函数不可重入，若多个任务使用本函数，则必须在调用前申请信号量。并且，由于没有使用OSSchedLock(见函数UART0Send_L)，
**           也就没有解决"防止低优先级任务调用本函数过程中被长时间剥夺CPU使用权，而导致发出的数据出现较长时间的间断"
**           的问题
********************************************************************************************************/
void USART2Send(uint8* buf,uint32 n)
{
	OS_CPU_SR cpu_sr;
	uint32 N=3; //为了减少系统关中断时间,并减少开关中断次数，这里采用写N个字节关开一次中断
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=10时，测试得出关中断时间约为63us，
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=6时，测试得出关中断时间约为40us，(见文件末尾说明)
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=3时，测试得出关中断时间约为23us，
				//为了降低系统的中断延迟时间，原则上N 不应大于6。
	uint32 i,j,p,x,y;
	
	//======================等待发送队列可用空间>=n
	if(QueueNotEnoughPend((void*)USART2SendQueue,n)){
		return ;
	}
	//======================
	x=n/N;	y=n%N;	p=0;
	for(i=0;i<x;i++){
		OS_ENTER_CRITICAL();
		for(j=0;j<N;j++){
			__USART2Putch(buf[p++]);
		}
		OS_EXIT_CRITICAL();
	}

	OS_ENTER_CRITICAL();
	for(j=0;j<y;j++){
		__USART2Putch(buf[p++]);
	}
	OS_EXIT_CRITICAL();
	
	//以上均为 将需要发送的数据，写入队列，写入完成后开启发送缓冲寄存器为空中断，开始中断发送
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	
}

/*********************************************************************************************************
** 函数名称: UART0Putch
** 功能描述: 发送一个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: Uart0Send
********************************************************************************************************/
void USART2Putch(uint8 Data)
{
	USART2Send(&Data,1);
}


/*********************************************************************************************************
** 函数名称: USART2Getch
** 功能描述: 从USART2接收队列读一个字节，若队列内无数据则按OutTime做超时等待. 供上层调用
** 输　入: OutTime,等待超时时间
** 输　出: data 接收到的数据
** 全局变量: 无
** 调用模块:
** 特别说明： ！！！！！！！！！！此函数不可重入！！！！！！！！！！
********************************************************************************************************/

uint8 USART2Getch(uint8* data, uint16 OutTime)
{
	uint8 err;
	
	while (QueueRead(data,(void*)USART2RecQueue)!=QUEUE_OK)
	{
        OSSemPend(Usart2RecQueSem, OutTime, &err); //等待接收数据
        FeedTaskDog();
    	if(err!=OS_ERR_NONE)
    		{
    			return err;
			}
	}
	return 0;
}

/*********************************************************************************************************
** 函数名称: TaskUsart2Rec
** 功能描述: UART2接收任务，将收到的数据写入UART0接收队列，此任务应具有较高的优先级
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
********************************************************************************************************/

//消息队列方式
void TaskUsart2Rec(void *pdata)
{
	uint8 err;
	uint8 *pReadData = NULL;	

	pdata = pdata;
	IRQBuf_Counter = 0x00;
	QueueFlush((void*)USART2RecQueue); //清空接收队列
	memset(IRQBuf, 0x00, 32);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	while(1)
	{
		do{
			FeedTaskDog();
			pReadData = (uint8 *)OSQPend(Usart2QMsg, OS_TICKS_PER_SEC*6, &err);
		}while( (err != OS_ERR_NONE) || (pReadData == NULL) );
		
		QueueWrite((void*)USART2RecQueue, *pReadData); //将收到的数据写入队列
		OSSemPost(Usart2RecQueSem);	
	}
}

/*********************************************************************************************************
** 函数名称: USART2_IRQHandler
** 功能描述: UART2接收中断
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
********************************************************************************************************/

void USART2_IRQHandler(void)
{
    CPU_SR		cpu_sr;
    uint8 Err = 0x00;
	uint8 tmp = 0x00;
	
	
	OS_ENTER_CRITICAL();
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	//有产生 Tansmit Data Register empty interrupt
	if(USART_GetITStatus(USART2, USART_IT_TXE) == SET)
		{
			Err = QueueRead(&tmp, USART2SendQueue);
			if (Err == QUEUE_OK)
				{
					USART2->DR = tmp;
				}
			else if(Err == QUEUE_EMPTY)
				{
					USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
				}
			
		}
	else if(USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
			{ //产生 Receive Data register not empty interrupt
				IRQBuf[IRQBuf_Counter] = USART2->DR;	
				OSQPost(Usart2QMsg, &IRQBuf[IRQBuf_Counter]);
				if(IRQBuf_Counter>=30)	IRQBuf_Counter = 0;
				IRQBuf_Counter++;
				
				//{
				//	IRQ2testBuf[IRQ2testBuf_Counter]=IRQBuf[IRQBuf_Counter];
				//	IRQ2testBuf_Counter ++;
				//	if(IRQ2testBuf_Counter>=18)	
         		//	{
         		//	IRQ2testBuf_Counter = 0;
         		//	}
				//}

			}
		else if( (USART_GetFlagStatus(USART2,USART_FLAG_ORE)==SET) || 
				 (USART_GetFlagStatus(USART2,USART_FLAG_NE)==SET) ||
				 (USART_GetFlagStatus(USART2,USART_FLAG_FE)==SET) ||
				 (USART_GetFlagStatus(USART2,USART_FLAG_PE)==SET) )
    			{//溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题
        			USART_ClearFlag(USART2, USART_FLAG_ORE);    //读SR
        			tmp = USART2->DR;							//读DR
    			}
			else
				{
					//清除中断标志
					USART_ClearITPendingBit(USART2, USART_IT_TC);
				}
	OSIntExit();
}

