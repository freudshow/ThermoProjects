/*******************************************Copyright (c)*******************************************
         							山东华宇空间技术公司(西安分部)                                                 
**  文   件   名: tasks_up.cpp
**  创   建   人: 勾江涛
**  创 建  日 期: 2012.08.07 
**  描        述: 与主站的通信任务
**  修 改  记 录:   	
*****************************************************************************************************/

/********************************************** include *********************************************/
#include <includes.h>
#include "tasks_up.h"
#include "app_up.h"

/********************************************** extern *********************************************/
extern OS_EVENT* GprsManaMbox;
extern GPRS_RUN_STA gGprsRunSta;
extern OS_EVENT* HeartFrmMbox;


/********************************************** global *********************************************/
OS_MEM* CommTxBuffer;
uint32 CommTxPart[BLK_NUM][RECE_BUF_LEN];
OS_EVENT *CommTxPartSem;						//与内存分配绑定的信号量

void* MsgGrp[BLK_NUM];               			//定义消息指针数组
OS_EVENT *Str_Q;

void* UpSend_Q_Grp[BLK_NUM];
OS_EVENT *UpSend_Q;

uint32 num1,num2;

/********************************************** static *********************************************/
static uint8 gCommDeviceZigbee;
static uint8 gCommDeviceRS485;
static uint8 gCommDeviceRS232;
static uint8 gCommDeviceGprs;

static OS_STK    TaskUpSendStk[TASK_STK_SIZE];
static OS_STK    TaskUpAnalFrStk[TASK_STK_SIZE*2];
static OS_STK    TaskUpRecZigbeeStk[TASK_STK_SIZE];
static OS_STK    TaskUpRecRS485Stk[TASK_STK_SIZE];
static OS_STK    TaskUpRecRS232Stk[TASK_STK_SIZE];
static OS_STK    TaskUpRecGprsStk[TASK_STK_SIZE];
static OS_STK	 TaskReportUpStk[TASK_STK_SIZE];

//static uint8 testHeartFrm_xmz[]={"您好!这里是CZ513-6型集中器自检程序!GPRS数据通道已经成功建立!呵呵.请试着向集中器发送一些文字或数据.看看自检程序界面上是否收到.---您好!这里是晨泰集中器自检程序!GPRS数据通道已经成功建立!呵呵.请试着向集中器发送一些文字或数据.看看自检程序界面上是否收到.---您好!这里是晨泰集中器自检程序!GPRS数据通道已经成功建立!呵呵.请试着向集中器发送一些文字或数据.看看自检程序界面上是否收到.---您好!这里是晨泰集中器自检程序!GPRS数据通道已经成功建立!呵呵.请试着向集中器发送一些文字或数据.看看自检程序界面上是否收到."};

/****************************************************************************************************
**	任 务  名 称: TaskUpSend
**	任 务  功 能: 向上位机发送数据，发送任何数据，均需通过该任务来管理
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
void  TaskUpSend(void *pdata)
{
	uint8 Err 		= 0x00;
	uint8 csq;  
    uint16 BufNum = 0;
    
	UP_COMM_MSG	*pUpCommMsg	= NULL;
	GPRS_RUN_STA GprsRunSta;
	
    pdata = pdata;                            /* 避免编译警告*/
    
    while(1)
    {	
       	do 
      	{	
			FeedTaskDog();
       		pUpCommMsg = (UP_COMM_MSG*)OSQPend(UpSend_Q, OS_TICKS_PER_SEC*6, &Err);
      	}
      	while ((Err!= OS_ERR_NONE)||(pUpCommMsg==NULL));
        LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] Recived a UpSend_Q, Ready to UpSend Data !");
        
        if(pUpCommMsg->GprsGvc == TRUE)
        {
      		ReadGprsRunSta(&GprsRunSta);
            if(GprsRunSta.Ready == TRUE)
            	{
            		if(CMD_IpClose() == FALSE)
            			{
            				UpdGprsRunSta_IpCloseNum(0);
							LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] UpdGprsRunSta_IpCloseNum(0)!");
							debug_info(gDebugModule[GPRS_MODULE],"UpdGprsRunSta_IpCloseNum(0)!");
            				num1++;
            			}	
            		else
            			{
							/*begin:yangfei needed modified 2013-02-27*/
							#if 1
							UpdGprsRunSta_AddIpCloseNum();
							#endif
							/*end:yangfei needed modified 2013-02-27*/
							LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] UpdGprsRunSta_AddIpCloseNum()!");
							debug_err(gDebugModule[GPRS_MODULE],"UpdGprsRunSta_AddIpCloseNum()");
            				num2++;
            			}
            	
            		OSTimeDly(OS_TICKS_PER_SEC/20);
            		if(CMD_Csq(&csq)==0)
            			{
            				UpdGprsRunSta_Csq(csq);
            			}
            	
            	}
            
            OSMboxPost(GprsManaMbox,(void*)1);
            OSTimeDly(OS_TICKS_PER_SEC/20);
      		continue;//保证后面不作内存释放，因为这里使用的不是动态分配的内存
      	}
      	else
      		{ 	
      			switch (pUpCommMsg->Device)
    			{
        		    case UP_COMMU_DEV_GPRS:
        		    	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] Send Data By GPRS!");
        		    	ReadGprsRunSta(&GprsRunSta);
        		    	if(GprsRunSta.Ready==FALSE){
        		    		break;
        		    	}
        		    	Err=GprsIPSEND(pUpCommMsg->buff,pUpCommMsg->ByteNum,&BufNum);
        		    	if(Err==0){
        		    		UpdGprsRunSta_IpSendRemnBufNum((uint8)BufNum);
        		    		UpdGprsRunSta_FeedSndDog();	
        		    		UpdGprsRunSta_AddFrmSndTimes(pUpCommMsg->ByteNum);
							LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] IP Send Successful!");
        		    	}
        		    	else{
							LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: [TaskUpSend] IP Send Failure!");
        		    		UpdGprsRunSta_AddIpSendFailNum();
        		    	}  	
       					break;
       					
					case UP_COMMU_DEV_ZIGBEE:
						
            			UpDevSend(UP_COMMU_DEV_ZIGBEE, pUpCommMsg->buff, pUpCommMsg->ByteNum);
            			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] Send Data By Zigbee!");
            			break;
            			
            		case UP_COMMU_DEV_485:
            			
            			UpDevSend(UP_COMMU_DEV_485, pUpCommMsg->buff, pUpCommMsg->ByteNum);
            			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] Send Data By RS485!");
            			break;
            		case UP_COMMU_DEV_232:
            			UpDevSend(UP_COMMU_DEV_232, pUpCommMsg->buff, pUpCommMsg->ByteNum);
            			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] Send Data By RS232!");
            			break;	
        		    default :
        		    	break;
        		    	
   				}
     		}
   		
      	OSTimeDly(OS_TICKS_PER_SEC/100);
      	
       	OSMemPut(CommTxBuffer,(void*)(pUpCommMsg));
      	OSSemPost(CommTxPartSem);
      	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] Release OSMem!");
      	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpSend] Waiting for UpSend_Q!");
    } 
}

/****************************************************************************************************
**	任 务  名 称: TaskUpAnalFr
**	任 务  功 能: 接收到上位机的数据后，对数据帧的解析; 发送数据前对数据帧的组帧，均由该任务来管理
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
void  TaskUpAnalFr(void *pdata)
{
 	uint8     err = 0; 
 	uint16 lenBuf = 0; 
   	
	UP_COMM_MSG 	*pUpCommMsg;
	BaseProtocol 	*pBase;
	//int i;
  	_Protocol Pro_Object_Anal;
  	
  	pBase = (BaseProtocol *)&Pro_Object_Anal;
  	pdata = pdata;                            // 避免编译警告
        
    while(1)
    {	
       	do 
      	{	
      		FeedTaskDog();  
       		pUpCommMsg = (UP_COMM_MSG*)OSQPend(Str_Q, OS_TICKS_PER_SEC*6, &err); 
      	}
      	while((err!= OS_ERR_NONE)||(pUpCommMsg==NULL));
      	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpAnalFr] Recived a Str_Q, Ready to AnalysisFrame !");
      	
        err = pBase->AnalysisFrame(pUpCommMsg->buff, lenBuf);
		/*begin:yangfei added 2013-02-18 */
		//debugX(LOG_LEVEL_INFO,"Recvie %s \r\n",pUpCommMsg->buff);
		/*end:yangfei added 2013-02-18*/
 		if (err==0)
 		{
    	 	pUpCommMsg->ByteNum=lenBuf;
    	 	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpAnalFr] AnalysisFrame Successful, Ready to OSQPost UpSend_Q!");
   		 	OSQPost(UpSend_Q,(void*)(pUpCommMsg));
 			
 		}
 		else
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpAnalFr] The Return Err is %d, Release OSMem!", err);
			OSMemPut(CommTxBuffer,(void*)(pUpCommMsg));
  			OSSemPost(CommTxPartSem);
  		}
      	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpAnalFr] Waiting for Str_Q!");
	} 
}

/****************************************************************************************************
**	任 务  名 称: TaskUpRecZigbee
**	任 务  功 能: 负责接收Zigbee的数据，数据效验正确, 之后交由分析帧任务分析执行
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
void  TaskUpRecZigbee(void *pdata)
{
 	
 	uint8 err,temp;
    uint16 n,len;
	uint8 CommDevice;
	
	_Protocol 		Pro_Object_Rec;
	UP_COMM_MSG 	*pUpCommMsg;
	BaseProtocol 	*pBase;
  	
  	CommDevice = *((uint8*)pdata);
  	pBase = (BaseProtocol *)&Pro_Object_Rec;

 	while(1)
    {
		do 
        {	
        	//这里不喂狗,因为正常情况下一定能申请到(在保证足够多的内存块的情况下,即BLK_NUM要足够大),否则说明异常
        	OSSemPend(CommTxPartSem, 0, &err);
       	}while (err != OS_ERR_NONE);
       	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecZigbee] Recived a CommTxPartSem, Ready to OSMemGet !");
       	
		pUpCommMsg=(UP_COMM_MSG*)OSMemGet(CommTxBuffer, &err);
		if(err != OS_ERR_NONE)
		{
			OSSemPost(CommTxPartSem);
			LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: [TaskUpRecZigbee] OSMemGet Failure!");
			continue;
		}
		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecZigbee] OSMemGet Successful!");
			
loop:	len = pBase->minFrameLen;
		do 
        {
          	FeedTaskDog();
            err=UpGetStart(CommDevice, OS_TICKS_PER_SEC*6);   
    	}while ( (err!=0) );
    	
    	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecZigbee] OSMemGet Successful!");	
		
		n=0;
		pUpCommMsg->buff[n++]=0x7B;
		for(n=n; n<len; n++)
       	{
      	    err=UpGetch(CommDevice, &temp, OS_TICKS_PER_SEC);
         	if(err)
         	{
         		goto loop;
         	}
       		pUpCommMsg->buff[n]=temp;
		
       	}
		err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
		//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecZigbee] ReceiveFrame First, Return Value is %d", err);
		if (err == 4) 
       	{	
       		
       	   	if (len>(RECE_BUF_LEN*4)) 
       	   	{
       	     	goto loop;
       	   	}
       	   	//
       	   	for(n=n;n<len;n++)
       	   	{
     	    	err=UpGetch(CommDevice,&temp,OS_TICKS_PER_SEC);
         		if(err)
         		{
         			goto loop;
         		}
         		pUpCommMsg->buff[n]=temp;
       		}
       	   	err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
       	}
		if(err)
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: [TaskUpRecZigbee] ReceiveFrame Failure, Return Value is %d", err);
			goto loop;
		}
		else
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecZigbee] ReceiveFrame Successful, Ready to OSQPost Str_Q");
			pUpCommMsg->GprsGvc = FALSE;
        	pUpCommMsg->Device = CommDevice;
        	OSQPost(Str_Q,(void*)(pUpCommMsg));
        }
        LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecZigbee] Waiting Recive Frame Data!");
	}
} 

/****************************************************************************************************
**	任 务  名 称: TaskUpRecRS485
**	任 务  功 能: 负责接收RS485的数据，数据效验正确, 之后交由分析帧任务分析执行
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
void  TaskUpRecRS485(void *pdata)
{
 	
 	uint8 err,temp;
    uint16 n,len;
	uint8 CommDevice;
	
	_Protocol 		Pro_Object_Rec;
	UP_COMM_MSG 	*pUpCommMsg;
	BaseProtocol 	*pBase;
  	
  	CommDevice = *((uint8*)pdata);
  	pBase = (BaseProtocol *)&Pro_Object_Rec;

 	while(1)
    {
		do 
        {	
        	//这里不喂狗,因为正常情况下一定能申请到(在保证足够多的内存块的情况下,即BLK_NUM要足够大),否则说明异常
        	OSSemPend(CommTxPartSem, 0, &err);
       	}while (err != OS_ERR_NONE);
       	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS485] Recived a CommTxPartSem, Ready to OSMemGet !");
       	
		pUpCommMsg=(UP_COMM_MSG*)OSMemGet(CommTxBuffer, &err);
		if(err != OS_ERR_NONE)
		{
			OSSemPost(CommTxPartSem);
			LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: [TaskUpRecRS485] OSMemGet Failure!");
			continue;
		}
		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS485] OSMemGet Successful!");
			
loop:	len = pBase->minFrameLen;
		do 
        {
          	FeedTaskDog();
            err=UpGetStart(CommDevice, OS_TICKS_PER_SEC*6);   
    	}while ( (err!=0) );
    	
    	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS485] OSMemGet Successful!");	
		
		n=0;
		pUpCommMsg->buff[n++]=0x7B;
		for(n=n; n<len; n++)
       	{
      	    err=UpGetch(CommDevice, &temp, OS_TICKS_PER_SEC);
         	if(err)
         	{
         		goto loop;
         	}
       		pUpCommMsg->buff[n]=temp;
		
       	}
		err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
		//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS485] ReceiveFrame First, Return Value is %d", err);
		if (err == 4) 
       	{	
       		
       	   	if (len>(RECE_BUF_LEN*4)) 
       	   	{
       	     	goto loop;
       	   	}
       	   	//
       	   	for(n=n;n<len;n++)
       	   	{
     	    	err=UpGetch(CommDevice,&temp,OS_TICKS_PER_SEC);
         		if(err)
         		{
         			goto loop;
         		}
         		pUpCommMsg->buff[n]=temp;
       		}
       	   	err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
       	}
		if(err)
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "ERROR: [TaskUpRecRS485] ReceiveFrame Failure, Return Value is %d", err);
			goto loop;
		}
		else
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS485] ReceiveFrame Successful, Ready to OSQPost Str_Q");
			pUpCommMsg->GprsGvc = FALSE;
        	pUpCommMsg->Device = CommDevice;
			//
        	OSQPost(Str_Q,(void*)(pUpCommMsg));
        }
        LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS485] Waiting Recive Frame Data!");
	}
}

/****************************************************************************************************
**	任 务  名 称: TaskUpRecRS232
**	任 务  功 能: 负责接收RS232的数据，数据效验正确, 之后交由分析帧任务分析执行
**	输 入  参 数: 无
**	输 出  参 数: 无 
**    返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
void  TaskUpRecRS232(void *pdata)
{
 	
 	uint8 err,temp;
    uint16 n,len;
	uint8 CommDevice;
	
	_Protocol 		Pro_Object_Rec;
	UP_COMM_MSG 	*pUpCommMsg;
	BaseProtocol 	*pBase;
  	
  	CommDevice = *((uint8*)pdata);
  	pBase = (BaseProtocol *)&Pro_Object_Rec;

 	while(1)
    {
		do 
        {	
        	//这里不喂狗,因为正常情况下一定能申请到(在保证足够多的内存块的情况下,即BLK_NUM要足够大),否则说明异常
        	OSSemPend(CommTxPartSem, 0, &err);
       	}while (err != OS_ERR_NONE);
       	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS232] Recived a CommTxPartSem, Ready to OSMemGet !");
       	
		pUpCommMsg=(UP_COMM_MSG*)OSMemGet(CommTxBuffer, &err);
		if(err != OS_ERR_NONE)
		{
			OSSemPost(CommTxPartSem);
			LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: [TaskUpRecRS232] OSMemGet Failure!");
			continue;
		}
		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS232] OSMemGet Successful!");
			
loop:	len = pBase->minFrameLen;
		do 
        {
          	FeedTaskDog();
            err=UpGetStart(CommDevice, OS_TICKS_PER_SEC*6);   
    	}while ( (err!=0) );
    	
    	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS232] OSMemGet Successful!");	
		
		n=0;
		pUpCommMsg->buff[n++]=0x7B;
		for(n=n; n<len; n++)
       	{
      	    err=UpGetch(CommDevice, &temp, OS_TICKS_PER_SEC);
         	if(err)
         	{
         		goto loop;
         	}
       		pUpCommMsg->buff[n]=temp;
		
       	}
		err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS232] ReceiveFrame First, Return Value is %d", err);
		if (err == 4) 
       	{	
       		
       	   	if (len>(RECE_BUF_LEN*4)) 
       	   	{
       	     	goto loop;
       	   	}
       	   	//
       	   	for(n=n;n<len;n++)
       	   	{
     	    	err=UpGetch(CommDevice,&temp,OS_TICKS_PER_SEC);
         		if(err)
         		{
         			goto loop;
         		}
         		pUpCommMsg->buff[n]=temp;
       		}
       	   	err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
       	}
		if(err)
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: [TaskUpRecRS232] ReceiveFrame Failure, Return Value is %d", err);
			goto loop;
		}
		else
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS232] ReceiveFrame Successful, Ready to OSQPost Str_Q");
			pUpCommMsg->GprsGvc = FALSE;
        	pUpCommMsg->Device = CommDevice;
        	OSQPost(Str_Q,(void*)(pUpCommMsg));
        }
        LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecRS232] Waiting Recive Frame Data!");
	}
}

/****************************************************************************************************
**	任 务  名 称: TaskUpRecGPRS
**	任 务  功 能: 负责接收GPRS的数据，数据效验正确, 之后交由分析帧任务分析执行
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
uint32 gGprsErrCounter	= 0x00;
//char gGPRSTestData[1024] = {0x00};
void  TaskUpRecGprs(void *pdata)
{
 	
 	uint8 err,temp;
    uint16 n,len;
	uint8 CommDevice;
	
	_Protocol 		Pro_Object_Rec;
	UP_COMM_MSG 	*pUpCommMsg;
	BaseProtocol 	*pBase;
  	
	gGprsErrCounter	= 0x00;
  	CommDevice = *((uint8*)pdata);
  	pBase = (BaseProtocol *)&Pro_Object_Rec;
  	//memset(gGPRSTestData, 0x00, 1024);

 	while(1)
    {
		do 
        {	
        	//这里不喂狗,因为正常情况下一定能申请到(在保证足够多的内存块的情况下,即BLK_NUM要足够大),否则说明异常
        	OSSemPend(CommTxPartSem, 0, &err);
       	}while (err != OS_ERR_NONE);
       	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] Recived a CommTxPartSem, Ready to OSMemGet !");
       	
		pUpCommMsg=(UP_COMM_MSG*)OSMemGet(CommTxBuffer, &err);
		if(err != OS_ERR_NONE)
		{
			OSSemPost(CommTxPartSem);
			LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: [TaskUpRecGprs] OSMemGet Failure!");
			continue;
		}
		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] OSMemGet Successful!");
		
		#if TEST_GPRS_EN > 0
		n=TestUpRec(CommDevice, pUpCommMsg->buff, MAX_REC_MC52i_BYTENUM-4);
		pUpCommMsg->GprsGvc  = FALSE;
        pUpCommMsg->Device   = CommDevice;
       	pUpCommMsg->ByteNum  = n;
       	OSQPost(Str_Q,(void*)(pUpCommMsg));
        continue;
		#endif
			
loop:	len = pBase->minFrameLen;
		do 
        {
          	FeedTaskDog();
            err=UpGetStart(CommDevice, OS_TICKS_PER_SEC*6);   
    	}while ( (err!=0) );
    	
    	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] OSMemGet Successful!");	
		//debug_debug(gDebugModule[GPRS_MODULE]," [TaskUpRecGprs] PreSmybol get Successful!");
		
		n=0;
		pUpCommMsg->buff[n++]=0x7B;
		for(n=n; n<len; n++)
       	{
      	    err=UpGetch(CommDevice, &temp, OS_TICKS_PER_SEC);
         	if(err)
         	{
         		goto loop;
         	}
       		pUpCommMsg->buff[n]=temp;
		
       	}
		err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
		//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] The Receive Data is %s", PUBLIC_HexStreamToString(pUpCommMsg->buff, len, gGPRSTestData));
		if (err == 4) 
       	{	
       		
       	   	if (len>(RECE_BUF_LEN*4)) 
       	   	{
       	   		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] Receive Frame Length is short, Return Value is %d", err);
       	     	//LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] The Receive Data is %s", PUBLIC_HexStreamToString(pUpCommMsg->buff, len, gGPRSTestData));
       	     	goto loop;
       	   	}
       	   	//
       	   	for(n=n;n<len;n++)
       	   	{
     	    	err=UpGetch(CommDevice,&temp,OS_TICKS_PER_SEC);
         		if(err)
         		{
         			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] Com get data err, Return Value is %d", err);
         		//	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] The Receive Data is %s", PUBLIC_HexStreamToString(pUpCommMsg->buff, len, gGPRSTestData));
         			goto loop;
         		}
         		pUpCommMsg->buff[n]=temp;
       		}
       	   	err = pBase->ReceiveFrame(pUpCommMsg->buff,len);
       	}
		if(err)
		{
			gGprsErrCounter++;
			LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: [TaskUpRecGprs] ReceiveFrame Failure, Return Value is %d" 
								  "The Error Counter is %2d", err, gGprsErrCounter);
		    //LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] The Receive Data is %s", PUBLIC_HexStreamToString(pUpCommMsg->buff, len, gGPRSTestData));
			goto loop;
		}
		else
		{
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] ReceiveFrame Successful, Ready to OSQPost Str_Q");
			pUpCommMsg->GprsGvc = FALSE;
        	pUpCommMsg->Device  = CommDevice;
        	OSQPost(Str_Q, (void*)(pUpCommMsg));
        }
        LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: [TaskUpRecGprs] Waiting Recive Frame Data!");
	}
}

/****************************************************************************************************
**	任 务  名 称: TaskReportUp
**	任 务  功 能: 负责系统的主动上报功能
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
void  TaskReportUp(void *pdata)
{
   	uint8 err;
   	void* cp1;
   
   	_Protocol 		Pro_Object_Rec;
	UP_COMM_MSG 	*pUpCommMsg;
	BaseProtocol 	*pBase;
	//uint8 testHeartFrm[100] = {0x00};

  	pBase = (BaseProtocol *)&Pro_Object_Rec;
 	
	while(1)
    {   	    	
    	do 
        {	
        	//这里不喂狗,因为正常情况下一定能申请到(在保证足够多的内存块的情况下,即BLK_NUM要足够大),否则说明异常
        	OSSemPend(CommTxPartSem, 0, &err);
       	}while (err != OS_ERR_NONE) ;
       	 
		pUpCommMsg=(UP_COMM_MSG*)OSMemGet(CommTxBuffer,&err);
		if(err != OS_ERR_NONE)
			{
				OSSemPost(CommTxPartSem);
				continue; 
			}
			
		do 
        {	
        	FeedTaskDog();
        	OSTimeDly(3*OS_TICKS_PER_SEC); 
        	cp1 = OSMboxAccept(HeartFrmMbox);
        	
       	}while ( (cp1==(void*)0) );
       	
    	if (cp1!=(void*)0)
    		{
    				
     			pUpCommMsg->GprsGvc = FALSE;
				pUpCommMsg->Device = UP_COMMU_DEV_GPRS;
				debug("INFO: [TaskReportUp] Up Report Msg The cp1 = %d \r\n", cp1);
		    	
    			pUpCommMsg->ByteNum = pBase->CreateHeartFrm(pUpCommMsg->buff,(uint32)cp1);//sizeof(testHeartFrm);////注意,这里要改称调用组心跳帧函数
       			OSQPost(UpSend_Q,(void*)(pUpCommMsg));
            
   			}
   		else
   			{
   				OSMemPut(CommTxBuffer,(void*)(pUpCommMsg));
      			OSSemPost(CommTxPartSem);
   			}
	}
}
   
/****************************************************************************************************
**	函 数  名 称: Createtasks_up
**	函 数  功 能: 创建上行通信任务
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/
void Createtasks_up(void)
{
	uint8 err;
	err=OSTaskCreate(TaskUpSend, (void *)0, &TaskUpSendStk[TASK_STK_SIZE - 1], PRIO_TASK_UP_SEND);
    if(err!=OS_ERR_NONE){
    	while(1);
    }
	err=OSTaskCreate(TaskUpAnalFr, (void *)0, &TaskUpAnalFrStk[TASK_STK_SIZE*2 - 1], PRIO_TASK_UP_ANAL_FR);
    if(err!=OS_ERR_NONE){
    	while(1);
    }
    
	gCommDeviceZigbee = UP_COMMU_DEV_ZIGBEE;
    gCommDeviceRS485  = UP_COMMU_DEV_485;
    gCommDeviceRS232  = UP_COMMU_DEV_232;
	gCommDeviceGprs   = UP_COMMU_DEV_GPRS;

    err=OSTaskCreate(TaskUpRecZigbee, (void *)(&gCommDeviceZigbee), &TaskUpRecZigbeeStk[TASK_STK_SIZE - 1], PRIO_TASK_UP_REC_ZIGBEE);
    if(err!=OS_ERR_NONE){
    	while(1);
    }
    err=OSTaskCreate(TaskUpRecRS485, (void *)(&gCommDeviceRS485), &TaskUpRecRS485Stk[TASK_STK_SIZE - 1], PRIO_TASK_UP_REC_RS485);
    if(err!=OS_ERR_NONE){
    	while(1);
    }
    err=OSTaskCreate(TaskUpRecRS232, (void *)(&gCommDeviceRS232), &TaskUpRecRS232Stk[TASK_STK_SIZE - 1], PRIO_TASK_UP_REC_RS232);
    if(err!=OS_ERR_NONE){
    	while(1);
    }
    err=OSTaskCreate(TaskUpRecGprs, (void *)(&gCommDeviceGprs), &TaskUpRecGprsStk[TASK_STK_SIZE - 1], PRIO_TASK_UP_REC_GPRS);     
    if(err!=OS_ERR_NONE){
    	while(1);
    }
	err=OSTaskCreate(TaskReportUp, (void *)(0), &TaskReportUpStk[TASK_STK_SIZE - 1], PRIO_TASK_REPORT_UP);     
    if(err!=OS_ERR_NONE){
    	while(1);
    }
	
}

/****************************************************************************************************
**	函 数  名 称: Createtasks_up
**	函 数  功 能: 创建上行通信任务所需的信号量，消息邮箱，消息队列等
**	输 入  参 数: 无
**	输 出  参 数: 无 
**  返   回   值: 
**	备		  注: 无
*****************************************************************************************************/  
void CreateMboxs_up(void)
{
	uint8 err;
	

	UpSend_Q = OSQCreate(&UpSend_Q_Grp[0],BLK_NUM);
	if (UpSend_Q == NULL)
    {
        while (1);
    }
	Str_Q = OSQCreate(&MsgGrp[0],BLK_NUM);
	if (Str_Q == NULL)
    {
        while (1);
    }
	/*begin:yangfei added 2013-03-15  for 增加buff 缓存大小*/
    CommTxBuffer = OSMemCreate(CommTxPart,BLK_NUM,(RECE_BUF_LEN*4),&err);//内存分配
    
  	if (CommTxBuffer == NULL)
    {
        while (1);
    }
 	CommTxPartSem = OSSemCreate(BLK_NUM);//初始化为内存块个数，随内存块的释放和使用而增减
    if (CommTxPartSem == NULL)
    {
        while(1);
	}

}
    
/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
