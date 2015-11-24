
/********************************************************************

********************************************************************/
#include <includes.h>


#define USART1_SEND_QUEUE_LENGTH			2072 			//给UART1发送数据队列分配的空间大小,单位：字节
#define USART1_REC_QUEUE_LENGTH			    2072 			//给UART1接收数据队列分配的空间大小,单位：字节

//注意：**QUEUE_LENGTH 包含队列指针及函数指针，实际上用来存数据的是**QUEUE_LENGTH-24 个字节
static uint8 USART1SendQueue[USART1_SEND_QUEUE_LENGTH];
	   uint8 USART1RecQueue[USART1_REC_QUEUE_LENGTH];
	   uint8 IRQ1testBuf[32] = {0x00};//muxiaoqing test
	   uint8 IRQ1testBuf_Counter = 0;

static uint8 IRQBuf[32] = {0x00};
static void *Q_IRQBuf[32];
static uint8 IRQBuf_Counter = 0;
uint8 forbidsending1 = 0;//muxiaoqing test
static OS_EVENT *Usart1QMsg;								//中断到任务的消息邮箱
	   OS_EVENT *Usart1RecQueSem; 							//与接收队列绑定的信号量
	   OS_EVENT *Usart1SendQueSem;							//发送数据的信号量
//static uint8 testttttt=0;
extern uint8 autotestmode ;//muxiaoqing test
extern uint8 manualtestmode;

extern OS_EVENT *UartMsgOSQ;	/*按键消息队列*/

uint8 USART1_QueueCreate(void)
{
	//创建发送队列
    if (QueueCreate((void *)USART1SendQueue,
                     sizeof(USART1SendQueue),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
	//创建接收队列
	if (QueueCreate((void *)USART1RecQueue,
                     sizeof(USART1RecQueue),
                     NULL,
                     NULL)
                     == NOT_OK){
		while(1);
    }
	Usart1QMsg= OSQCreate(&Q_IRQBuf[0], 32);
	if (Usart1QMsg == NULL)
    {
        while (1);
    }
	//创建与接收队列绑定信号量
	Usart1RecQueSem = OSSemCreate(0);
    if (Usart1RecQueSem == NULL){
        while(1);
    }
    
    //创建与发送队列绑定信号量
	Usart1SendQueSem = OSSemCreate(0);
    if (Usart1SendQueSem == NULL){
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
static void __USART1Putch(uint8 Data)
{
	QueueWrite_Int((void *)USART1SendQueue, Data); 					//数据入队列
	
	/*
	if(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET)			//UART0发送保持寄存器空
	{ 									
		QueueRead_Int(&tmp, UART0SendQueue); 						//发送最初入队的数据
		//说明，这里读队列没有判断返回值，是因为此时是关中断的，而上面刚写队列了。至于在开中断状态下可以不可以不判断返回值，没有仔细考虑
		USART1->DR = tmp;
	}*/
}

/*********************************************************************************************************
** 函数名称: USART1Send
** 功能描述: 发送多个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: __UART0Putch
** 特别说明：本函数不可重入，若多个任务使用本函数，则必须在调用前申请信号量。并且，由于没有使用OSSchedLock(见函数UART0Send_L)，
**           也就没有解决"防止低优先级任务调用本函数过程中被长时间剥夺CPU使用权，而导致发出的数据出现较长时间的间断"
**           的问题
********************************************************************************************************/
void USART1Send(uint8* buf,uint32 n)
{
	OS_CPU_SR cpu_sr;
	uint32 N=3; //为了减少系统关中断时间,并减少开关中断次数，这里采用写N个字节关开一次中断
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=10时，测试得出关中断时间约为63us，
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=6时，测试得出关中断时间约为40us，(见文件末尾说明)
				//FCCLK=57.6MHZ，FPCLK=14.4MHZ时 ，N=3时，测试得出关中断时间约为23us，
				//为了降低系统的中断延迟时间，原则上N 不应大于6。
	uint32 i,j,p,x,y;
	
				
					if(!autotestmode)
						forbidsending1 = 0;
				if(forbidsending1)
				{
				}
				else
					{
	//======================等待发送队列可用空间>=n
	if(QueueNotEnoughPend((void*)USART1SendQueue,n)){
		return ;
	}
	//======================
	x=n/N;	y=n%N;	p=0;
	for(i=0;i<x;i++){
		OS_ENTER_CRITICAL();
		for(j=0;j<N;j++){
			__USART1Putch(buf[p++]);
		}
		OS_EXIT_CRITICAL();
	}

	OS_ENTER_CRITICAL();
	for(j=0;j<y;j++){
		__USART1Putch(buf[p++]);
	}
	OS_EXIT_CRITICAL();
	
	//以上均为 将需要发送的数据，写入队列，写入完成后开启发送缓冲寄存器为空中断，开始中断发送
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
					}
}

/*********************************************************************************************************
** 函数名称: UART0Putch
** 功能描述: 发送一个字节数据
** 输　入: Data：发送的数据数据
** 输　出:无
** 全局变量: 无
** 调用模块: Uart0Send
********************************************************************************************************/
void USART1Putch(uint8 Data)
{
	USART1Send(&Data,1);
}

/*********************************************************************************************************
** 函数名称: USART1Getch
** 功能描述: 从USART1接收队列读一个字节，若队列内无数据则按OutTime做超时等待. 供上层调用
** 输　入: OutTime,等待超时时间
** 输　出: data 接收到的数据
** 全局变量: 无
** 调用模块:
** 特别说明： ！！！！！！！！！！此函数不可重入！！！！！！！！！！
********************************************************************************************************/

uint8 USART1Getch(uint8* data, uint16 OutTime)
{
	uint8 err;
	
	while (QueueRead(data,(void*)USART1RecQueue)!=QUEUE_OK)
	{
        OSSemPend(Usart1RecQueSem, OutTime, &err); //等待接收数据
        FeedTaskDog();
    	if(err!=OS_ERR_NONE)
    		{
    			return err;
			}
	}
	return 0;
}

/*********************************************************************************************************
** 函数名称: TaskUsart1Rec
** 功能描述: UART1接收任务，将收到的数据写入UART0接收队列，此任务应具有较高的优先级
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
********************************************************************************************************/

void TaskUsart1Rec(void *pdata)
{
	uint8 err;
	uint8 *pReadData = NULL;
	
	pdata = pdata;
	IRQBuf_Counter = 0x00;
	QueueFlush((void*)USART1RecQueue); //清空接收队列
	memset(IRQBuf, 0x00, 32);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	while(1)
	{
		do{
			FeedTaskDog();
			//pReadData = (uint8 *)OSMboxPend(Usart1Mbox, OS_TICKS_PER_SEC*6, &err);
			pReadData = (uint8 *)OSQPend(Usart1QMsg, OS_TICKS_PER_SEC*6, &err);
		}while( (err != OS_ERR_NONE) || (pReadData == NULL) );
		
		QueueWrite((void*)USART1RecQueue, *pReadData); //将收到的数据写入队列
		OSSemPost(Usart1RecQueSem);
		
		//OSTimeDlyHMSM(0, 0, 1, 0);	
	}
}

/****************************************************************************
** 函数名称: USART1_IRQHandler
** 功能描述: UART1接收中断
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
*****************************************************************************/
void USART1_IRQHandler(void)
{
    CPU_SR		cpu_sr;
    uint8 Err = 0x00;
	uint8 tmp = 0x00;
	
	
	OS_ENTER_CRITICAL();
	OSIntNesting++;
	OS_EXIT_CRITICAL();
	
	//有产生 Tansmit Data Register empty interrupt
	if(USART_GetITStatus(USART1, USART_IT_TXE) == SET)
		{
			Err = QueueRead(&tmp, USART1SendQueue);
			if (Err == QUEUE_OK)
				{
					USART1->DR = tmp;
				}
			else if(Err == QUEUE_EMPTY)
				{
					USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
					if(autotestmode)
						{
                          forbidsending1 = 1;
					    }
				}
			
		}
	else if(USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
			{ //产生 Receive Data register not empty interrupt
				IRQBuf[IRQBuf_Counter] = USART1->DR;
				OSQPost(Usart1QMsg, &IRQBuf[IRQBuf_Counter]);
				if(IRQBuf_Counter>=30)	IRQBuf_Counter = 0;
				IRQBuf_Counter++;
				
                //#ifdef DEBUG_SHELL
               // #if 0
               // Uart_ReceiveChar(); 
               // #else	
	           // {
				//	IRQ1testBuf[IRQ1testBuf_Counter]=IRQBuf[IRQBuf_Counter];
				//	IRQ1testBuf_Counter ++;
				//	if(IRQ1testBuf_Counter>=19)	
         		//	{
         		//	IRQ1testBuf_Counter = 0;
         		//	}
				//}		

               	//#endif
			}
		else if( (USART_GetFlagStatus(USART1,USART_FLAG_ORE)==SET) || 
				 (USART_GetFlagStatus(USART1,USART_FLAG_NE)==SET) ||
				 (USART_GetFlagStatus(USART1,USART_FLAG_FE)==SET) ||
				 (USART_GetFlagStatus(USART1,USART_FLAG_PE)==SET) )
    			{//溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题
        			USART_ClearFlag(USART1, USART_FLAG_ORE);    //读SR
        			tmp = USART1->DR;							//读DR
    			}
			else
				{
					//清除中断标志
					USART_ClearITPendingBit(USART1, USART_IT_TC);
				}
	OSIntExit();
}


//if you don't use vsprintf(), the code size is reduced very much.
void Uart_Printf(char *fmt,...)
{
	va_list ap;
	char string[256] = {0};
  
	va_start(ap,fmt);
	vsprintf(string,fmt,ap);
    USART1Send((uint8*) string,sizeof(string));
	//Uart_SendString(string);
	va_end(ap);
}
/****************************************************************************
** 函数名称: Uart_Printf_Time
** 功能描述: 带时间的打印函数
** 输　入:
** 输　出:
** 全局变量:
** 调用模块:
*****************************************************************************/
void Uart_Printf_Time(char *fmt,...)
{
    char StringBuf[356] 	   	= {0x00};
	uint8 lTimeDate[6]			= {0x00};
	int16 StringLen				= 0x00;
	va_list argptr;
	
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
	
	va_start(argptr, fmt);
	StringLen = vsnprintf(&StringBuf[23], sizeof(StringBuf), fmt, argptr);
	if(StringLen)			;						//此处需对复制的长度进行判断
	USART1Send((uint8*) StringBuf,sizeof(StringBuf));
	va_end(argptr);
}

void Uart_ReceiveChar (void) 
{
	u32 RecChar;
	RecChar = (u32)(USART1->DR & 0xFF);
	OSQPost ( UartMsgOSQ, ( void* )RecChar );  //将字符指针化放入消息队列
}

u8 Uart_GetChar(void)
{
	u8 Err;
	u8 TempChar;
	TempChar =(u8) ( (u32) (u32*) OSQPend ( UartMsgOSQ, 0, &Err )  &0xFF );
	return TempChar;
}



