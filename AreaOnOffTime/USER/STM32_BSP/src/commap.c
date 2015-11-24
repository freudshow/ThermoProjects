
#include <includes.h>
#include "publicfunc.h"

#define GPRS_REC_QUEUES_LENGTH				5000
//考虑IPDATA连续两次以上发过来,由于对列没有做满了等待而是丢掉,这里定义的比较大
#define		ZIGBEE_SEND_BYTES	      200

static uint8 UpRecQueue_Gprs[GPRS_REC_QUEUES_LENGTH];
static OS_EVENT *UpRecQueSem_Gprs;

static uint8 *pQueues[UP_COMMU_DEV_ARRAY];
static OS_EVENT *QueueSems[UP_COMMU_DEV_ARRAY];

uint8 UpcomInit(void)
{
	if (QueueCreate((void *)UpRecQueue_Gprs,
                     sizeof(UpRecQueue_Gprs),
                     NULL,NULL) == NOT_OK){
		while(1);
	}
	
	UpRecQueSem_Gprs = OSSemCreate(0);
	if (UpRecQueSem_Gprs == NULL){
		while(1);
	}
	
	pQueues[UP_COMMU_DEV_ZIGBEE] 		= UART4RecQueue;
	QueueSems[UP_COMMU_DEV_ZIGBEE] 		= UART4RecQueSem;
	
	pQueues[UP_COMMU_DEV_485] 			= UART5RecQueue;
	QueueSems[UP_COMMU_DEV_485] 		= UART5RecQueSem;
	
	pQueues[UP_COMMU_DEV_232] 			= USART1RecQueue;
	QueueSems[UP_COMMU_DEV_232] 		= Usart1RecQueSem;
	
	pQueues[UP_COMMU_DEV_GPRS] 			= UpRecQueue_Gprs;
	pQueues[UP_COMMU_DEV_AT]			= USART3RecQueue_At;
	pQueues[UP_COMMU_DEV_ATIPD]			= USART3RecQueue_AtIPD;
	QueueSems[UP_COMMU_DEV_GPRS] 		= UpRecQueSem_Gprs;
	QueueSems[UP_COMMU_DEV_AT]			= USART3RecQueSem_At;
	QueueSems[UP_COMMU_DEV_ATIPD]		= USART3RecQueSem_AtIPD;
	
    return 0;
}

uint8 UpGetStart(uint8 dev, uint16 OutTime)
{
	uint8 err 			= 0x00;
	uint8 DataTemp 		= 0x00;
	uint8 Flag			= 0x01;
	
	while(Flag)
	{
		while(QueueRead(&DataTemp, (void*)pQueues[dev]) != QUEUE_OK)
		{
			FeedTaskDog();	
			OSSemPend(QueueSems[dev], OutTime, &err);
		}
		if(err!=OS_ERR_NONE)
			{
				return err;
			}
       // debug("485:%x \r\n",DataTemp);/*TODO:need to be deleted */
		if(DataTemp == 0xFB)
			{//若是前导符，则继续查找
				Flag++;
			}
		if((DataTemp!=0xFB) && (DataTemp!=0x7B))	Flag = 1;

		if(Flag >= 3)
			{
				if(DataTemp == 0x7B)	
					{//找到开始符
						Flag = 0;
					}
			}
	}
	return 0;
}

uint8 UpGetch(uint8 dev, uint8* data, uint16 OutTime)
{
	uint8 err = 0x00;
	
	while(QueueRead(data, (void*)pQueues[dev]) != QUEUE_OK)
	{	
		OSSemPend(QueueSems[dev], OutTime, &err);
		FeedTaskDog();
		if(err!=OS_ERR_NONE)
		{
			return err;
		}
	}
	
	
	return 0;
}

uint8 UpQueueFlush(uint8 dev)
{
	QueueFlush((void*)pQueues[dev]);
    return 0;
}
/*
read n betys buf to dev
*/
//写上行接收队列,说明: 不是所有的操作都调用此函数进行,比如UART3接收队列的写操作没有调用本函数
uint8 UpRecQueueWrite(uint8 dev,uint8* buf,uint32 n) //不可重入
{
	uint32 i;
	
	for(i=0;i<n;i++){ 
	  	QueueWrite((void*)pQueues[dev],buf[i]);
	}
	OSSemPost(QueueSems[dev]);
	return 0;
}

void UpDevSend(uint8 dev,uint8 *Data, uint32 n) 		//不可重入
{
	uint32 i, CirTimes, Residual;
	
	switch (dev)
	{
		case UP_COMMU_DEV_ZIGBEE:						//针对于Zigbee发送数据，每次100个字节，每次发送间隔333MS
		/*beign:zhangtao modified 2013-09-18 for 发送太快，掌机接收不到数据，加延时*/
             //OSTimeDly(OS_TICKS_PER_SEC/2);/*是由于掌机没有立即收模式导致的，掌机应该去改下*/
             /*end:zhangtao modified 2013-09-18 */
			CirTimes 	= n/ZIGBEE_SEND_BYTES;
			Residual 	= n%ZIGBEE_SEND_BYTES;
			
			for(i=0; i<CirTimes; i++)
			{
				UART4Send(Data,ZIGBEE_SEND_BYTES);
				Data += ZIGBEE_SEND_BYTES;
                           /*beign:yangfei modified 2013-03-28 */
				#if 0
                             OSTimeDly(OS_TICKS_PER_SEC/3);
		             #else
		               OSTimeDly(OS_TICKS_PER_SEC/100);
		              #endif
		              /*end:yangfei modified 2013-03-28 */
				FeedTaskDog();
			}
			
			UART4Send(Data,	Residual);
			
			/*beign:yangfei modified 2013-03-28 */
			#if 0
                      OSTimeDly(OS_TICKS_PER_SEC/3);
                     #else
                      OSTimeDly(OS_TICKS_PER_SEC/100);
                     #endif
                     /*end:yangfei modified 2013-03-28 */
			break;
		case UP_COMMU_DEV_485:
		       /*beign:zhangtao added 2013-09-26 for 发送太快，掌机接收不到数据，加延时*/
                    OSTimeDly(50);
                     /*end:zhangtao added 2013-09-18 */
			UART5Send(Data,n);
			break;
		
		case UP_COMMU_DEV_232:
			USART1Send(Data,n);
			break;
			
		case UP_COMMU_DEV_GPRS:
			USART3Send(Data,n);
			break;
		
		
		default :
			while(1);
	}
}

uint8 DuQueueFlush(uint8 dev)
{
	uint8 tmp = NO_ERR;
	switch (dev)
	{
		case DOWN_COMM_DEV_MBUS:
			QueueFlush((void*)USART2RecQueue);			//MBUS接收队列清空
			break;
       /*begin:yangfei added 2013-03-25 for support 485 meter*/
      case DOWN_COMM_DEV_485:
			QueueFlush((void*)UART5RecQueue);			//485接收队列清空
			break;
       /*end:yangfei added 2013-03-25 */
		default :
			{
				tmp = ERR_1;
				debug("ERROR:%s %d default dev = %d\r\n",__FUNCTION__,__LINE__,dev);
				break;
			}
			
	}
	return tmp;
}

uint8 DuGetch(uint8 dev,uint8* data,uint16 OutTime)
{
	uint8 tmp;
	switch (dev)
	{
		case DOWN_COMM_DEV_MBUS:
			tmp=USART2Getch(data, OutTime); 				//MBUS抄表
			break;
       /*begin:yangfei added 2013-03-25 for support 485 meter*/
      	case DOWN_COMM_DEV_485:
			tmp=UART5Getch(data, OutTime); 				//485抄表
			break;
      /*end:yangfei added 2013-03-25 */
		default :
			while(1);
	}
	return tmp;
}

/*
void DuSend(uint8 dev,uint8* buf,uint32 n)
{
	switch (dev)
	{
		case DOWN_COMM_DEV_MBUS:
			USART2Send(buf, n); 							//MBUS抄表---发送数据
			break;
      //begin:yangfei added 2013-03-25 for support 485 meter
      	case DOWN_COMM_DEV_485:
			UART5Send(buf, n); 					//485抄表
			break;
      //end:yangfei added 2013-03-25
		default :
			while(1);
	}
}
 */

void DuSend(uint8 dev,uint8* buf,uint32 n)
{
	switch (dev)
	{
		case DOWN_COMM_DEV_MBUS:
			USART2Send(buf, n); 							//MBUS抄表---发送数据
			break;
      /*begin:yangfei added 2013-03-25 for support 485 meter*/
      	case DOWN_COMM_DEV_485:
			UART5Send(buf, n); 					//485抄表
			break;
      /*end:yangfei added 2013-03-25 */
		default :
			while(1);
	}
}

/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
