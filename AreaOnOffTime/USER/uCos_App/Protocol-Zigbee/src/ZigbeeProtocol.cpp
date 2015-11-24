/*******************************************Copyright (c)*******************************************
**									山东华宇空间技术公司(西安分部)                                **                            
**  文   件   名: ReadMeterdata.cpp																  **
**  创   建   人: 勾江涛																		  **
**	版   本   号: 0.1																		      **
**  创 建  日 期: 2012年9月20日 													     	      **
**  描        述: zigbee协议									      						  **
**	修 改  记 录:   							  												  **
****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>
#include "app_flashmem.h"
#include "app_up.h"
#include "TermControl.h"
/********************************************** static *********************************************/
extern OS_EVENT *UpLandAnswMbox;
extern TermParaSaveType gPARA_TermPara;

static uint8 gPRO_MsgIndex	= 0;							//上传数据的消息序号
static uint8 gPRO_ReceiveData[1024];						//上行接收最大数据帧字节
static uint8 gPRO_SendData[1024];							//上行发送最大数据帧字节
char gPrintData[1024]={0};	

uint8 gDebugModule[20]={0};		
uint8 gDebugBuffer[250]={0};
extern uint16 gDebugLength;
/********************************************************************************************************
**  函 数  名 称: PRO_InitGlobalVar				    							                       
**	函 数  功 能: 初始化全局变量                                                                   			
**	输 入  参 数: none							         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 	                                                                                   																									
*********************************************************************************************************/
void PRO_InitGlobalVar(void)
{
	gPRO_MsgIndex = 0x00;
	memset(gPRO_ReceiveData, 0x00, sizeof(gPRO_ReceiveData));
	memset(gPRO_SendData, 0x00, sizeof(gPRO_SendData));
	memset(gDebugModule, 0x00, sizeof(gDebugModule));

	memset(gu8MBusShortFlag,0,METER_CHANNEL_NUM);
	gu8ReadAllMeterFlag = 0;
	
}
/********************************************************************************************************
**  函 数  名 称: IfFrameHead				    							                       
**	函 数  功 能: 头指针                                                               			
**	输 入  参 数: uint8 bHead							         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 	                                                                                   																									
*********************************************************************************************************/
uint8 _Protocol::IfFrameHead(uint8 bHead)
 {
 	if (bHead!=frameStart) return FALSE;
	return TRUE;
 }
/********************************************************************************************************
**  函 数  名 称: CreateFrame				    							                       
**	函 数  功 能: 创建帧                                                               			
**	输 入  参 数: uint8 *_data
**                uint8 *_sendBuf	
**                uint16 &lenFrame						         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 	                                                                                   																									
*********************************************************************************************************/ 
 void _Protocol::CreateFrame(uint8 *_data, uint8 *_sendBuf, uint16 &lenFrame)
 {
    uint8 *pTemp			= _sendBuf;
    uint8 *pHeadCheck		= NULL;
    uint8 *pContentCheck	= NULL;

	LOG_assert_param( _data == NULL );
	LOG_assert_param(_sendBuf == NULL );

    _ProtocolType *p_ProData=NULL;
    p_ProData = (_ProtocolType *)_data;
    
    lenFrame = 0;
    *pTemp++ = 0xFB;							//前导符，个数默认为2个
    *pTemp++ = 0xFB;
    
    *pTemp++ = 0x7B;							//起始符， 0x7B
    pHeadCheck = pTemp;							//消息头校验预留
    *pTemp++ = PROTOCOL_VER;					//版本号
    lenFrame = 4;
    
    memcpy(pTemp, p_ProData->DestAddr, 6);		//源地址为集中器地址，是主站发送来的目标地址
    pTemp += 6;
    memcpy(pTemp, p_ProData->SourceAddr, 6);	//目标地址为主站地址，是主站发来的源地址
    pTemp += 6;
    lenFrame += 12;
    
    *pTemp++ = gPRO_MsgIndex++;					//MID 消息序列
    
    *pTemp++ = p_ProData->MsgLength;			//消息体长度
    *pTemp++ = p_ProData->MsgLength >> 8;
    
    *pTemp++ = p_ProData->MsgType;				//消息类型
    lenFrame += 4;
    
    memcpy(pTemp, p_ProData->TimeSmybol, 6);	//时间标签
    pTemp += 6;
    
    *pTemp++ = CountCheck(pHeadCheck, 23);		//消息头校验字节
    pContentCheck = pTemp;						//消息体校验字节预留
    lenFrame += 7;
    
    memcpy(pTemp, p_ProData->Data.pDataBack, p_ProData->MsgLength);
    pTemp += p_ProData->MsgLength;
    lenFrame += p_ProData->MsgLength;
    											//消息体校验字节
    *pTemp++ = CountCheck(pContentCheck, p_ProData->MsgLength);		
    
    *pTemp++ = 0xFD;
    *pTemp++ = 0xFD;

	lenFrame += 3;
    
	//下面为配合主站测试用
	//*pTemp++ = 0x7D;

    //lenFrame += 4;
 }
/********************************************************************************************************
**  函 数  名 称: ReceiveFrame				    							                       
**	函 数  功 能: 接收帧                                                              			
**	输 入  参 数: uint8* _receBuf	
**                uint16 &lenFrame						         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 	                                                                                   																									
*********************************************************************************************************/ 
 uint8 _Protocol::ReceiveFrame(uint8* _receBuf, uint16 &lenFrame)
 {
 	int8 CheckSource 	= 0;
 	int8 CheckDest 		= 0;
 	uint8 b1, b2;
 	uint8 HeadCheck 	= 0x00;
 	uint8 ContentCheck 	= 0x00;
 	uint8 *pTemp		= _receBuf;
 	uint8 *pHeadTemp;
 	uint8 *pContentTemp;
 	uint8 SourceAddr[6] = {0x00};		    //源地址，发送方地址
 	uint8 DestAddr[6]	= {0x00};			//目的地址，接收方地址
 	uint8 TimeSmybol[6] = {0x00};			//TS 时间标签
  	uint16 DataLen=0, lenTemp=0, lenTemp2=0;
	
	LOG_assert_param( _receBuf == NULL );

	CPU_SR		cpu_sr;

	pTemp++;								//数据帧头
	pHeadTemp = pTemp;						//预留指针，用于对消息头进行效验
    pTemp++;								//协议版本号
    
    memcpy(SourceAddr, pTemp, sizeof(SourceAddr));
    pTemp += sizeof(SourceAddr);			//复制源地址
    memcpy(DestAddr, pTemp, sizeof(DestAddr));
    pTemp += sizeof(DestAddr);				//复制目的地址
    OS_ENTER_CRITICAL();
    CheckSource = CmpMeterNo(SourceAddr, gPARA_TermPara.HostAddr);
    CheckDest	= CmpMeterNo(DestAddr, gPARA_TermPara.TermAddr);
    OS_EXIT_CRITICAL();
	
    if((CheckSource!=0) || (CheckDest!=0))				
    	{
			
			LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <_Protocol::ReceiveFrame> Source or Dest Addr is not correct!");
    		return 2;
    	}
    pTemp++;								//MID消息序号
        
	b1 = *pTemp++;
	b2 = *pTemp++;
	DataLen =  (b2<<8) + b1;				//数据域长度2个字节
	
	pTemp++;								//MT消息类型

	memcpy(TimeSmybol, pTemp, sizeof(TimeSmybol));
	pTemp += sizeof(TimeSmybol);			//复制时间标签
	/*yangfei deleted 2013-01-22 for debug need changed*/
	#if 0
	if(TimeCheck(TimeSmybol))									
		{
    		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <_Protocol::ReceiveFrame> TimeSmybol is not correct!");
    		return 3;
    	}
	#endif
	//对消息头进行检验
	HeadCheck = *pTemp++;
	if(HeadCheck != CountCheck(pHeadTemp, 23) )					
		{
    		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <_Protocol::ReceiveFrame> HeadCheck is not correct!");
    		return 5;
    	}
	
	pContentTemp = pTemp;					//预留指针对消息体进行效验
	
	lenTemp2 = lenFrame;
	lenTemp = DataLen + 26 + 2;				// 数据域长度 + 消息头 + 2个字节的结束符
    lenFrame = lenTemp;
	/*yangfei deleted 2013-01-22 for debug need changed*/
	#if 1
	if (lenTemp2<lenTemp) 	
		{									//长度不够，未接收到完整帧
    		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <_Protocol::ReceiveFrame> ReceivedFrame Lenght is not correct!");
    		return 4;
    	}		
	#endif
	
	pTemp = _receBuf + lenFrame-3;
	ContentCheck = *pTemp++;
	if(ContentCheck != CountCheck(pContentTemp, DataLen) )		
		{
    		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <_Protocol::ReceiveFrame> BodyCheck is not correct!");
    		return 5;
    	}
		
	if ( (*pTemp++ !=0xFD) || (*pTemp++ !=0xFD)) 		
		{
    		LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <_Protocol::ReceiveFrame> EndSmybol is not correct!");
    		return 6;
    	}
	/*begin:yangfei added 2013-03-12 for 帧转发*/
	#if  0
	{
	 uint8 PreSmybol[2]={0xFB,0xFB};
	 if(CmpMeterNo(DestAddr, gPARA_TermPara.HostAddr)==0&&CmpMeterNo(SourceAddr, gPARA_TermPara.TermAddr)!=0)/*转发集抄器上行数据*/
	 	{
	 	UpDevSend(UP_COMMU_DEV_GPRS, PreSmybol, 2);
	 	UpDevSend(UP_COMMU_DEV_GPRS,  _receBuf, lenFrame);
		return 7;
	 	}
	if(CheckSource == 0&&CheckDest != 0)/*转发集抄器下行数据*/
     	{
     	UpDevSend(UP_COMMU_DEV_ZIGBEE,  PreSmybol, 2); 
     	UpDevSend(UP_COMMU_DEV_ZIGBEE,  _receBuf, lenFrame); 
		return 8;
     	}
	}
	#endif
	/*end:yangfei added 2013-03-12 for 帧转发*/
	return 0;
 }


/********************************************************************************************************
**  函 数  名 称: AnalysisFrame				    							                       
**	函 数  功 能: 帧解析                                                            			
**	输 入  参 数: uint8* _receBuf	
**                uint16 &lenFrame						         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 	                                                                                   																									
*********************************************************************************************************/ 
 uint8 _Protocol::AnalysisFrame(uint8* _receBuf, uint16 &lenBuf)
 {
  	uint8 *pTemp 	= NULL;
    uint8 b1,b2;
	uint8 err = 0;
	
									//_receBuf为接收到上行报文的指针，指向头一个为FRAME_START
   	LOG_assert_param( _receBuf == NULL );

    _ProtocolType _ProData;
    _ProData.Data.pData = gPRO_ReceiveData;
    _ProData.Data.pDataBack = gPRO_SendData;
    
	pTemp = _receBuf;
	pTemp++;									//消息头
	_ProData._Ver = *pTemp++;					//协议版本号
	memcpy(_ProData.SourceAddr, pTemp, 6);		//源地址
	pTemp += 6;
	memcpy(_ProData.DestAddr, pTemp, 6);		//目标地址
	pTemp += 6;
	
	_ProData.MsgID = *pTemp++;					//消息序号
	
	b1 = *pTemp++;
	b2 = *pTemp++;
	_ProData.MsgLength = (b2<<8) + b1;			//消息体长度

	_ProData.MsgType = *pTemp++;				//消息类型
	
	memcpy(_ProData.TimeSmybol, pTemp, 6);		//时间标签
	pTemp += 6;

	pTemp++;									//消息头效验
	
	_ProData.Data.pData 	= pTemp;			//消息体
	_ProData.Data.lenData 	= _ProData.MsgLength;
	/*begin:yangfei added 2013-02-18 */
	if(gDebugModule[TASKUP_MODULE] >= KERN_DEBUG)
		{
			PUBLIC_HexStreamToString(_receBuf, 28+_ProData.MsgLength, gPrintData);
			debug_debug(gDebugModule[TASKUP_MODULE],"recevie data:%s\r\n",gPrintData);	
		}
	/*end:yangfei added 2013-02-18*/
	if( (_ProData.MsgType == 0x2D) )
		{
			if( (_ProData.Data.lenData == 0x01)&&(_ProData.Data.pData[0] == 0x01) )
				{	
					LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <AnalysisFrame> Recvie GPRS UpLand Confirm Frame OK! Then Post UpLandAnswMbox!");
					OSMboxPost(UpLandAnswMbox,(void*)1);
				}
			return 1;
		}
	if( (_ProData.MsgType == 0x2F) )
		{
			if( (_ProData.Data.lenData == 0x01)&&(_ProData.Data.pData[0] == 0x01) )
				{
					LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <AnalysisFrame> Recvie GPRS Heart Confirm Frame OK!");
					return 2;
				}
			
		}

	UpdGprsRunSta_FeedRecDog();
	UpdGprsRunSta_FeedSndDog();
	LOG_WriteSysLog_Format(LOG_LEVEL_INFO, "INFO: <AnalysisFrame> Recvie GPRS Data OK! Clear GPRS Send and Recive Dog!"
						   "The MsgType is %2XH", _ProData.MsgType);	
	/*begin:yangfei added 2013-03-12 for 帧转发needed deleted*/
	#if  0
	if(_ProData.MsgType == 0x82||_ProData.MsgType == 0x83||_ProData.MsgType == 0x84||_ProData.MsgType == 0x85)
	{
	 uint8 PreSmybol[2]={0xFB,0xFB};
	 if(CmpMeterNo(_ProData.DestAddr, gPARA_TermPara.HostAddr)==0&&CmpMeterNo(_ProData.SourceAddr, gPARA_TermPara.TermAddr)==0)/*转发集抄器上行数据*/
	 	{
	 	uint16 BufNum=0xaa55;/*send PreSmybo 0xFB0xFB*/
	 	debug_debug(gDebugModule[TASKUP_MODULE],"GPRS retransmit\r\n");
		GprsIPSEND_xmz(_receBuf,_ProData.MsgLength+28,&BufNum);
		return 7;
	 	}
	if(CmpMeterNo(_ProData.DestAddr, gPARA_TermPara.TermAddr)==0&&CmpMeterNo(_ProData.SourceAddr, gPARA_TermPara.HostAddr)==0)/*转发集抄器下行数据*/
     	{
     	debug_debug(gDebugModule[TASKUP_MODULE],"ZIGBEE retransmit\r\n");	
     	UpDevSend(UP_COMMU_DEV_ZIGBEE,  PreSmybol, 2); 
     	UpDevSend(UP_COMMU_DEV_ZIGBEE,  _receBuf,  _ProData.MsgLength+28); 
		return 8;
     	}
	}
	#endif
	/*end:yangfei added 2013-03-12 for 帧转发*/	
	err = ProcessFrame(_ProData); 
	if(err == NO_ERR)
		{
	    CreateFrame((uint8 *)&_ProData, _receBuf, lenBuf);
		/*begin:yangfei added 2013-02-18 */
		if(gDebugModule[TASKUP_MODULE] >= KERN_DEBUG)
			{
				PUBLIC_HexStreamToString(_receBuf, lenBuf, gPrintData);
				debug_debug(gDebugModule[TASKUP_MODULE],"send data:%s\r\n",gPrintData);	
			}
		/*end:yangfei added 2013-02-18*/
		}
    return err;
 }

/********************************************************************************************************
**  函 数  名 称: CreateHeartFrm			    							                       
**	函 数  功 能: 创建心跳帧                                                            			
**	输 入  参 数: uint8* _receBuf	
**                uint16 &lenFrame						         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 	                                                                                   																									
*********************************************************************************************************/
uint8 _Protocol::CreateHeartFrm(uint8* HeartFrm, uint8 flag)
{
	uint16 len = 0x00;
	_ProtocolType _ProData;
	CPU_SR	cpu_sr;
	
	_ProData.Data.pDataBack = gPRO_SendData;
	_ProData._Ver = PROTOCOL_VER;				//协议版本号
    
    OS_ENTER_CRITICAL();
	memcpy(_ProData.SourceAddr, gPARA_TermPara.HostAddr, 6);	//源地址
	memcpy(_ProData.DestAddr, gPARA_TermPara.TermAddr, 6);		//目标地址
	OS_EXIT_CRITICAL();
	
	if(flag == 0x01) /*GPRS登陆帧*/
		{
			_ProData.MsgLength = 6;						//消息体长度
			_ProData.MsgType = 0x2C;					//消息类型
			memcpy(_ProData.Data.pDataBack, gPARA_TermPara.TermAddr, 6);
			_ProData.Data.lenDataBack 	= 6;
			/*begin:yangfei added 2013-02-20*/
			debug_debug(gDebugModule[GPRS_MODULE],"GPRS log in frame !\r\n");
			/*end:yangfei added 2013-02-20*/
		}
	if(flag == 0x03)  /*GPRS 心跳帧*/
		{
			_ProData.MsgLength = 1;						//消息体长度
			_ProData.MsgType = 0x2E;					//消息类型
			_ProData.Data.pDataBack[0] = 0x01;
			_ProData.Data.lenDataBack 	= 1;
			/*begin:yangfei added 2013-02-20*/
			debug_debug(gDebugModule[GPRS_MODULE],"GPRS heart frame !\r\n");
			/*end:yangfei added 2013-02-20*/
		}
	/*begin:yangfei added 2013-08-03 for add debug message */
	if(flag == 0x10)  /*调试信息帧，发送给上位机*/
		{
			uint16 len = gDebugLength;
			_ProData.MsgLength = len;						//消息体长度
			_ProData.MsgType = 0x10;					//消息类型
			//_ProData.Data.pDataBack[0] = 0x01;
			memcpy(_ProData.Data.pDataBack,gDebugBuffer,_ProData.MsgLength);
			_ProData.Data.lenDataBack 	= len;
			/*begin:yangfei added 2013-02-20*/
			debug_debug(gDebugModule[GPRS_MODULE],"GPRS debug message  frame ! len=%d \r\n",_ProData.MsgLength);
			/*end:yangfei added 2013-02-20*/
		}
	/*end:yangfei added 2013-08-03 for add debug message */
	ReadDateTime(_ProData.TimeSmybol);			//时间标签
	
	CreateFrame((uint8 *)&_ProData, HeartFrm, len);
	
	return len; 
}
/********************************************************************************************************
**  函 数  名 称: ProcessFrame			    							                       
**	函 数  功 能: 处理帧                                                           			
**	输 入  参 数: 					         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 单帧传输或多帧传输的开始，用该函数	                                                                                   																									
*********************************************************************************************************/ 
uint8 _Protocol::ProcessFrame(_ProtocolType &_ProData) 
{
	uint8 err = 0;
	if(_ProData.MsgType != 0x11)			//备份存储数据类型，但是0x11不能存储
		{
			memcpy((uint8 *)&MultiBackup, (uint8 *)&_ProData, sizeof(_ProtocolType));
			//此处数据域备份需要注意，ProData.Data.lenData不能大于100.
			memcpy(DataFiledBack, _ProData.Data.pData, _ProData.Data.lenData);
			memset(ControlSave, 0x00, sizeof(ControlSave));
		}
	
	switch(_ProData.MsgType)
    {
    	case 0x02:
    	    PARA_ProcessMsg_02(_ProData);
    	    break;
    	case 0x04:
    		//test_proMsg_04(_ProData);    //通断时间面积法中去除这条指令。相同功能由0x06实现。
    		err = ERR_1;
    		break;
		case 0x06:
			DEAL_ProcessMsg_06(_ProData);  //根据计量点单抄表。
			break;
    	case 0x0C:
    	    PARA_ProcessMsg_0C(_ProData);
    	    break;
		case 0x13:
			PARA_ProcessMsg_13(_ProData); //修改单个表地址信息。
			break;

    	case 0x0E:
    	    DEAL_ProcessMsg_0E(_ProData, ControlSave);
    	    break;       
    	case 0x11:								//多帧情况下，才可以请求重新传输和下一包传输
    	      /*begin:2013102501 yangfei modified 2013-10-25 for 最好一包丢失时，上位机再请求时不会进入ProcessMsg_11，因为ControlSave[0] =0*/
              ProcessMsg_11(_ProData, ControlSave);
			  /*end:yangfei modified 2013-10-25 for 最好一包丢失时，上位机再请求时不会进入ProcessMsg_11，因为ControlSave[0] =0*/
    	    break;
    	case 0x18:
    	    DEAL_ProcessMsg_18(_ProData);
    	    break;
    	case 0x1A: 
    	    DEAL_ProcessMsg_1A(_ProData, ControlSave);
    	    break;
    	case 0x1C:
    		DEAL_ProcessMsg_1C(_ProData);
    		break;
    	case 0x1E:
    		DEAL_ProcessMsg_1E(_ProData, ControlSave);
    		break;
    	case 0x20:
    	    PARA_ProcessMsg_20(_ProData);
    	    break;
    	case 0x22:
    	    DEAL_ProcessMsg_22(_ProData, ControlSave);
    	    break;
		case 0x24:
    	    PARA_ProcessMsg_24(_ProData);
    	    break;
    	case 0x26:
    	    PARA_ProcessMsg_26(_ProData);
    	    break;
		case 0x34:
			PARA_ProcessMsg_34(_ProData);  //指令重启集中器
			break;
		case 0x3A:
    	    PARA_ProcessMsg_3A(_ProData);  //用于指令设置补抄表相关参数。
    	    break;
		case 0x3C:
    	    DEAL_ProcessMsg_3C(_ProData);  //读指定时间指定表号的历史数据。
    	    break;
		case 0x3E:
			DEAL_ProcessMsg_3E(_ProData);  //设置集中器濉板"立即抄表"
			break;
			
		case 0x40:
    	    DEAL_ProcessMsg_40(_ProData, ControlSave);
			break;
		case 0x42:
    	    DEAL_ProcessMsg_42(_ProData);  //任意透传
			break;

		case 0x50:				/*读取终端基础信息*/
			PARA_ProcessMsg_50(_ProData);
			break;		
		case 0x51:				/*读取终端表档案信息*/
			err = PARA_ProcessMsg_51(_ProData); //Todo:need to be modified
			//PARA_ProcessMsg_83(_ProData); 	
			break;
		case 0x52:				/*读取表档案统计信息*/
			PARA_ProcessMsg_52(_ProData);
			break;
		case 0x53:				/*读取终端抄表时间点*/
			PARA_ProcessMsg_53(_ProData);
			break;
		case 0x54:				/*读取主站IP和端口通讯参数*/
			PARA_ProcessMsg_54(_ProData);
			break;		
		case 0x55:				/*读取终端IP和端口参数*/
			err = PARA_ProcessMsg_55(_ProData); //Todo:need to be modified
			//PARA_ProcessMsg_83(_ProData); 	
			break;
		case 0x56:				/*读取调试用表档案信息*/
			PARA_ProcessMsg_56(_ProData);
			break;

		case 0x66:
			PARA_ProcessMsg_66(_ProData);  //下发通断时间面积法上位机分摊值。
			break;
			
		/*begin:yangfei added 2013-01-16 for update*/		
		case 0x82:              /*主站发送升级开始标志*/
    	    PARA_ProcessMsg_82(_ProData);
    	    break;		
    	case 0x83:				/*主站发送升级程序*/
    	    err = PARA_ProcessMsg_83(_ProData); //Todo:need to be modified
			//PARA_ProcessMsg_83(_ProData);		
    	    break;
		case 0x84:				/*主站下发远程升级结束标志*/
    	    PARA_ProcessMsg_84(_ProData);
    	    break;
		case 0x85:				/*主站询问文件缺包情况*/
    	    PARA_ProcessMsg_85(_ProData);
    	    break;
		/*end:yangfei added 2013-01-16 for update*/	

		case 0x90:
			PARA_ProcessMsg_90(_ProData);  //读集中器IP/端口号/软硬件版本等信息。
			break;

		case 0x92:
			DEAL_ProcessMsg_92(_ProData, ControlSave);  //读集中器中的表基础信息。
			break;

		case 0x94:
			PARA_ProcessMsg_94(_ProData);  //指令更改IP、端口号。
			break;
			
    	default:
    		err = ERR_1;
			debug_debug(gDebugModule[TASKUP_MODULE],"Message Type err!\r\n");	
    	 	break;
		}
	return err;
}

/********************************************************************************************************
**  函 数  名 称: ProcessMsg_11		    							                       
**	函 数  功 能: 信息处理                                                         			
**	输 入  参 数: 					         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 本函数只会正在有多帧传输数据时才会进入	                                                                                   																									
*********************************************************************************************************/ 
uint8 _Protocol::ProcessMsg_11(_ProtocolType &_ProData, uint8 *pControlSave)
	{
		uint8 MultiControlData;

		LOG_assert_param( pControlSave == NULL );

		MultiControlData = *(_ProData.Data.pData);			//取需要发送第几包
		
		memcpy((uint8 *)&_ProData, (uint8 *)&MultiBackup, sizeof(_ProtocolType));
		memcpy(_ProData.Data.pData, DataFiledBack, _ProData.Data.lenData);
		
		switch(_ProData.MsgType)
    	{
   			case 0x0E:
				MultiFrameMsg_0E *pMultiPack_0E;
   				pMultiPack_0E = (MultiFrameMsg_0E *)pControlSave;
   				
   				if(MultiControlData == pMultiPack_0E->SendPackIndex)
   					{										//请求重发上一包
   						DEAL_ProcessMsg_0E(_ProData, pControlSave);
   					}
   				if(MultiControlData == (pMultiPack_0E->SendPackIndex + 1))
   					{										//请求下一包数据
   						pMultiPack_0E->StartMeterSn += pMultiPack_0E->SendMeterNums;
						pMultiPack_0E->SendPackIndex++;
   						DEAL_ProcessMsg_0E(_ProData, pControlSave);
   					}
    		    //请求其他包数据，不予响应
    		    break;
   			case 0x18:
    		    DEAL_ProcessMsg_18(_ProData);
    		    break;
   			case 0x1A: 
    		    DEAL_ProcessMsg_1A(_ProData, ControlSave);
    		    break;
    		case 0x1C:
    			DEAL_ProcessMsg_1C(_ProData);
    			break;
    		case 0x1E:
    			MultiFrameMsg_1E *pMultiPack_1E;
   				pMultiPack_1E = (MultiFrameMsg_1E *)pControlSave;
   				
   				if(MultiControlData == pMultiPack_1E->SendPackIndex)
   					{										//请求重发上一包
   						DEAL_ProcessMsg_1E(_ProData, pControlSave);
   					}
   				if(MultiControlData == (pMultiPack_1E->SendPackIndex + 1))
   					{										//请求下一包数据
   						pMultiPack_1E->StartReadAddr += pMultiPack_1E->SendByteNums;
						pMultiPack_1E->SendPackIndex++;
   						DEAL_ProcessMsg_1E(_ProData, pControlSave);
   					}
    			break;
    		
			case 0x22:
			{
    			MultiFrameMsg_22 *pMultiPack_22;
   				pMultiPack_22 = (MultiFrameMsg_22 *)pControlSave;
   				
   				if(MultiControlData == pMultiPack_22->SendPackIndex)
   					{										//请求重发上一包
   						DEAL_ProcessMsg_22(_ProData, pControlSave);
   					}
   				if(MultiControlData == (pMultiPack_22->SendPackIndex + 1))
   					{										//请求下一包数据
   						pMultiPack_22->StartMeterSn += pMultiPack_22->SendMeterNums;
						pMultiPack_22->SendPackIndex++;
   						DEAL_ProcessMsg_22(_ProData, pControlSave);
   					}
    		    break;
			}
			case 0x40:
			{
    			MultiFrameMsg_40 *pMultiPack_40;
   				pMultiPack_40 = (MultiFrameMsg_40 *)pControlSave;
   				
   				if(MultiControlData == pMultiPack_40->SendPackIndex)
   					{										//请求重发上一包
   						DEAL_ProcessMsg_40(_ProData, pControlSave);
   					}
   				if(MultiControlData == (pMultiPack_40->SendPackIndex + 1))
   					{										//请求下一包数据
   						pMultiPack_40->StartMeterSn += pMultiPack_40->SendMeterNums;
						pMultiPack_40->SendPackIndex++;
   						DEAL_ProcessMsg_40(_ProData, pControlSave);
   					}
    		    break;
    		}

			case 0x92:  //上传表地址信息。
			{
    			MultiFrameMsg_92 *pMultiPack_92;
   				pMultiPack_92 = (MultiFrameMsg_92 *)pControlSave;
   				
   				if(MultiControlData == pMultiPack_92->SendPackIndex)
   					{										//请求重发上一包
   						DEAL_ProcessMsg_92(_ProData, pControlSave);
   					}
   				if(MultiControlData == (pMultiPack_92->SendPackIndex + 1))
   					{										//请求下一包数据
   						pMultiPack_92->StartMeterSn += pMultiPack_92->SendMeterNums;
						pMultiPack_92->SendPackIndex++;
   						DEAL_ProcessMsg_92(_ProData, pControlSave);
   					}
    		    break;
			}
    	
    		default:
			{
				debug("ERROR:%s  _ProData.MsgType=%d multi frame transfer insert other frame\r\n",__FUNCTION__,_ProData.MsgType);
			}
    	 		break;
    	}
		return 0;
	}


/********************************************************************************************************
**  函 数  名 称: CountCheck	    							                       
**	函 数  功 能: 统计监测                                                        			
**	输 入  参 数: uint8 *_data
**                uint16 _len				         			 	       
**	输 出  参 数: none											                                       
**  返   回   值: none													                               
**	备		  注: 本函数只会正在有多帧传输数据时才会进入	                                                                                   																									
*********************************************************************************************************/ 
uint8 _Protocol::CountCheck(uint8 *_data, uint16 _len)
	{
		uint8 cs = 0;
    	uint16 i;

       	LOG_assert_param( _data == NULL );

    	for(i=0;i<_len;i++)
    	{
       		cs += *_data++;
    	}
    	
    	cs = (~cs) + 1;
    	
    	return cs; 
	}


/*****************************************************************************************
**								End of File
*****************************************************************************************/
