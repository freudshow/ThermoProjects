/*******************************************Copyright (c)*******************************************
**									É½¶«»ªÓî¿Õ¼ä¼¼Êõ¹«Ë¾(Î÷°²·Ö²¿)                                **                            
**  ÎÄ   ¼þ   Ãû: ReadMeterdata.cpp																  **
**  ´´   ½¨   ÈË: ¹´½­ÌÎ																		  **
**	°æ   ±¾   ºÅ: 0.1																		      **
**  ´´ ½¨  ÈÕ ÆÚ: 2012Äê9ÔÂ20ÈÕ 													     	      **
**  Ãè        Êö: ¶ÁÈ¡¼ÆÁ¿Æ÷Êý¾Ý									      						  **
**	ÐÞ ¸Ä  ¼Ç Â¼:   							  												  **
**  ±¸		  ×¢: ·§¿ØºÍÎÂ¿ØµÄÐ­Òé¶©µÄ²»ºÃ£¬Ôì³É³ÌÐòÊéÐ´Âß¼­½ÏÎªÂé·³!·ÑÉñ!
****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>
#include "app_flashmem.h"
#include "app_down.h"
#include "app_up.h"
#include "Valve.h"
#include "TermControl.h"



extern"C"
{
#include "ModBus.h" //²âÊÔmodbus  ,ÁÖÏþ±òÌí¼Ó
}
/********************************************** static *********************************************/
static uint8 gDEAL_ReadMailBox_Msg18[20] = {0x00};
static uint8 gDEAL_ReadMailBox_Msg1C[20] = {0x00};
static uint8 gDEAL_ReadDataTemp[10*METER_PER_CHANNEL_NUM] = {0x00};

/********************************************** global **********************************************/
void* CmdMsgQ[10];               	//¶¨ÒåÏûÏ¢Ö¸ÕëÊý×é, ×î¶à´æ´¢10¸öÏûÏ¢
OS_EVENT *CMD_Q; 

extern uint8 gDownCommDev485;



/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: test_proMsg_04				    							                       
**	º¯ Êý  ¹¦ ÄÜ: ²âÊÔÐ­ÒéÐÅÏ¢                                                                         			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	       
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢: 	                                                                                   																									
*********************************************************************************************************/

uint8 test_proMsg_04( _ProtocolType &_ProData)
{
	uint8 i=0, Err=0;
	uint8 Channel = 1;
	uint8 RetryTimes = 2;
	uint8 TempLen	= 0x00;
	uint8 *pTemp = _ProData.Data.pData;
	uint8 ProtocolVer	= 0x00;
	uint8 TempData[128] = {0x00};
	uint8 AddrTemp[7] = {0x00};
	uint8 err=0;
	//uint8 addr[7] = {0xFF, 0xFF, 0xF1, 0x22, 0x65, 0x48, 0x25};
	DELU_Protocol	ProtocoalInfo;
	
	ProtocolVer = *pTemp++;						//Ð­ÒéÀàÐÍ
	Channel = *pTemp++;
	
	if(ProtocolVer>PROTOCOL_MAX)	  ProtocolVer = 0;
	/*begin:yangfei added 2013-11-2 for ÅÐ¶Ï485 ÈÈ±í±ØÐëÎª7Í¨µÀ*/
	if(ProtocolVer==4&&Channel!=7)
		{
		debug("%s:485 meter channel!=7 \r\n",__FUNCTION__);
		goto ERROR;
		}
	/*end:yangfei added 2013-11-2 for ÅÐ¶Ï485 ÈÈ±í±ØÐëÎª7Í¨µÀ*/
	/*begin:yangfei added 2013-11-17 for ¶¨µã³­±í±êÖ¾£¬¶¨µã³­±íµÄÊ±ºò²»ÔÊÐíÊµÊ±³­±í£¬·ÀÖ¹Í¨µÀÇÐ»»*/
	OSSemPend(METERChangeChannelSem, OS_TICKS_PER_SEC, &err); //ÉêÇëMBUSÍ¨µÀÇÐ»»È¨Àû
	if(err!=OS_ERR_NONE)
		{
		debug("%s:nwo timing  read meter!\r\n",__FUNCTION__);
		goto ERROR;
		}
	/*end:yangfei added 2013-11-17 for ¶¨µã³­±í±êÖ¾£¬¶¨µã³­±íµÄÊ±ºò²»ÔÊÐíÊµÊ±³­±í£¬·ÀÖ¹Í¨µÀÇÐ»»*/
	(*METER_ComParaSetArray[gMETER_Table[ProtocolVer][0]])();
	/*begin:yangfei added for support 485 meter*/
      gDownCommDev485 = gMETER_Table[ProtocolVer][3];
       /*end:yangfei added for support 485 meter*/
	METER_ChangeChannel(Channel);

	//ÌîÐ´ÏàÓ¦µÄ ÈÈ¼ÆÁ¿±íÍ¨ÐÅÐ­Òé ½á¹¹Ìå
	ProtocoalInfo.PreSmybolNum  = gMETER_Table[ProtocolVer][2];
	ProtocoalInfo.MeterType 	= 0x20;

	memcpy(AddrTemp, pTemp, 7);
	if(ProtocolVer == 2)
	{							//ÈôÎªµ¤·ðË¹±í£¬Ôò½«±íµØÖ·Ç°5Î»ÎªFFFFF
		for(i=0; i<7; i++)
		{
			ProtocoalInfo.MeterAddr[i] = *(pTemp+6-i);
		}
		ProtocoalInfo.MeterAddr[0] = 0xFF;
		ProtocoalInfo.MeterAddr[1] = 0xFF;
		ProtocoalInfo.MeterAddr[2] |= 0xF0;
	}
	/*begin:yangfei added for support wanhua big meter 2013-08-12*/
	else if(ProtocolVer == 5)
	{
	memcpy(ProtocoalInfo.MeterAddr, pTemp, 7);
	ProtocoalInfo.MeterAddr[5] = 0x01;
	}
	/*end:yangfei added for support wanhua big meter 2013-08-12*/
	else
	{
		memcpy(ProtocoalInfo.MeterAddr, pTemp, 7);
	}
	//memcpy(ProtocoalInfo.MeterAddr, addr, 7);
	ProtocoalInfo.ControlCode 	= 0x01;
	ProtocoalInfo.Length		= 0x03;
	ProtocoalInfo.DataIdentifier= gMETER_Table[ProtocolVer][1];
	memset(ProtocoalInfo.DataBuf, 0x00, sizeof(ProtocoalInfo.DataBuf));


	/*begin:yangfei added 2013-08-05 for add HYDROMETER*/
	if(ProtocolVer == HYDROMETER775_VER||ProtocolVer == ZENNER_VER||ProtocolVer == LANDISGYR_VER||ProtocolVer == ENGELMANN_VER)
	{	
	Err = HYDROMETER(&ProtocoalInfo,ProtocolVer);
	if(Err == NO_ERR)
		{
		CJ188_Format CJ188_Data;
		
		CJ188_Data=METER_Data_To_CJ188Format(ProtocolVer,ProtocoalInfo.DataBuf,ProtocoalInfo.Length-3,&err);
		if(err==0)
			{
			 memcpy(&TempData[8], &CJ188_Data, sizeof(CJ188_Data));
			 ProtocoalInfo.Length = sizeof(CJ188_Data) + 3;
			}
		else
			{
			debug("%s %d METER_Data_To_CJ188Format err=%d\r\n",__FUNCTION__,__LINE__,err );
			 memcpy(&TempData[8], ProtocoalInfo.DataBuf, ProtocoalInfo.Length-3);/*ÉÏ±¨Ô­Ê¼Êý¾Ý*/
			}
       
		}
	else
		{
		debug("%s %d  err=%d\r\n",__FUNCTION__,__LINE__,err );
		}
	memcpy(TempData, AddrTemp, 7);
 	TempData[7] = ProtocoalInfo.Length-3;
	_ProData.MsgLength		= ProtocoalInfo.Length-3+8;  
	}
	else
    /*end:yangfei added 2013-08-05 for add HYDROMETER*/
	{
		for(i=0; i<RetryTimes; i++)
 		{
 		Err = METER_MeterCommunicate(&ProtocoalInfo, &TempData[8], &TempLen);
 		if(Err == NO_ERR)
 			{
 				Err = METER_DELU_AnalDataFrame(&ProtocoalInfo, &TempData[8]);
 				if(Err == NO_ERR)
 					{
 					       CJ188_Format CJ188_Data;
 						memcpy(TempData, AddrTemp, 7);
 						TempData[7] = ProtocoalInfo.Length-3;
						/*begin:yangfei added 2013-03-27 for CJ188 format*/
						#if 0
 						memcpy(&TempData[8], ProtocoalInfo.DataBuf, ProtocoalInfo.Length-3);
						#else
						CJ188_Data = METER_Data_To_CJ188Format(ProtocolVer,ProtocoalInfo.DataBuf,ProtocoalInfo.Length-3,&err);
						memcpy(&TempData[8], &CJ188_Data, ProtocoalInfo.Length-3);
						#endif
						/*end:yangfei added 2013-03-27 for CJ188 format*/
 						break;
 					}
 			}
 		}
		_ProData.MsgLength		= ProtocoalInfo.Length-3+8;
	}
	
	OSSemPost(METERChangeChannelSem);
	if(Err != NO_ERR)
		{
ERROR:	    debug("%s:read one meter fail\r\n",__FUNCTION__);
	           _ProData.MsgLength		= 1;
			*_ProData.Data.pDataBack= 0x10;					//Òì³£»ØÓ¦
			Err = 0;
		}
	else
		{
			memcpy(_ProData.Data.pDataBack, TempData, ProtocoalInfo.Length-3+8);
		}
	_ProData.MsgType		= 0x05;
	ReadDateTime(_ProData.TimeSmybol);
	//DISABLE_MBUS();   ÊµÊ±³­±íÊ±ÎªÁËÌá¸ßËÙ¶È£¬³­Íê1Ö»²»·Å¿ªÍ¨µÀ¡£
	
	return Err;
	
}

/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_06				    							                       
**	º¯ Êý  ¹¦ ÄÜ: ¸ù¾Ý¼ÆÁ¿µã³­±í¡¢·§ÐÅÏ¢²¢·µ»Ø¡£                                                                    			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	       
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢: 	                                                                                   																									
*********************************************************************************************************/

uint8 DEAL_ProcessMsg_06( _ProtocolType &_ProData)
{
	uint8  Err=0;
	uint8 *pTemp = _ProData.Data.pData;
	uint16 lu16MeterID	= 0x00;
	uint16 lu16MeterSn = 0;
	uint8 TempData[128] = {0x00};
	uint8 lu8MBusChannelBusy = 0;//MBUSÍ¨µÀÊÇ·ñÃ¦±êÖ¾£¬0-²»Ã¦£¬1-Ã¦¡£
	MeterFileType	mf;
	
	lu16MeterID = *((uint16 *)pTemp);			//»ñÈ¡MeterID.
	pTemp += 2;

	
	/*begin:yangfei added 2013-11-17 for ¶¨µã³­±í±êÖ¾£¬¶¨µã³­±íµÄÊ±ºò²»ÔÊÐíÊµÊ±³­±í£¬·ÀÖ¹Í¨µÀÇÐ»»*/
	OSSemPend(METERChangeChannelSem, OS_TICKS_PER_SEC, &Err); //ÉêÇëMBUSÍ¨µÀÇÐ»»È¨Àû
	if(Err != OS_ERR_NONE){
		lu8MBusChannelBusy = 1;//±ê¼ÇMBUSÍ¨µÀ±»Õ¼ÓÃ£¬Ã¦¡£
		debug("%s:nwo timing  read meter!\r\n",__FUNCTION__);
		goto POSITION_1;
	}
	/*end:yangfei added 2013-11-17 for ¶¨µã³­±í±êÖ¾£¬¶¨µã³­±íµÄÊ±ºò²»ÔÊÐíÊµÊ±³­±í£¬·ÀÖ¹Í¨µÀÇÐ»»*/

	//¸ù¾Ýlu16MeterID ²éÑ¯±íÐÅÏ¢¡£
	Err = PARA_GetMeterSn_ByMeterID(&lu16MeterID,&lu16MeterSn);

	if(Err == NO_ERR){
		PARA_ReadMeterInfo(lu16MeterSn, &mf);
		METER_ChangeChannel(mf.ChannelIndex);  //ÇÐ»»µ½¶ÔÓ¦Í¨µÀ

		if(mf.EquipmentType == HEAT_METER_TYPE)  //Èç¹ûÊÇÈÈÁ¿±í
			Err = METER_ReadMeterDataTiming(lu16MeterSn, TempData);
		else	//·ñÔò¾ÍÈÏÎªÊÇ·§¿Ø¡£
			Err = VALVE_ReadMeterDataTiming(lu16MeterSn, TempData);

		if(Err == NO_ERR){
			_ProData.MsgLength		= TempData[0];
			memcpy(_ProData.Data.pDataBack, &TempData[1], TempData[0]);
		}
		else{
			_ProData.MsgLength		= 1;
			*_ProData.Data.pDataBack = 0x10; 				//Òì³£»ØÓ¦
		}
		

		OSSemPost(METERChangeChannelSem);
		
	}
	else{  //²éÑ¯²»µ½±í»ù´¡ÐÅÏ¢¡£
		OSSemPost(METERChangeChannelSem);
		debug("%s:get one meter infomation fail\r\n",__FUNCTION__);
		_ProData.MsgLength		= 1;
		*_ProData.Data.pDataBack= 0x11; 				//Òì³£»ØÓ¦,²éÑ¯²»µ½±íÐÅÏ¢¡£
		Err = 0;

		_ProData.MsgType		= 0x07;
		ReadDateTime(_ProData.TimeSmybol);
		
		return Err;
	}


	
POSITION_1:		
	if(lu8MBusChannelBusy == 1){
		_ProData.MsgLength		= 1;
		*_ProData.Data.pDataBack = 0x12; //Òì³£»ØÓ¦£¬MBUSÍ¨µÀÕýÃ¦¡£
	}
				
	Err = 0;
	
	_ProData.MsgType		= 0x07;
	ReadDateTime(_ProData.TimeSmybol);
	//DISABLE_MBUS();  ÊµÊ±³­±íÊ±ÎªÁËÌá¸ßËÙ¶È£¬³­Íê1Ö»²»·Å¿ªÍ¨µÀ¡£
		
	return Err;

	
}

/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_08				    							                   
**	º¯ Êý  ¹¦ ÄÜ: ×ª·¢Ð­ÒéÐÅÏ¢                                                                         			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	       
**	Êä ³ö  ²Î Êý: none											                                      
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢: ´Ë´¦Í¸Ã÷×ª·¢Êý¾Ý£¬ÀíÂÛÉÏÓ¦¸ÃÊÇ·¢³öÊý¾Ý£¬³¬Ê±µÈ´ý£¬								   
**                ½ÓÊÕµ½Êý¾Ýºó£¬Ö±½ÓÔÙÉÏ´«ÉÏÈ¥¼´¿É£¬²»¶Ô½ÓÊÕµÄÊý¾Ý½øÐÐÅÐ¶Ï							   
**                ÏÖÓÐ×ö·¨£¬ÈÔ¶Ô½ÓÊÕµ½µÄÊý¾Ý½øÐÐÈÈ¼ÆÁ¿±íÐ­ÒéÊý¾ÝµÄÅÐ¶Ï	                               																									
*********************************************************************************************************/
//uint8 DEAL_ProcessMsg_08(_ProtocolType &_ProData)
uint8 DEAL_ProcessMsg_31(_ProtocolType &_ProData)
{
	uint8 Err = 0x00;
	uint8 TempData[128]	= {0x00};
	uint8 TempLen		= 0x00;
	
	TempLen = _ProData.MsgLength;
	memcpy(TempData, _ProData.Data.pData, TempLen);
	
	Err = METER_MeterCommunicate_Direct(TempData, TempLen, TempData, &TempLen);
	
	_ProData.MsgLength		= TempLen;
	_ProData.MsgType		= 0x09;
	ReadDateTime(_ProData.TimeSmybol);
	
	if(Err != NO_ERR)
		{
			_ProData.MsgLength		= 1;
			*_ProData.Data.pDataBack= 0x10;					//Òì³£»ØÓ¦
			Err = 0;
		}
	else
		{
			memcpy(_ProData.Data.pDataBack, TempData, TempLen);
		}
	
	return Err;	
}


/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_0E				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Ö÷Õ¾¶Á¼¯ÖÐÆ÷ÏÂËùÓÐÒÇ±íÊý¾Ý£¨×î½üÒ»´ÎÊý¾Ý£©                                                                      			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢: SD¿¨ÖÐÃ¿¸ö±íµÄÊý¾ÝÕ¼ÓÃ128×Ö½ÚµÄ¿Õ¼ä												   
**                ÆäÖÐµÚÒ»¸ö×Ö½ÚÎªÊý¾Ý³¤¶È(1×Ö½Ú) + Êý¾Ý(N+11) + CSÀÛ¼ÓºÍÐ£Ñé(1×Ö½Ú)
**		         ÆäÖÐÊý¾Ý¸ñÊ½Îª: ÈÈ¼ÆÁ¿±íµØÖ·(7×Ö½Ú) + ÈÈÁ¿Êý¾Ý³¤¶È(1×Ö½Ú) + ÈÈÁ¿Êý¾Ý(N) + ÎÂ¶ÈÊý¾Ý(3×Ö½Ú)
**		         Èô¶ÁÈ¡µ½µÃÊý¾Ý²»ÕýÈ·£¬»òÃ»ÓÐ¶ÁÈ¡µ½Êý¾Ý£¬Ôò¹æ¶¨ÈÈÁ¿Êý¾Ý³¤¶ÈÎª0£¬ÎÞÈÈÁ¿Êý¾Ý¡£ 	                                                                                   **																									
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_0E(_ProtocolType &_ProData, uint8 *ControlSave)
{
	uint8 Err					= 0x00;
	uint8 Res					= 0x00;
	uint8 Cs					= 0x00;
	uint8 *pTemp = _ProData.Data.pDataBack;
	uint8 *pFirst				= pTemp;
	uint8 ReadTime[6] = {0x00};
	uint8 MeterDataTemp[1024] 	= {0x00};					//ÔÝÊ±´æ´¢SD¿¨ÖÐÊý¾Ý
	uint8 *pDataTemp 			= MeterDataTemp;
	uint8 MeterFrameLen			= 0x00;						//¼ìÑé¶ÁÈ¡µ½µÄÊý¾ÝÖ¡ÕýÈ·ÐÔ£¬²¢·µ»ØÊý¾ÝÖ¡×Ö½ÚÊý
	uint8 DataCounter			= 0x00;
	uint8 DataErrCounter		= 0x00;
	uint8 SendReadyFalg 		= 0x00;
	uint16 StartMeterSn			= 0x00;
	uint16 MeterSnMax			= 0x00;
	uint16 FrameDataFileLenMax	= 0x00;						//»ñÈ¡µ±Ç°ÉèÖÃµÄ×î´óÊý¾Ý°üµÄÊý¾ÝÓò×î´ó×Ö½ÚÊý
	uint16 DataBackLen			= 0x00;
	//uint32 LogicAddr			= LOGIC_ADDR_NULL;
	/*begin:yangfei modified 2012-12-24 find path*/
	char  path[] = "/2012/12/24/1530";
	/*end   :yangfei modified 2012-12-24*/
	MultiFrameMsg_0E *pMultiFrame = NULL;
	pMultiFrame = (MultiFrameMsg_0E *)ControlSave;
	CPU_SR		cpu_sr;
	
	LOG_assert_param( ControlSave == NULL );

	if(ControlSave == NULL)		while(1);
	/*begin:yangfei modified 2012-12-24 for get time*/
	OS_ENTER_CRITICAL();
	memcpy(ReadTime,gSystemTime,sizeof(gSystemTime));
	OS_EXIT_CRITICAL();
	/*end   :yangfei modified 2012-12-24*/
	//²éÕÒµ±Ç°Ê±¼äÏÂ×î½üÒ»´Î³­±íÍê³ÉµÄÊ±¼ä£¬²»¿çÌì
	
	//»ñÈ¡µ±Ç°Êý¾ÝÓò×î´ó×Ö½ÚÊý
	//»ñÈ¡±íµµ°¸ÖÐ±íÐòºÅµÄ×î´óÊý, ÓÉÓÚÊÇÊµÊ±Êý¾Ý£¬Ö±½Ó¶ÁÈ¡ÄÚ´æ±¸·Ý¼´¿É
	OS_ENTER_CRITICAL();
	FrameDataFileLenMax = gPARA_TermPara.DebugPara.FrameLen_Max;
	MeterSnMax = gPARA_MeterNum;
	OS_EXIT_CRITICAL();
	
	//»ñÈ¡ÉÏ´ÎÉÏ´«µ½ÄÄ¸ö±íÐòºÅ
	StartMeterSn = pMultiFrame->StartMeterSn;

    pFirst = pTemp++;												//ÊÇ·ñÓÐºóÐøÖ¡±êÖ¾£¬×éÖ¡Íêºó²Å¿ÉÒÔÌîÐ´
    DataBackLen++;
    *pTemp++ = pMultiFrame->SendPackIndex;					//µ±Ç°·¢ËÍµÄ°üÐòºÅ
    DataBackLen++;
	/*begin:yangfei modified 2012-12-24 find crruent file path*/
	GetTimeNodeFilePath(path,ReadTime,gPARA_TimeNodes);
	/*end   :yangfei modified 2012-12-24*/
    while(!SendReadyFalg)
    {
    	//ReadTime[1] = 0x30;
		//ReadTime[2] = 0x04;
		/*begin:yangfei modified 2012-12-24 */
		OSMutexPend (FlashMutex, 0, &Err);
		Res = SDReadData(path, MeterDataTemp, sizeof(MeterDataTemp),StartMeterSn*128); 
		debug_err(gDebugModule[METER_DATA],"%s %d StartMeterSn =%d MeterSnMax=%d!\r\n",__FUNCTION__,__LINE__,StartMeterSn,MeterSnMax);
    	       OSMutexPost (FlashMutex);
		/*end   :yangfei modified 2012-12-24*/
    	if(Res!=NO_ERR)				
		{
		debug("%s %d SDReadData err=%d!\r\n",__FUNCTION__,__LINE__,Err);
		goto ERROR;
		}
		DataCounter = 0;
		pDataTemp =  MeterDataTemp;

    	while(DataCounter< (1024/METER_FRAME_LEN_MAX))
    	{
			MeterFrameLen = *pDataTemp++;
			//´Ë´ÎÒªÅÐ¶Ï³¤¶ÈÊÇ·ñÎªÓÐÐ§
        	
    		if( (DataBackLen+MeterFrameLen) > FrameDataFileLenMax )
    			{						
    				SendReadyFalg = 1;
    				break;										//Ô¤´¦Àí½øÐÐÅÐ¶Ï³¤¶ÈÊÇ·ñ³¬¹ýÏÞÖÆ
    			}
    		else
    			{
    				if(StartMeterSn < MeterSnMax)				//ÅÐ¶ÏÊÇ·ñ»¹ÓÐ±íÃ»´«Íê
    					{
    						/*begin :yangfei modified 2013-08-29 for µ±×îºóÒ»°üµ½Ê±ÎÞÏÂÒ»°üÊý¾Ý,·ÀÖ¹Ö÷»ú²»Í£µÄÒªÊý¾Ý*/
                             if(StartMeterSn+1==MeterSnMax)
                             	{
                             	debug_info(gDebugModule[METER_DATA],"StartMeterSn+1==MeterSnMax\r\n");
							    *pFirst = 0x00;	
                             	}
					         else
					         	{
					         	*pFirst = 0x01;						//ÈÔÓÐÊý¾ÝÐèÒª´«Êä
					         	}
					       /*end :yangfei modified 2013-08-29 for µ±×îºóÒ»°üµ½Ê±ÎÞÏÂÒ»°üÊý¾Ý*/	
    						
    						Cs = *(pDataTemp+MeterFrameLen);
    						if( Cs == PUBLIC_CountCS(pDataTemp, MeterFrameLen) )
    							{
    								if( (MeterFrameLen>0x00) && (MeterFrameLen<0x80) )
    									{
    										memcpy(pTemp, pDataTemp, MeterFrameLen);
    										pTemp 		+= MeterFrameLen;
    										DataBackLen += MeterFrameLen;
    									}
								else
    									{
    										DataErrCounter++;
    									}
    							}
    						else
    							{
    								DataErrCounter++;
    							}
							DataCounter++;
    						pDataTemp 	= &MeterDataTemp[DataCounter*METER_FRAME_LEN_MAX];
    						StartMeterSn++;
    					}
    				else
    					{
    						*pFirst = 0x00;
    						SendReadyFalg = 1;
    						break;
    					}
    			}
    	}
    }
    pMultiFrame->MultiFlag 		= *pFirst;
    pMultiFrame->SendMeterNums 	= StartMeterSn - (pMultiFrame->StartMeterSn);
    
    //¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
	_ProData.MsgLength		= DataBackLen;
	_ProData.MsgType		= 0x10;
	ReadDateTime(_ProData.TimeSmybol);
	
	if((DataErrCounter == pMultiFrame->SendMeterNums)&&(DataErrCounter!=0))							//Èç¹ûÊý¾ÝÈ«²¿²»ÕýÈ·
		{
			*_ProData.Data.pDataBack = 0x10;					//Òì³£»ØÓ¦
			goto ERROR;
		}
	
	return NO_ERR;
	
ERROR:
	_ProData.MsgLength		= 1;
	_ProData.MsgType		= 0x10;
	*_ProData.Data.pDataBack = 0x10;					//Òì³£»ØÓ¦

	return ERR_1;
}


/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_18				    							                   
**	º¯ Êý  ¹¦ ÄÜ: ×ª·¢Ð­ÒéÐÅÏ¢                                                                         			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢: ¶Ô·§ÃÅºÍÎÂ¿ØÃæ°åµÄ²Ù×÷£¬»¹ÓÐ´ýÐ­Òé¶©ÐÍ
** 	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_18(_ProtocolType &_ProData)
{
	uint8 Err			= 0x00;
	uint8 ForAllOrOne	= 0x00;
	uint8 *pTemp = _ProData.Data.pData;
	
	ForAllOrOne	= *pTemp++;
	
	
	if(0x0A == ForAllOrOne)
		{
			//Err = DEAL_ProcessMsg_18_ForAll(_ProData);
		}
	else if(0x0B == ForAllOrOne)
		{
			Err = DEAL_ProcessMsg_18_ForOne(_ProData);
		}
	else
		{
			//Èº·¢ µ¥·¢ ±êÖ¾´íÎó
		}
	
	return Err;
}


/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_18_ForOne				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Õë¶Ôµ¥·¢µÄ×ª·¢Ð­ÒéÐÅÏ¢£¬²Ù×÷¼¯ÖÐÆ÷ÏÂµÄÎÂ¿Ø¿ØÖÆ                                                                         			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_18_ForOne(_ProtocolType &_ProData)
{
	uint8 Err			= 0x00;
	uint8 MsgType		= 0x00;
	uint8 DataBackLen	= 0x00;
	//uint8 HeatMeterAddr[7] = {0x00};
	uint8 *pTemp = _ProData.Data.pData;
	uint8 *pTempBack = _ProData.Data.pDataBack;
	uint8 *pTempAddr = pTemp;
	uint16 MeterSn		= 0x00;
	MeterFileType   mf;
	uint8 DataFrame[METER_FRAME_LEN_MAX];
	uint8 DataLen_Vave = 0x00;
	uint8 lu8DataIn[20] = {0};  //ÓÃÓÚ´æ´¢º¯Êý´«µÝ²ÎÊý¡£
	uint16 lu16MeterID = 0;

	*pTempBack++ = *pTemp++;
	DataBackLen++;

	MsgType		= *pTemp++;
	
	//´Ë´¦ÐèÅÐ¶Ïµ¥·¢ºÍÈº·¢±êÖ¾ ÊÇ·ñÕýÈ·
	
	*pTempBack++	= MsgType;
	DataBackLen++;

	OSSemPend(METERChangeChannelSem, OS_TICKS_PER_SEC, &Err); //ÉêÇëMBUSÍ¨µÀÇÐ»»È¨Àû
	if(Err != OS_ERR_NONE)
	 	goto MBUS_Busy;
	
	
	switch(MsgType)
	{
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:{
				pTempAddr = pTemp;
				break;
		}
		case 0x0E:{
				pTempAddr = pTemp + 3;
				break;
		}
		case 0x0F:{
				pTempAddr = pTemp + 6;
				break;
		}
		case 0x10:{
			pTempAddr = pTemp + 9;
			break;
		}
		case 0x11:{
			pTempAddr = pTemp + 3;
			break;
		}
		case 0x12:
		case 0x13:
		case 0x14:{
			pTempAddr = pTemp + 1;
			break;
		}
		
		default:
			break;
	}
	//memcpy(HeatMeterAddr, pTempAddr, 7);
	//Err = PARA_GetMeterSn(HeatMeterAddr, &MeterSn);
	lu16MeterID = *((uint16 *)pTempAddr);
	Err = PARA_GetMeterSn_ByMeterID(&lu16MeterID,&MeterSn);  //ÒÀ¿¿MeterID²éÕÒ±í·§»ù´¡ÐÅÏ¢¡

	if(Err!=NO_ERR)			
		{
		*pTempBack = 0x10;
		DataBackLen++;
		//return ERR_1;			//±íµØÖ·Ã»ÓÐ²éÕÒµ½·µ»Ø´íÎó
		goto READ_FAIL;
		}
	PARA_ReadMeterInfo(MeterSn, &mf);
	//ÐèÅÐ¶Ï¿ØÖÆÃæ°åµØÖ·ÊÇ·ñÕýÈ·ÓÐÐ§
	METER_ChangeChannel(mf.ChannelIndex);
	
	switch(MsgType)
	{
			case 0x09:									//ÉèÖÃÎÂ¿Ø¿ØÖÆÀàÐÍ
			case 0x0B:
			case 0x0C:
			case 0x0D:{
				lu8DataIn[0] = MsgType;
				Err = ValveContron(&mf,SETVALVE_CONTROLTYPE,lu8DataIn,DataFrame,&DataLen_Vave);
				
				if(Err == NO_ERR){
					*pTempBack = 0x01;
					DataBackLen++;
				}
				else{
					*pTempBack = 0x10;
					DataBackLen++;
						
				}

			}

			break;
			
		case 0x0A:									//³­ÊÒÄÚÊµ¼ÊÎÂ¶È
			{
				if(mf.ValveProtocal == VALVE_ADE){
					Err = ValveContron(&mf,READ_VALVEDATA,lu8DataIn,DataFrame,&DataLen_Vave);
					
					if(Err == NO_ERR){
						*pTempBack++ = DataFrame[20];  //·ûºÅÎ»
						DataBackLen++;
						*pTempBack++ = DataFrame[21];
						DataBackLen++;
						*pTempBack++ = DataFrame[22];  //Ð¡ÊýÎ»¹Ì¶¨Îª0.
						DataBackLen++;
						
						debug_info(gDebugModule[TASKDOWN_MODULE],"%s Read indoor temperature ok ",__FUNCTION__);
					}
					else{
						*pTempBack = 0x10;
						DataBackLen++;
						debug_err(gDebugModule[TASKDOWN_MODULE],"%s Read indoor temperature failed ",__FUNCTION__);
					}

				}
				else{
					Err = ValveContron(&mf,ReadVALVE_All,lu8DataIn,DataFrame,&DataLen_Vave);
					
					if(Err == NO_ERR){				
						*pTempBack++ = 0; 	//Ð¡ÊýÎ»¹Ì¶¨Îª0.
						DataBackLen++;
						*pTempBack++ = DataFrame[8];
						DataBackLen++;
						*pTempBack++ = 0;    //·ûºÅÎ»
						DataBackLen++;

						debug_info(gDebugModule[TASKDOWN_MODULE],"%s Read indoor temperature ok ",__FUNCTION__);
					}
					else{
						*pTempBack = 0x10;
						DataBackLen++;
						debug_err(gDebugModule[TASKDOWN_MODULE],"%s Read indoor temperature failed ",__FUNCTION__);
					}
				}

			}

			break;
		
		case 0x0E:							 /*ÉèÖÃÊÒÄÚÉè¶¨ÎÂ¶È*/
				{
					memcpy(lu8DataIn, pTemp, 3);
					Err = ValveContron(&mf,SETROOM_TEMP,lu8DataIn,DataFrame,&DataLen_Vave);

					if(Err == NO_ERR){
						*pTempBack = 0x01;
						DataBackLen++;
						debug_info(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature ok ",__FUNCTION__);
					}
					else{
						*pTempBack = 0x10;
						DataBackLen++;
						debug_err(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature failed ",__FUNCTION__);
					}
				}
				break;

		case 0x0F:                          /*ÉèÖÃÊÒÄÚÉÏÏÂÏÞÎÂ¶È*/
			{
				memcpy(lu8DataIn, pTemp, 6);
				Err = ValveContron(&mf,SETTEMP_RANGE,lu8DataIn,DataFrame,&DataLen_Vave);
				
				if(Err == NO_ERR){
					*pTempBack = 0x01;
					DataBackLen++;
					debug_info(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature ok ",__FUNCTION__);
				}
				else{
					*pTempBack = 0x10;
					DataBackLen++;
					debug_err(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature failed ",__FUNCTION__);
				}
			}

			break;

		case 0x10:{
				memcpy(lu8DataIn, pTemp, 9);
				Err = ValveContron(&mf,SETTEMP_ANDRANGE,lu8DataIn,DataFrame,&DataLen_Vave);
				
				if(Err == NO_ERR){
					*pTempBack = 0x01;
					DataBackLen++;
					debug_info(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature ok ",__FUNCTION__);
				}
				else{
					*pTempBack = 0x10;
					DataBackLen++;
					debug_err(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature failed ",__FUNCTION__);
				}

				break;
			}

		case 0x11:{
			memcpy(lu8DataIn, pTemp, 3);
			Err = ValveContron(&mf,SETVALVE_OFFSETTEMP,lu8DataIn,DataFrame,&DataLen_Vave);
			
			if(Err == NO_ERR){
				*pTempBack = 0x01;
				DataBackLen++;
			}
			else{
				*pTempBack = 0x10;
				DataBackLen++;
			}

			break;
		}
		
		case 0x12:{
			memcpy(lu8DataIn, pTemp, 1);
			Err = ValveContron(&mf,SETVALVE_HYSTEMP,lu8DataIn,DataFrame,&DataLen_Vave);
			
			if(Err == NO_ERR){
				*pTempBack = 0x01;
				DataBackLen++;
			}
			else{
				*pTempBack = 0x10;
				DataBackLen++;
			}

			break;
		}
		
		case 0x13:{
			memcpy(lu8DataIn, pTemp, 1);
			Err = ValveContron(&mf,SETTEMP_GATHERPERIOD,lu8DataIn,DataFrame,&DataLen_Vave);
			
			if(Err == NO_ERR){
				*pTempBack = 0x01;
				DataBackLen++;
			}
			else{
				*pTempBack = 0x10;
				DataBackLen++;
			}

			break;
		}
		
		case 0x14:{
			memcpy(lu8DataIn, pTemp, 1);
			Err = ValveContron(&mf,SETTEMP_UPLOADPERIOD,lu8DataIn,DataFrame,&DataLen_Vave);
			
			if(Err == NO_ERR){
				*pTempBack = 0x01;
				DataBackLen++;
			}
			else{
				*pTempBack = 0x10;
				DataBackLen++;
			}

			break;
		}
		
		default:
			break;
	}
	OSSemPost(METERChangeChannelSem);
	
READ_FAIL:
	//»Ø¸´Êý¾ÝÖ¡
	_ProData.MsgLength	= DataBackLen;
	_ProData.MsgType	= 0x19;
	ReadDateTime(_ProData.TimeSmybol);
	//DISABLE_MBUS();  ÊµÊ±³­±íÊ±ÎªÁËÌá¸ßËÙ¶È£¬³­Íê1Ö»²»·Å¿ªÍ¨µÀ¡£
	
	return Err;

MBUS_Busy:
	*pTempBack = 0x12;  //MBUSÍ¨µÀÕýÃ¦¡£
	DataBackLen++;
	_ProData.MsgLength	= DataBackLen;
	_ProData.MsgType	= 0x19;
	ReadDateTime(_ProData.TimeSmybol);
		
	return Err;

}


/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_18_ForAll				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Õë¶ÔÈº·¢µÄ×ª·¢Ð­ÒéÐÅÏ¢                                                                         			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_18_ForAll(_ProtocolType &_ProData)
{
	uint8 Err			= 0x00;
	uint8 ForAllOrOne	= 0x00;
	uint8 MsgType		= 0x00;
	uint8 DataBackLen	= 0x00;
	uint8 *pTemp = _ProData.Data.pData;
	uint8 *pTempBack = _ProData.Data.pDataBack;
	
	ForAllOrOne	= *pTemp++;
	MsgType		= *pTemp++;
	
	//´Ë´¦ÐèÅÐ¶Ïµ¥·¢ºÍÈº·¢±êÖ¾ ÊÇ·ñÕýÈ·
	
	*pTempBack++	= ForAllOrOne;
	*pTempBack++	= MsgType;
	DataBackLen 	+= 2;
	
	*pTempBack++	= 0x01;						//Èº·¢Ö±½Ó¸øÕýÈ·ÏìÓ¦ ¼´¿É
	//·¢ËÍÐÅÏ¢¸øÖ´ÐÐÈÎÎñÀ´£¬¿ªÊ¼³­¶Á»ò¿ØÖÆÈ«ÌåÈÈÁ¦±í£¬ÎÂ¿ØÃæ°å£¬·§ÃÅµÈ£»
	memset(gDEAL_ReadMailBox_Msg18, 0x00, sizeof(gDEAL_ReadMailBox_Msg18));
	gDEAL_ReadMailBox_Msg18[0] = MSG_18;
	gDEAL_ReadMailBox_Msg18[1] = MsgType;
	memcpy(&gDEAL_ReadMailBox_Msg18[2], pTemp, _ProData.MsgLength-2);
	
	OSQPost(CMD_Q, gDEAL_ReadMailBox_Msg18);
	
	//»Ø¸´Êý¾ÝÖ¡
	_ProData.MsgLength	= DataBackLen+1;
	_ProData.MsgType	= 0x19;
	ReadDateTime(_ProData.TimeSmybol);
	
	return Err;
}

/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_MSG18_ForAll				    							                   
**	º¯ Êý  ¹¦ ÄÜ: ¶ÁÈ¡ËùÓÐ¼ÆÁ¿±íµÄÐÅÏ¢                                                                         			
**	Êä Èë  ²Î Êý: _ProtocolType &_ProData   							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
void DEAL_MSG18_ForAll(uint8 *pData)
{
	uint8 i,j,Err;
	uint8 MsgType	= 0x00;
	uint8 *pTemp = pData;
	uint8 Res		= 0x00;
	uint16 TempArray[METER_PER_CHANNEL_NUM] = {0x00};
	uint8 TempNums	= 0x00;
	uint16 MeterNums= 0x00;
	uint16 MeterCmplete	= 0x00;
	uint16 MeterFailure = 0x00;
	uint16 SaveCounter  = 0x00;
	uint16 DataTempLen	= 0x00;
	uint8 paratemp[20]  = {0x00};
	uint8 paralen		= 0x00;
	uint32 LogicAddr	= METER_DATA_TEMP;
	DELU_Protocol	ReadData;
	MeterFileType   mf;
	CPU_SR			cpu_sr;
	
	MsgType = *pTemp++;
	LogicAddr += 4;							//4¸ö×Ö½ÚÔ¤Áô¸ø  ÏûÏ¢ÀàÐÍ(1×Ö½Ú) + Ö¸ÁîÀàÐÍ(1×Ö½Ú) + ×Ü×Ö½ÚÊý(2×Ö½Ú)
	if(MsgType!=0x0A)	LogicAddr += 6;		//6¸ö×Ö½ÚÔ¤Áô¸ø  ²Ù×÷Éè±¸×ÜÊý(2Byte)+³É¹¦Éè±¸Êý(2Byte)+Ê§°ÜÉè±¸Êý(2Byte)
	//¶ÔÎÂ¿ØÃæ°åµÄ²Ù×÷
	for(i=0; i<6; i++)
	{
		//ÇÐ»»MbusÍ¨µÀ
		METER_ChangeChannel(i+1);		//Í¨µÀºÅ ´Ó 1¿ªÊ¼
		
		OS_ENTER_CRITICAL();
		memcpy((uint8 *)TempArray, (uint8 *)gPARA_ConPanelChannel, gPARA_ConPanelChannelNum[i]*sizeof(uint16));
		TempNums = gPARA_ConPanelChannelNum[i];
		OS_EXIT_CRITICAL();
		MeterNums += TempNums;
		
		for(j=0; j<TempNums; j++)
		{
			PARA_ReadMeterInfo(TempArray[j], &mf);
			ReadData.MeterType 		= 0x20;
			ReadData.ControlCode 	= 0x04;
			ReadData.Length 		= 0x0B;
			ReadData.DataIdentifier = 0x22A0;
			memcpy(ReadData.DataBuf, mf.ControlPanelAddr, 7);
			switch(MsgType)
				{
					case 0x0A:
						{
							//×é½¨×Ô½¨Ð­Òé£¬¶ÁÎÂ¿ØÃæ°åÎÂ¶È
							ReadData.DataIdentifier = 0x20A0;
							ReadData.DataBuf[7]		= 0x11;
							break;
						}
					case 0x09:
						{
							//×é½¨×Ô½¨Ð­Òé£¬ÉèÖÃÊ¹ÄÜ×´Ì¬
							ReadData.DataBuf[7]		= 0x55;
							break;
						}
					case 0x0B:
						{
							//×é½¨×Ô½¨Ð­Òé£¬ÉèÖÃ½ûÓÃ×´Ì¬
							ReadData.DataBuf[7]		= 0x66;
							break;
						}
					case 0x0C:
						{
							//×é½¨×Ô½¨Ð­Òé£¬ÉèÖÃ×Ô¶¯×´Ì¬
							ReadData.DataBuf[7]		= 0x77;
							break;
						}
					case 0x0D:
						{
							//×é½¨×Ô½¨Ð­Òé£¬ÉèÖÃ¶¨Ê±×´Ì¬
							ReadData.DataBuf[7]		= 0x88;
							break;
						}
					case 0x0E:
						{
							//×é½¨×Ô½¨Ð­Òé£¬ÉèÖÃÊÒÄÚÉè¶¨ÎÂ¶È
							ReadData.DataIdentifier = 0x23A0;
							memcpy(&ReadData.DataBuf[7], pTemp, 3);
							break;
						}
					case 0x0F:
						{
							//×é½¨×Ô½¨Ð­Òé£¬ÉèÖÃÊÒÄÚÉè¶¨ÎÂ¶È
							ReadData.DataIdentifier = 0x21A0;
							memcpy(&ReadData.DataBuf[7], pTemp, 6);
							break;
						}
					default:
						break;
				}
			
			Res = METER_ReadMeterDataCur(&ReadData, TempArray[j]);
//
			if(MsgType == 0x0A)
				{
					if(Res)
						{
							memcpy(&gDEAL_ReadDataTemp[10*j], mf.MeterAddr, 7);
							memset(&gDEAL_ReadDataTemp[10*j+7], 0xEE, 3);
							MeterFailure++;
						}
					else
						{
							memcpy(&gDEAL_ReadDataTemp[10*j], mf.MeterAddr, 7);
							memcpy(&gDEAL_ReadDataTemp[10*j+7], ReadData.DataBuf, 3);
							MeterCmplete++;
						}
					SaveCounter += 10;
				}
			else			//ÉèÖÃ¿ØÖÆÀàÖ¸Áî£¬ Ö»½øÐÐ¼ÇÂ¼Ê§°Ü±íµØÖ·
				{
					if(Res)
						{
							memcpy(&gDEAL_ReadDataTemp[7*j], mf.MeterAddr, 7);
							MeterFailure++;
							SaveCounter += 7;
						}
					else
						{
							MeterCmplete++;
						}
				}
		}
		OSMutexPend (FlashMutex,0,&Err);
		/*need modified*/
    	Err =AppSdWrRdBytes(LogicAddr, DataTempLen, gDEAL_ReadDataTemp, SD_WR);
    	OSMutexPost (FlashMutex);
		if(Err == true)		Err = NO_ERR;
			else
				{
					
				}
		LogicAddr += DataTempLen;
		
	}
	//¶ÔËùÓÐÉè±¸²Ù×÷Íê³Éºó£¬¸ù¾Ý³É¹¦ºÍÊ§°Ü¼ÆÊýÀ´×Ü½áÉè±¸²Ù×÷Çé¿ö
	LogicAddr	= METER_DATA_TEMP;
	paratemp[paralen++] = 0x18;
	paratemp[paralen++] = MsgType;
	if(MsgType == 0x0A)
		{
			paratemp[paralen++] = SaveCounter;
			paratemp[paralen++] = SaveCounter>>8;
		}
	else if( (MeterFailure==0x00) && (MeterCmplete!=0x00) )
		{														//Ê§°ÜÊýÎª0£¬Ôò±íÊ¾ËùÓÐ¶¼³É¹¦
			paratemp[paralen++] = 0x01;
			paratemp[paralen++] = 0x00;
			paratemp[paralen++] = 0x01;
		}
	else if((MeterCmplete==0x00) && (MeterFailure!=0x00) )
		{														//³É¹¦ÊýÎª0£¬Ôò±íÊ¾ËùÓÐ¶¼Ê§°Ü
			paratemp[paralen++] = 0x01;
			paratemp[paralen++] = 0x00;
			paratemp[paralen++] = 0x10;
		}
	else
		{
			paratemp[paralen++] = SaveCounter;
			paratemp[paralen++] = SaveCounter>>8;
			paratemp[paralen++] = (MeterFailure+MeterCmplete);
			paratemp[paralen++] = (MeterFailure+MeterCmplete)>>8;
			paratemp[paralen++] = MeterCmplete;
			paratemp[paralen++] = MeterCmplete>>8;
			paratemp[paralen++] = MeterFailure;
			paratemp[paralen++] = MeterFailure>>8;
		}
	OSMutexPend (FlashMutex,0,&Err);
	/*need modified*/
    Err =AppSdWrRdBytes(LogicAddr, paralen, paratemp, SD_WR);
    OSMutexPost (FlashMutex);
	if(Err)					;
	
}

/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_1A				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Ö÷Õ¾¶ÁÈ¡Èº·¢ÎÂ¶È¿ØÖÆÐÅÏ¢                                                                  			
**	Êä Èë  ²Î Êý: uint8 *ControlSave 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_1A(_ProtocolType &_ProData, uint8 *ControlSave)
{
	uint8 Err					= 0x00;
	uint8 Res					= 0x00;
	uint8 ComType				= 0x00;						//Ö¸ÁîÀàÐÍ
	//uint8 *pData = _ProData.Data.pData;						//Ö÷Õ¾ÏÂ·¢µÄÏûÏ¢ÌåÄÚÈÝ
	uint8 *pTemp = _ProData.Data.pDataBack;					//ÉÏ´«¸øÖ÷Õ¾µÄÏûÏ¢ÌåÄÚÈÝ
	uint8 *pFirst				= pTemp;
	uint8 MeterDataTemp[1024] 	= {0x00};					//ÔÝÊ±´æ´¢SD¿¨ÖÐÊý¾Ý
	uint8 *pDataTemp 			= MeterDataTemp;
	uint8 SendReadyFlag			= 0x00;
	uint8 LoopCounter			= 0x00;
	uint8 LoopMax				= 0x00;
	uint16 LoopLen				= 0x00;						//ÏûÏ¢ÌåÊý¾Ý×îÐ¡³¤¶È£¬ÓÃÓÚ¶àÖ¡´«Êä£¬²ð°ü²»´ò¶Ï»ù±¾Êý¾Ý³¤¶È
	uint16 FrameDataFileLenMax	= 0x00;						//»ñÈ¡µ±Ç°ÉèÖÃµÄ×î´óÊý¾Ý°üµÄÊý¾ÝÓò×î´ó×Ö½ÚÊý
	uint16 DataBackLen			= 0x00;
	uint16 DataStoreLen			= 0x00;
	uint32 StartLogicAddr		= 0x00;
	//uint32 EndLogicAddr			= 0x00;

	LOG_assert_param( ControlSave == NULL );

	MultiFrameMsg_1A *pMultiFrame = NULL;
	pMultiFrame = (MultiFrameMsg_1A *)ControlSave;
	
	FrameDataFileLenMax = 450;								//»ñÈ¡µ±Ç°Êý¾ÝÓò×î´ó×Ö½ÚÊý
	
	*pTemp++ = 0;											//ÊÇ·ñÓÐºóÐøÖ¡±êÖ¾£¬×éÖ¡Íêºó²Å¿ÉÒÔÌîÐ´
	*pTemp++ = pMultiFrame->SendPackIndex;
    ComType  = *_ProData.Data.pData;						//Ö¸ÁîÀàÐÍ
    DataBackLen += 2;
	
	//»ñÈ¡ÉÏ´«¿ªÊ¼µØÖ·
	if( (pMultiFrame->MultiFlag) == 0x00)
		{													//´«ÊäµÄµÚÒ»Ö¡£¬ÉÐ²»Çå³þÊÇ·ñÐèÒª¶àÖ¡´«Êä
			StartLogicAddr = METER_DATA_TEMP;
			OSMutexPend (FlashMutex, 0, &Err);
			/*need modified*/
    		Res = AppSdWrRdBytes(StartLogicAddr, 512, MeterDataTemp, SD_RD);
    		OSMutexPost (FlashMutex);
    		//Ðè¶ÔRes½øÐÐÅÐ¶Ï
			if(Res)				;

    		//Ðè¶Ô¶ÁÈ¡Êý¾ÝµÄÇ°N¸ö²ÎÊý½øÐÐÅÐ¶Ï
    		if(MeterDataTemp[0] != 0x1A)		;			//ÏûÏ¢ÀàÐÍ²»ÕýÈ·
    		if(MeterDataTemp[1] != ComType)		
			{		 //Ö¸ÁîÀàÐÍ²»ÕýÈ·
				*pTemp++ = 	MeterDataTemp[1];
				DataBackLen += 1;	
			}			
			else
			{
				*pTemp++ = 	ComType;
				DataBackLen += 1;
			}
    		
    		DataStoreLen = MeterDataTemp[2] | (MeterDataTemp[3]<<8);
    		pMultiFrame->EndReadAddr = pMultiFrame->StartReadAddr + DataStoreLen;
    		
    		if(DataStoreLen > 1)
    			{
    				if( ComType==0x0A )
    					{
    						StartLogicAddr = METER_DATA_TEMP;
    					}
    				else
    					{
    						StartLogicAddr = METER_DATA_TEMP+10;
    					}
    					
    				if( (ComType==0x0B) && (ComType==0x0C) && (ComType==0x0D) && (ComType==0x0E) && (ComType==0x0F) )
    					{												//Ö¸ÁîÀàÐÍÎªBCDEF, ¾ùÎª¿ØÖÆÉèÖÃÃüÁî
    						memcpy(pTemp, &MeterDataTemp[4], 6);		//µÚÒ»°ü½«²Ù×÷¸ÅÒªÐÅÏ¢ÉÏ´«
    						pTemp += 6;
    					}		
    			}
    		else
    			{	
    				//È«²¿³É¹¦ ºÍ È«²¿Ê§°ÜµÄÇé¿öÏÂ£¬Ö±½Ó·µ»Ø
    				*pTemp++ = MeterDataTemp[4];
					DataBackLen++;
    				//¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
					_ProData.MsgLength		= DataBackLen;
					_ProData.MsgType		= 0x1F;
					ReadDateTime(_ProData.TimeSmybol);
	
					return NO_ERR;
    			}
    		
		}
	else
		{
			if( ComType==0x0A )
    			{
    				StartLogicAddr = pMultiFrame->StartReadAddr + METER_DATA_TEMP+10;
    			}
    		else
    			{
    				StartLogicAddr = pMultiFrame->StartReadAddr + METER_DATA_TEMP;
    			}
		}
    
    if( (ComType==0x0B) && (ComType==0x0C) && (ComType==0x0D) && (ComType==0x0E) && (ComType==0x0F) )
    		{												//Ö¸ÁîÀàÐÍÎªBCDEF, ¾ùÎª¿ØÖÆÉèÖÃÃüÁî
    			LoopLen = 0x07;
    		}
    if(ComType==0x0A)
    	{
    		LoopLen	= 0x0A;
    	}

    while(!SendReadyFlag)
    {
    	OSMutexPend (FlashMutex, 0, &Err);
			/*need modified*/
    	Res = AppSdWrRdBytes(StartLogicAddr, 512, MeterDataTemp, SD_RD);
    	OSMutexPost (FlashMutex);
    	//Ðè¶ÔRes½øÐÐÅÐ¶Ï
		if(Res)				;
		LoopCounter = 0x00;
    	LoopMax		= 512 / LoopLen;
    	while(LoopCounter<LoopMax)
    	{
    		if( (DataBackLen+LoopLen) > FrameDataFileLenMax )
    			{	
    				SendReadyFlag = 1;					
    				break;										//Ô¤´¦Àí½øÐÐÅÐ¶Ï³¤¶ÈÊÇ·ñ³¬¹ýÏÞÖÆ
    			}
    		else
    			{
    				if( (StartLogicAddr+DataBackLen-3) < (pMultiFrame->EndReadAddr) )				//ÅÐ¶ÏÊÇ·ñ»¹ÓÐÊý¾ÝÃ»´«Íê
    					{
    						*pFirst = 0x01;						//ÈÔÓÐÊý¾ÝÐèÒª´«Êä
    						
    						memcpy(pTemp, pDataTemp, LoopLen);
    						pTemp 		+= LoopLen;
    						DataBackLen += LoopLen;
    						pDataTemp 	+= LoopLen;
        	
    					}
    				else
    					{
    						*pFirst = 0x00;
    						SendReadyFlag = 1;		
    						break;
    					}
    			}
    		LoopCounter++;
    	}
    }
    pMultiFrame->MultiFlag 		= *pFirst;
    pMultiFrame->SendByteNums 	= DataBackLen - 3;
    
    //¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
    if( ComType==0x0A )
    	{
    		_ProData.MsgLength		= DataBackLen+10;
    	}
    else
    	{
    		_ProData.MsgLength		= DataBackLen;
    	}
	_ProData.MsgType		= 0x1B;
	ReadDateTime(_ProData.TimeSmybol);
	
	return NO_ERR;
}

/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_1C				    							                   
**	º¯ Êý  ¹¦ ÄÜ: ²Ù×÷¼¯ÖÐÆ÷ÏÂµÄ·§ÃÅ¿ØÖÆ                                                                 			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_1C(_ProtocolType &_ProData)
{
	uint8 Err			= 0x00;
	uint8 ForAllOrOne	= 0x00;
	uint8 *pTemp = _ProData.Data.pData;
	
	ForAllOrOne	= *pTemp++;
	
	if(0x0A == ForAllOrOne)
		{
			//Err = DEAL_ProcessMsg_1C_ForAll(_ProData);
		}
	else if(0x0B == ForAllOrOne)
		{
			Err = DEAL_ProcessMsg_1C_ForOne(_ProData);
		}
	else
		{
			//Èº·¢ µ¥·¢ ±êÖ¾´íÎó
		}
	
	return Err;
}



/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_1C_ForOne				    							                   
**	º¯ Êý  ¹¦ ÄÜ: ¶ÔÓÚµ¥·¢µÄÊý¾Ý´¦Àí                                                                      			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_1C_ForOne(_ProtocolType &_ProData)
{
	uint8 Err			= 0x00;
	uint8 MsgType		= 0x00;
	uint8 DataBackLen	= 0x00;
	//uint8 HeatMeterAddr[7] = {0x00};
	uint8 *pTemp = _ProData.Data.pData;
	uint8 *pTempBack = _ProData.Data.pDataBack;
	uint16 MeterSn		= 0x00;
	MeterFileType	mf;
	uint16 lu16MeterID = 0;
		
	uint8 DataFrame[METER_FRAME_LEN_MAX];
	uint8 DataLen_Vave = 0x00;
	uint8 lu8DataIn[20] = {0};  //ÓÃÓÚ´æ´¢º¯Êý´«µÝ²ÎÊý¡£
		
//	uint16 ValveState		= 0x00;
	uint8 lu8workmode = 0;
	uint8 lu8tmp = 0;
	
		
	*pTempBack++	= *pTemp++;
	DataBackLen++;
	
	MsgType 	= *pTemp++;
	
		//´Ë´¦ÐèÅÐ¶Ïµ¥·¢ºÍÈº·¢±êÖ¾ ÊÇ·ñÕýÈ·
		
	*pTempBack++	= MsgType;
	DataBackLen++;

	OSSemPend(METERChangeChannelSem, OS_TICKS_PER_SEC, &Err); //ÉêÇëMBUSÍ¨µÀÇÐ»»È¨Àû
	if(Err != OS_ERR_NONE)
	 	goto MBUS_Busy;
	 	
		
	//memcpy(HeatMeterAddr, pTemp, 7);
	//Err = PARA_GetMeterSn(HeatMeterAddr, &MeterSn);
	lu16MeterID = *((uint16 *)pTemp);
	Err = PARA_GetMeterSn_ByMeterID(&lu16MeterID,&MeterSn);  //ÒÀ¿¿MeterID²éÕÒ±í·§»ù´¡ÐÅÏ¢¡£

	if(Err!=NO_ERR) 		
	{
		debugX(LOG_LEVEL_ERROR,"%s PARA_GetMeterSn err!\r\n",__FUNCTION__);
		//return ERR_1; 		//±íµØÖ·Ã»ÓÐ²éÕÒµ½·µ»Ø´í
		goto READ_FAIL;
	}
	PARA_ReadMeterInfo(MeterSn, &mf);
	METER_ChangeChannel(mf.ChannelIndex);
	
	
	if(MsgType == 0xff){  /*¶ÁÈ¡·§ÃÅ×´Ì¬*/
		Err = ValveContron(&mf,ReadVALVE_All,lu8DataIn,DataFrame,&DataLen_Vave);
			
	}
	else{
		lu8DataIn[0] = MsgType;
		Err = ValveContron(&mf,SETVALVE_STATUS,lu8DataIn,DataFrame,&DataLen_Vave);
	}
		
	if(Err == NO_ERR){
		if(MsgType == 0xff){			/*¶ÁÈ¡·§ÃÅ×´Ì¬*/
			lu8tmp = DataFrame[7];
			if(lu8tmp & 0x01)	   //Èç¹ûbit0=1±íÊ¾·§ÃÅ¿ª×´Ì¬£¬0¹Ø×´Ì¬¡£
					*pTempBack = 0x55;  // È«¿ª
				else
					*pTempBack = 0x99;  //È«¹Ø¡£

			debug_info(gDebugModule[TASKDOWN_MODULE],"ValveState = %x",lu8workmode);
				
		}
		else{
			*pTempBack = 0x01;
		}
	
		DataBackLen++;
		debug_info(gDebugModule[TASKDOWN_MODULE],"%s Send Vavle state ok ",__FUNCTION__);
	}
	else{
		
READ_FAIL:			
				*pTempBack++ = 0x10;
				DataBackLen++;
				debug_err(gDebugModule[TASKDOWN_MODULE],"%s Send Vavle state failed",__FUNCTION__);
	}

	OSSemPost(METERChangeChannelSem);
		
		//»Ø¸´Êý¾ÝÖ¡
	_ProData.MsgLength	= DataBackLen;
	_ProData.MsgType	= 0x1D;
	ReadDateTime(_ProData.TimeSmybol);
		
	//DISABLE_MBUS(); ÊµÊ±³­±íÊ±ÎªÁËÌá¸ßËÙ¶È£¬³­Íê1Ö»²»·Å¿ªÍ¨µÀ¡£

	return Err;


MBUS_Busy:
	*pTempBack++ = 0x12;  //MBUSÍ¨µÀÕýÃ¦¡£
	DataBackLen++;
	_ProData.MsgLength	= DataBackLen;
	_ProData.MsgType	= 0x1D;
	ReadDateTime(_ProData.TimeSmybol);
		
	return Err;
		
}


/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_1C_ForAll			    							                   
**	º¯ Êý  ¹¦ ÄÜ: ¶ÔÓÚÈº·¢µÄÊý¾Ý´¦Àí                                                                      			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_1C_ForAll(_ProtocolType &_ProData)
{
	uint8 Err			= 0x00;
	uint8 DataBackLen	= 0x00;
	uint8 MsgType		= 0x00;
	uint8 *pTemp = _ProData.Data.pData;
	uint8 *pTempBack = _ProData.Data.pDataBack;
	
	*pTempBack++	= *pTemp++;
	MsgType 		= *pTemp;
	*pTempBack++	= *pTemp++;
	DataBackLen 	+= 2;
	
	*pTempBack++	= 0x01;						//Èº·¢Ö±½Ó¸øÕýÈ·ÏìÓ¦ ¼´¿É
	//·¢ËÍÐÅÏ¢¸øÖ´ÐÐÈÎÎñÀ´£¬¿ªÊ¼³­¶Á»ò¿ØÖÆÈ«ÌåÈÈÁ¦±í£¬ÎÂ¿ØÃæ°å£¬·§ÃÅµÈ£»
	memset(gDEAL_ReadMailBox_Msg1C, 0x00, sizeof(gDEAL_ReadMailBox_Msg1C));
	gDEAL_ReadMailBox_Msg1C[0] = MSG_1C;
	gDEAL_ReadMailBox_Msg1C[1] = MsgType;
	memcpy(&gDEAL_ReadMailBox_Msg1C[2], _ProData.Data.pData, _ProData.MsgLength);
	
	OSQPost(CMD_Q, gDEAL_ReadMailBox_Msg1C);
	
	//»Ø¸´Êý¾ÝÖ¡
	_ProData.MsgLength	= DataBackLen+1;
	_ProData.MsgType	= 0x1D;
	ReadDateTime(_ProData.TimeSmybol);

	*pTempBack++ = 0x01;					//Õý³£»ØÓ¦

	return Err;
}

/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_MSG1C_ForAll				    							                   
**	º¯ Êý  ¹¦ ÄÜ:                                                                       			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
void DEAL_MSG1C_ForAll(uint8 *pData)
{
	uint8 i,j,Err;
	uint8 *pTemp = pData;
	uint8 Res		= 0x00;
	uint8 TempNums	= 0x00;
	uint8 MsgTemp	= 0x00;
	uint8 paratemp[20] = {0x00};
	uint8 paralen	= 0x00;
	uint16 TempArray[METER_PER_CHANNEL_NUM] = {0x00};
	uint16 MeterCmplete	= 0x00;
	uint16 MeterFailure = 0x00;
	uint16 DataTempLen	= 0x00;
	uint16 SaveCounter  = 0x00;
	uint32 LogicAddr	= METER_DATA_TEMP;
	DELU_Protocol	ReadData;
	MeterFileType   mf;
	CPU_SR		cpu_sr;
	
	MsgTemp					= *pTemp;
	pTemp++;

	//×é½¨µÂÂ³Ð­Òé  ¿ØÖÆ·§ÃÅ
	ReadData.MeterType 		= 0x20;
	ReadData.ControlCode 	= 0x04;
	ReadData.Length 		= 0x04;
	ReadData.DataIdentifier = 0x17A0;
	ReadData.DataBuf[0] 	= *pTemp;
	
	LogicAddr += 4;							//4¸ö×Ö½ÚÔ¤Áô¸ø  ÏûÏ¢ÀàÐÍ(1×Ö½Ú) + Ö¸ÁîÀàÐÍ(1×Ö½Ú) + ×Ü×Ö½ÚÊý(2×Ö½Ú)
	LogicAddr += 6;							//6¸ö×Ö½ÚÔ¤Áô¸ø  ²Ù×÷Éè±¸×ÜÊý(2Byte)+³É¹¦Éè±¸Êý(2Byte)+Ê§°ÜÉè±¸Êý(2Byte)
	//¶Ô·§ÃÅµÄ²Ù×÷
	for(i=0; i<6; i++)
	{
		DataTempLen = 0x00;
		METER_ChangeChannel(i+1);		//Í¨µÀºÅ ´Ó 1¿ªÊ¼
		
		//ÇÐ»»MbusÍ¨µÀ
		OS_ENTER_CRITICAL();
		memset(gDEAL_ReadDataTemp, 0x00, sizeof(gDEAL_ReadDataTemp));
		memcpy((uint8 *)TempArray, (uint8 *)&gPARA_ValveConChannel[i], gPARA_ValveConChannelNum[i]*sizeof(uint16));
		TempNums = gPARA_ValveConChannelNum[i];
		OS_EXIT_CRITICAL();
		
		for(j=0; j<TempNums; j++)
		{
			PARA_ReadMeterInfo(TempArray[j], &mf);
			
			//ÅÐ¶Ï·§ÃÅµØÖ·ÊÇ·ñÓÐÐ§
			if(MeterNoBcdCheck(mf.ValveAddr) == TRUE)
				{
					memcpy(ReadData.MeterAddr, mf.ValveAddr, 7);
			
					Res = METER_ReadMeterDataCur(&ReadData, TempArray[j]);
					//Res = 1;
					if(Res)	//ÉèÖÃ¿ØÖÆÀàÖ¸Áî£¬ Ö»½øÐÐ¼ÇÂ¼Ê§°Ü±íµØÖ·
						{
							memcpy(&gDEAL_ReadDataTemp[7*j], mf.MeterAddr, 7);
							MeterFailure++;
							DataTempLen += 7;
							SaveCounter += 7;
						}
					else
						{
							MeterCmplete++;
						}
				}
		}
		OSMutexPend (FlashMutex,0,&Err);
		/*need modified*/
    	Err =AppSdWrRdBytes(LogicAddr, DataTempLen, gDEAL_ReadDataTemp, SD_WR);
    	OSMutexPost (FlashMutex);
		if(Err == true)		Err = NO_ERR;
			else
				{
					
				}
		LogicAddr += DataTempLen;
	}
	//¶ÔËùÓÐÉè±¸²Ù×÷Íê³Éºó£¬¸ù¾Ý³É¹¦ºÍÊ§°Ü¼ÆÊýÀ´×Ü½áÉè±¸²Ù×÷Çé¿ö
	LogicAddr	= METER_DATA_TEMP;
	paratemp[paralen++] = 0x1C;
	paratemp[paralen++] = MsgTemp;
	if( (MeterFailure==0x00) && (MeterCmplete!=0x00) )
		{														//Ê§°ÜÊýÎª0£¬Ôò±íÊ¾ËùÓÐ¶¼³É¹¦
			paratemp[paralen++] = 0x01;
			paratemp[paralen++] = 0x00;
			paratemp[paralen++] = 0x01;
		}
	
	else if((MeterCmplete==0x00) && (MeterFailure!=0x00) )
		{														//³É¹¦ÊýÎª0£¬Ôò±íÊ¾ËùÓÐ¶¼Ê§°Ü
			paratemp[paralen++] = 0x01;
			paratemp[paralen++] = 0x00;
			paratemp[paralen++] = 0x10;
		}
	else
		{
			paratemp[paralen++] = SaveCounter+6;
			paratemp[paralen++] = (SaveCounter+6)>>8;
			paratemp[paralen++] = (MeterFailure+MeterCmplete);
			paratemp[paralen++] = (MeterFailure+MeterCmplete)>>8;
			paratemp[paralen++] = MeterCmplete;
			paratemp[paralen++] = MeterCmplete>>8;
			paratemp[paralen++] = MeterFailure;
			paratemp[paralen++] = MeterFailure>>8;
		}
	OSMutexPend (FlashMutex,0,&Err);
	/*need modified*/
    Err =AppSdWrRdBytes(LogicAddr, paralen, paratemp, SD_WR);
    OSMutexPost (FlashMutex);
	if(Err)					;
}

/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_1E				    							                   
**	º¯ Êý  ¹¦ ÄÜ:                                                                       			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:  	                                                                                   																								
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_1E(_ProtocolType &_ProData, uint8 *ControlSave)
{
	uint8 Err					= 0x00;
	uint8 Res					= 0x00;
	uint8 ComType				= 0x00;						//Ö¸ÁîÀàÐÍ
	uint8 *pTemp = _ProData.Data.pDataBack;					//ÉÏ´«¸øÖ÷Õ¾µÄÏûÏ¢ÌåÄÚÈÝ
	uint8 *pFirst				= pTemp;
	uint8 MeterDataTemp[600] 	= {0x00};					//ÔÝÊ±´æ´¢SD¿¨ÖÐÊý¾Ý
	uint8 *pDataTemp 			= MeterDataTemp;
	uint8 SendReadyFlag			= 0x00;
	uint16 LoopLen				= 0x00;						//ÏûÏ¢ÌåÊý¾Ý×îÐ¡³¤¶È£¬ÓÃÓÚ¶àÖ¡´«Êä£¬²ð°ü²»´ò¶Ï»ù±¾Êý¾Ý³¤¶È
	uint16 LoopCounter			= 0x00;
	uint16 FrameDataFileLenMax	= 0x00;						//»ñÈ¡µ±Ç°ÉèÖÃµÄ×î´óÊý¾Ý°üµÄÊý¾ÝÓò×î´ó×Ö½ÚÊý
	uint16 DataBackLen			= 0x00;
	uint16 DataStoreLen			= 0x00;
	uint32 StartLogicAddr		= 0x00;
	//uint32 EndLogicAddr			= 0x00;

	LOG_assert_param( ControlSave == NULL );

	MultiFrameMsg_1E *pMultiFrame = NULL;
	pMultiFrame = (MultiFrameMsg_1E *)ControlSave;
	
	FrameDataFileLenMax = 450;								//»ñÈ¡µ±Ç°Êý¾ÝÓò×î´ó×Ö½ÚÊý
	
	*pTemp++ = 0;											//ÊÇ·ñÓÐºóÐøÖ¡±êÖ¾£¬×éÖ¡Íêºó²Å¿ÉÒÔÌîÐ´
	*pTemp++ = pMultiFrame->SendPackIndex;
    ComType  = *_ProData.Data.pData;						//Ö¸ÁîÀàÐÍ
    DataBackLen += 2;
	
	//»ñÈ¡ÉÏ´«¿ªÊ¼µØÖ·
	if( (pMultiFrame->MultiFlag) == 0x00)
		{													//´«ÊäµÄµÚÒ»Ö¡£¬ÉÐ²»Çå³þÊÇ·ñÐèÒª¶àÖ¡´«Êä
															//¶ÁÈ¡´æ´¢²ÎÊýÇøÊý¾ÝÐÅÏ¢
			StartLogicAddr = METER_DATA_TEMP;
			OSMutexPend (FlashMutex, 0, &Err);
			/*need modified*/
    		Res = AppSdWrRdBytes(StartLogicAddr, 512, MeterDataTemp, SD_RD);
    		OSMutexPost (FlashMutex);
    		//Ðè¶ÔRes½øÐÐÅÐ¶Ï
			if(Res)				;

    		//Ðè¶Ô¶ÁÈ¡Êý¾ÝµÄÇ°N¸ö²ÎÊý½øÐÐÅÐ¶Ï
    		if(MeterDataTemp[0] != 0x1C)		;			//ÏûÏ¢ÀàÐÍ²»ÕýÈ·
    		if(MeterDataTemp[1] != ComType)		
			{		 //Ö¸ÁîÀàÐÍ²»ÕýÈ·
				*pTemp++ = 	MeterDataTemp[1];
				DataBackLen += 1;	
			}			
			else
			{
				*pTemp++ = 	ComType;
				DataBackLen += 1;
			}
    		
    		DataStoreLen = MeterDataTemp[2] | (MeterDataTemp[3]<<8);
    		pMultiFrame->EndReadAddr = pMultiFrame->StartReadAddr + DataStoreLen + METER_DATA_TEMP;
    		
    		if(DataStoreLen > 1)
    			{
    				StartLogicAddr = METER_DATA_TEMP+10;
    				/*if( (DataStoreLen - 6)%7 )
    					{
    						//×Ö½Ú³¤¶È¿ØÖÆÓÐÎÊÌâ
    					}
    				*/
    				
    				
    				
    			}
    		else
    			{	
    				//È«²¿³É¹¦ ºÍ È«²¿Ê§°ÜµÄÇé¿öÏÂ£¬Ö±½Ó·µ»Ø
    				*pTemp++ = MeterDataTemp[4];
					DataBackLen++;
    				//¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
					_ProData.MsgLength		= DataBackLen;
					_ProData.MsgType		= 0x1F;
					ReadDateTime(_ProData.TimeSmybol);
	
					return NO_ERR;
    			}
		}
	else
		{
			StartLogicAddr = pMultiFrame->StartReadAddr + METER_DATA_TEMP+10;
		}
    LoopLen	= 7;

    while(!SendReadyFlag)
    {
    	OSMutexPend (FlashMutex, 0, &Err);
		/*need modified*/
    	Res = AppSdWrRdBytes(StartLogicAddr, 518, MeterDataTemp, SD_RD);
    	OSMutexPost (FlashMutex);
    	//Ðè¶ÔRes½øÐÐÅÐ¶Ï
		if(Res)				;
		LoopCounter = 0x00;
    	
    	while(LoopCounter<74)
    	{
    		if( (DataBackLen+LoopLen) > FrameDataFileLenMax )
    			{	
    				SendReadyFlag = 1;					
    				break;										//Ô¤´¦Àí½øÐÐÅÐ¶Ï³¤¶ÈÊÇ·ñ³¬¹ýÏÞÖÆ
    			}
    		else
    			{
    				if( (StartLogicAddr+DataBackLen-3) < (pMultiFrame->EndReadAddr) )				//ÅÐ¶ÏÊÇ·ñ»¹ÓÐÊý¾ÝÃ»´«Íê
    					{
    						*pFirst = 0x01;						//ÈÔÓÐÊý¾ÝÐèÒª´«Êä
    						
    						memcpy(pTemp, pDataTemp, LoopLen);
    						pTemp 		+= LoopLen;
    						DataBackLen += LoopLen;
    						pDataTemp 	+= LoopLen;
        	
    					}
    				else
    					{
    						*pFirst = 0x00;
    						SendReadyFlag = 1;		
    						break;
    					}
    			}
    		LoopCounter++;
    	}
    }
    pMultiFrame->MultiFlag 		= *pFirst;
    pMultiFrame->SendByteNums 	= DataBackLen - 3;
    
    //¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
	_ProData.MsgLength		= DataBackLen;
	_ProData.MsgType		= 0x1F;
	ReadDateTime(_ProData.TimeSmybol);
	
	return NO_ERR;
}


/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_22				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Ö÷Õ¾Ïò¼¯ÖÐÆ÷Ë÷Òª¹Ì¶¨Ê±¼äµãÊý¾Ý                                                                      			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢: SD¿¨ÖÐÃ¿¸ö±íµÄÊý¾ÝÕ¼ÓÃ128×Ö½ÚµÄ¿Õ¼ä£¬ÆäÖÐµÚÒ»¸ö×Ö½ÚÎªÊý¾Ý³¤¶È(1×Ö½Ú) + Êý¾Ý(N+11) + CSÀÛ¼ÓºÍÐ£Ñé(1×Ö½Ú)
** 	              ÆäÖÐÊý¾Ý¸ñÊ½Îª: ÈÈ¼ÆÁ¿±íµØÖ·(7×Ö½Ú) + ÈÈÁ¿Êý¾Ý³¤¶È(1×Ö½Ú) + ÈÈÁ¿Êý¾Ý(N) + ÎÂ¶ÈÊý¾Ý(3×Ö½Ú)
**				  Èô¶ÁÈ¡µ½µÃÊý¾Ý²»ÕýÈ·£¬»òÃ»ÓÐ¶ÁÈ¡µ½Êý¾Ý£¬Ôò¹æ¶¨ÈÈÁ¿Êý¾Ý³¤¶ÈÎª0£¬ÎÞÈÈÁ¿Êý¾Ý¡£                                                                    																								
 **   ÐÞ¸Ä¼ÇÂ¼   :1. add input para char*path 2012-12-26  
                                       
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_22(_ProtocolType &_ProData, uint8 *ControlSave)
{

	uint8 Err					= 0x00;
	uint8 Res					= 0x00;
	uint8 Cs					= 0x00;
	uint8 *pTemp = _ProData.Data.pDataBack;
	uint8 *pFirst				= pTemp;
	uint8 ReadTime[6] 			= {0x00};
	uint8 MeterDataTemp[1024] 	= {0x00};					//ÔÝÊ±´æ´¢SD¿¨ÖÐÊý¾Ý
	uint8 *pDataTemp 			= MeterDataTemp;
	uint8 MeterFrameLen			= 0x00;						//¼ìÑé¶ÁÈ¡µ½µÄÊý¾ÝÖ¡ÕýÈ·ÐÔ£¬²¢·µ»ØÊý¾ÝÖ¡×Ö½ÚÊý
	uint8 DataCounter			= 0x00;
	uint8 DataErrCounter		= 0x00;
	uint8 SendReadyFalg 		= 0x00;
	uint16 StartMeterSn			= 0x00;
	uint16 MeterSnMax			= 0x00;
	uint16 FrameDataFileLenMax	= 0x00;						//»ñÈ¡µ±Ç°ÉèÖÃµÄ×î´óÊý¾Ý°üµÄÊý¾ÝÓò×î´ó×Ö½ÚÊý
	uint16 DataBackLen			= 0x00;
	//uint32 LogicAddr			= LOGIC_ADDR_NULL;
	
	/*begin:yangfei modified 2012-12-24 find path*/
	char  DataPath[]="/2012/12/24/1530";
	char  NodePath[]="/2012/12/24/timenode";
	uint8   HexTime[6] 			= {0x00};
	/*end   :yangfei modified 2012-12-24*/
	
	LOG_assert_param( ControlSave == NULL );

	MultiFrameMsg_22 *pMultiFrame = NULL;
	pMultiFrame = (MultiFrameMsg_22 *)ControlSave;
	CPU_SR		cpu_sr;
	DataStoreParaType	History_Para;
	
	if(ControlSave == NULL)		while(1);
	
	//»ñÈ¡ÊµÊ±µÄ¿ØÖÆ±äÁ¿Êý¾Ý
	memcpy(ReadTime, _ProData.Data.pData, 6);
	//»ñÈ¡µ±Ç°Êý¾ÝÓò×î´ó×Ö½ÚÊý
	//»ñÈ¡±íµµ°¸ÖÐ±íÐòºÅµÄ×î´óÊý, ÓÉÓÚÊÇÊµÊ±Êý¾Ý£¬Ö±½Ó¶ÁÈ¡ÄÚ´æ±¸·Ý¼´¿É
	OS_ENTER_CRITICAL();
	FrameDataFileLenMax = gPARA_TermPara.DebugPara.FrameLen_Max;
	OS_EXIT_CRITICAL();
	//»ñÈ¡ÉÏ´ÎÉÏ´«µ½ÄÄ¸ö±íÐòºÅ
	StartMeterSn = pMultiFrame->StartMeterSn;
	/*begin:yangfei modified 2012-12-24 find path*/
	BcdTimeToHexTime(ReadTime,HexTime);
	//GetTimeNode(HexTime,TimeNodes);
	//GetTimeNodeFilePath(DataPath,HexTime,TimeNodes);
	GetFilePath(NodePath, HexTime, ARRAY_DAY);  /*»ñÈ¡¹Ì¶¨µãtimenode path*/
	OSMutexPend (FlashMutex, 0, &Err);
	Res = SDReadData(NodePath, (uint8 *)&History_Para, sizeof(History_Para),0);
	OSMutexPost (FlashMutex);
	//GetTimeNodeFilePath(DataPath,HexTime,History_Para.TimeNode);
	GetTimeNodeFilePath_Forward(DataPath,HexTime,&History_Para);  //²éÕÒ¸ø¶¨Ê±¼äµãÇ°Ò»¸ö¶¨Ê±³­Êý¾Ý´æ·ÅÂ·¾¶¡£
	/*end   :yangfei modified 2012-12-24*/
    //Ðè¶ÔRes½øÐÐÅÐ¶Ï
	if(Res!=NO_ERR)	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
    	}
	
	MeterSnMax = History_Para.MeterNums;
	
    pFirst = pTemp++;										//ÊÇ·ñÓÐºóÐøÖ¡±êÖ¾£¬×éÖ¡Íêºó²Å¿ÉÒÔÌîÐ´
    DataBackLen++;
    *pTemp++ = pMultiFrame->SendPackIndex;					//µ±Ç°·¢ËÍµÄ°üÐòºÅ
    DataBackLen++;
	/*begin:yangfei modified 2013-1-4 for add time*/
	memcpy(pTemp, ReadTime, 6);
	pTemp += 6;
	DataBackLen += 6;
	/*end:yangfei modified 2013-1-4 */
	
    while(!SendReadyFalg)
    {
	/*begin:yangfei modified 2012-12-24 find path*/
	OSMutexPend (FlashMutex, 0, &Err);
	Res = SDReadData(DataPath, MeterDataTemp, sizeof(MeterDataTemp),StartMeterSn*128); 
	debug_info(gDebugModule[METER_DATA],"%s %d StartMeterSn =%d MeterSnMax=%d!\r\n",__FUNCTION__,__LINE__,StartMeterSn,MeterSnMax);
	OSMutexPost (FlashMutex);
	if(Res!=NO_ERR)
    	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
		    debug_err(gDebugModule[METER_DATA],"ERROR: %s %d SDSaveData Error Res = %d !",__FUNCTION__,__LINE__,Res );
			goto ERROR;
    	}
	/*end  :yangfei modified 2012-12-24 find path*/
    	//Ðè¶ÔRes½øÐÐÅÐ¶Ï
		DataCounter = 0;
		pDataTemp =  MeterDataTemp;

    	while(DataCounter< (1024/METER_FRAME_LEN_MAX))
    	{
			MeterFrameLen = *pDataTemp++;
			//´Ë´ÎÒªÅÐ¶Ï³¤¶ÈÊÇ·ñÎªÓÐÐ§
        	
    		if( (DataBackLen+MeterFrameLen) > FrameDataFileLenMax )
    			{						
    				SendReadyFalg = 1;
    				break;										//Ô¤´¦Àí½øÐÐÅÐ¶Ï³¤¶ÈÊÇ·ñ³¬¹ýÏÞÖÆ
    			}
    		else
    			{
					/*begin :yangfei modified 2012-01-14 for StartMeterSn´Ó0¿ªÊ¼¼ÆÊý£¬¶à´«ÊäÁËÒ»´Î*/
						#if 0
						if(StartMeterSn <= MeterSnMax)				//ÅÐ¶ÏÊÇ·ñ»¹ÓÐ±íÃ»´«Íê
						#endif
						if(StartMeterSn < MeterSnMax)
    				/*end  :yangfei modified 2012-01-14*/
    					{
    					/*begin :yangfei modified 2013-08-29 for µ±×îºóÒ»°üµ½Ê±ÎÞÏÂÒ»°üÊý¾Ý,·ÀÖ¹Ö÷»ú²»Í£µÄÒªÊý¾Ý*/
                                            if(StartMeterSn+1==MeterSnMax)
                             	             {
                             	                 debug_info(gDebugModule[METER_DATA],"StartMeterSn+1==MeterSnMax\r\n");
							    *pFirst = 0x00;	
                             	             }
					         else
					         	{
					         	*pFirst = 0x01;						//ÈÔÓÐÊý¾ÝÐèÒª´«Êä
					         	}
					  /*end :yangfei modified 2013-08-29 for µ±×îºóÒ»°üµ½Ê±ÎÞÏÂÒ»°üÊý¾Ý*/	
    						
    						Cs = *(pDataTemp+MeterFrameLen);
    						if( Cs == PUBLIC_CountCS(pDataTemp, MeterFrameLen) )
    							{
    								if( (MeterFrameLen>0x00) && (MeterFrameLen<0x80) )
    									{
    										memcpy(pTemp, pDataTemp, MeterFrameLen);
    										pTemp 		+= MeterFrameLen;
    										DataBackLen += MeterFrameLen;
    									}
							      else
    									{
    										DataErrCounter++;
    									}
    							}
    						else
    							{
    								DataErrCounter++;
    							}
							DataCounter++;
    						pDataTemp 	= &MeterDataTemp[DataCounter*METER_FRAME_LEN_MAX];
    						StartMeterSn++;
    					}
    				else
    					{
    						*pFirst = 0x00;
    						SendReadyFalg = 1;
    						break;
    					}
    			}
    	}
    }
    pMultiFrame->MultiFlag 		= *pFirst;
    pMultiFrame->SendMeterNums 	= StartMeterSn - (pMultiFrame->StartMeterSn);
    
    debug("StartMeterSn =%d  pMultiFrame->StartMeterSn=%d\r\n ",StartMeterSn,pMultiFrame->StartMeterSn);
    
    //¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
	_ProData.MsgLength		= DataBackLen;
	_ProData.MsgType		= 0x23;
	ReadDateTime(_ProData.TimeSmybol);
	
	if((DataErrCounter == pMultiFrame->SendMeterNums)&&(DataErrCounter!=0))			//Èç¹ûÊý¾ÝÈ«²¿²»ÕýÈ·
		{
			_ProData.MsgLength		= 1;
			*_ProData.Data.pDataBack = 0x10;					//Òì³£»ØÓ¦
			LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d  Msg_22 Error!",__FUNCTION__,__LINE__ );
			debug_info(gDebugModule[METER_DATA],"WARNING: %s %d  Msg_22 Error DataErrCounter =%d!",__FUNCTION__,__LINE__ ,DataErrCounter);
			goto ERROR;
		}
	
	return NO_ERR;

ERROR:
	_ProData.MsgType		= 0x23;
	_ProData.MsgLength		= 1;
	*_ProData.Data.pDataBack = 0x10;					//Òì³£»ØÓ¦

	return ERR_1;
}



/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_3C				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Ö÷Õ¾Ïò¼¯ÖÐÆ÷Ë÷ÒªÖ¸¶¨Ê±¼äµãÖ¸¶¨¼ÆÁ¿µãºÅµÄÀúÊ·Êý¾Ý¡£                                                                   			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢: SD¿¨ÖÐÃ¿¸ö±íµÄÊý¾ÝÕ¼ÓÃ128×Ö½ÚµÄ¿Õ¼ä£¬ÆäÖÐµÚÒ»¸ö×Ö½ÚÎªÊý¾Ý³¤¶È(1×Ö½Ú) + Êý¾Ý(N+11) + CSÀÛ¼ÓºÍÐ£Ñé(1×Ö½Ú)
** 	              ÆäÖÐÊý¾Ý¸ñÊ½Îª: ÈÈ¼ÆÁ¿±íµØÖ·(7×Ö½Ú) + ÈÈÁ¿Êý¾Ý³¤¶È(1×Ö½Ú) + ÈÈÁ¿Êý¾Ý(N) + ÎÂ¶ÈÊý¾Ý(3×Ö½Ú)
**				  Èô¶ÁÈ¡µ½µÃÊý¾Ý²»ÕýÈ·£¬»òÃ»ÓÐ¶ÁÈ¡µ½Êý¾Ý£¬Ôò¹æ¶¨ÈÈÁ¿Êý¾Ý³¤¶ÈÎª0£¬ÎÞÈÈÁ¿Êý¾Ý¡£                                                                    																								
 **   ÐÞ¸Ä¼ÇÂ¼   :
                                       
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_3C(_ProtocolType &_ProData)
{
	uint16 i = 0;
	uint8 Err					= 0x00;
	uint8 Res					= 0x00;
	uint8 Cs					= 0x00;
	uint8 *pTemp = _ProData.Data.pDataBack;
	uint8 ReadTime[6] 			= {0x00};
	uint8 MeterDataTemp[128] 	= {0x00};					//ÔÝÊ±´æ´¢SD¿¨ÖÐÊý¾Ý
	uint8 *pDataTemp 			= MeterDataTemp;
	uint8 MeterFrameLen			= 0x00;						//¼ìÑé¶ÁÈ¡µ½µÄÊý¾ÝÖ¡ÕýÈ·ÐÔ£¬²¢·µ»ØÊý¾ÝÖ¡×Ö½ÚÊý
	uint16 MeterSnMax			= 0x00;
	uint16 DataBackLen			= 0x00;
	//uint32 LogicAddr			= LOGIC_ADDR_NULL;
	
	char  DataPath[]="/2012/12/24/1530";
	char  NodePath[]="/2012/12/24/timenode";
	uint8   HexTime[6] 			= {0x00};
	uint8 lu8MeterID = 0;
	uint16 lu8RecMeterID = 0;  //ÓÃÓÚ´æ´¢Ö÷»ú·¢À´µÄÄ¿µÄ¼ÆÁ¿µãºÅ¡£
	
	CPU_SR		cpu_sr;
	DataStoreParaType	History_Para;
	
	
	//»ñÈ¡ÊµÊ±µÄ¿ØÖÆ±äÁ¿Êý¾Ý
	memcpy(ReadTime, _ProData.Data.pData, 6);  //È¡³öÄ¿±êÊ±¼ä¡£
	memcpy((uint8 *)(&lu8RecMeterID), _ProData.Data.pData+6, 2);  //È¡³öÄ¿±ê±íµØÖ·¡£


	/*begin:yangfei modified 2012-12-24 find path*/
	BcdTimeToHexTime(ReadTime,HexTime);
	GetFilePath(NodePath, HexTime, ARRAY_DAY);  /*»ñÈ¡¹Ì¶¨µãtimenode path*/
	OSMutexPend (FlashMutex, 0, &Err);
	Res = SDReadData(NodePath, (uint8 *)&History_Para, sizeof(History_Para),0);
	OSMutexPost (FlashMutex);
	//Ðè¶ÔRes½øÐÐÅÐ¶Ï
	if(Res != NO_ERR)	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
			goto ERROR;
	}
	
	//GetTimeNodeFilePath(DataPath,HexTime,History_Para.TimeNode);
	GetTimeNodeFilePath_Forward(DataPath,HexTime,&History_Para);  //²éÕÒ¸ø¶¨Ê±¼äµãÇ°Ò»¸ö¶¨Ê±³­Êý¾Ý´æ·ÅÂ·¾¶¡£
	/*end   :yangfei modified 2012-12-24*/

	MeterSnMax = History_Para.MeterNums;
	
    *pTemp++ = 0;				//ÊÇ·ñÓÐºóÐøÖ¡±êÖ¾£¬ÎÞºóÐø£¬¹Ì¶¨Îª0.
    DataBackLen++;
    *pTemp++ = 0;					//µ±Ç°·¢ËÍµÄ°üÐòºÅ,¹Ì¶¨Îª0 ¡£
    DataBackLen++;
	/*begin:yangfei modified 2013-1-4 for add time*/
	memcpy(pTemp, ReadTime, 6);
	pTemp += 6;
	DataBackLen += 6;
	/*end:yangfei modified 2013-1-4 */

	for(i=0;i<MeterSnMax;i++){
		OSMutexPend (FlashMutex, 0, &Err);
		Res = SDReadData(DataPath, MeterDataTemp, sizeof(MeterDataTemp),i*128); 
		OSMutexPost (FlashMutex);
		if(Res != NO_ERR)	{
				LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
		}
		
		memcpy((uint8 *)(&lu8MeterID), &MeterDataTemp[1], 2);  //È¡³ö±íµØÖ·¡£
		if(lu8MeterID == lu8RecMeterID){
			MeterFrameLen = *pDataTemp++;
			Cs = *(pDataTemp+MeterFrameLen);
			if( Cs == PUBLIC_CountCS(pDataTemp, MeterFrameLen) )  {
				if( (MeterFrameLen>0x00) && (MeterFrameLen<0x80) )	{
						memcpy(pTemp, pDataTemp, MeterFrameLen);
						pTemp		+= MeterFrameLen;
						DataBackLen += MeterFrameLen;

						break;
				}
				else{
					goto ERROR;	
				}
			}

		}

		if(i == (MeterSnMax-1)){  //Ã»ÓÐ²éÑ¯µ½¡£
			goto ERROR;

		}

		
	}
	
    
    //¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
	_ProData.MsgLength		= DataBackLen;
	_ProData.MsgType		= 0x3D;
	ReadDateTime(_ProData.TimeSmybol);
	
	return NO_ERR;

ERROR:
	_ProData.MsgType		= 0x3D;
	_ProData.MsgLength		= 1;
	*_ProData.Data.pDataBack = 0x10;					//Òì³£»ØÓ¦

	return ERR_1;
}





/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_3E				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Ö÷Õ¾Ïò¼¯ÖÐÆ÷·¢ËÍÁ¢¼´³­±íÖ¸Áî¡£                                                                   			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**   ÐÞ¸Ä¼ÇÂ¼   :
                                       
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_3E(_ProtocolType &_ProData)
{
	uint8 lu8SendMsg[2]		= {0x00};
	
	if((gu8ReadAllMeterFlag!=0) && (gu8ReadAllMeterFlag!=1)){  //·ÀÖ¹³ö´í¡£
		gu8ReadAllMeterFlag = 0;
	}
	

	ReadDateTime(_ProData.TimeSmybol);

	
	if(gu8ReadAllMeterFlag == 1){//ÕýÔÚÈ«³­±í¡£
		_ProData.MsgType		= 0x3F;
		_ProData.MsgLength		= 1;
		*_ProData.Data.pDataBack = 0x12;	//Òì³£»ØÓ¦,ÕýÔÚÈ«³­¡£
		return ERR_1;

	}
	else{
		lu8SendMsg[0] =  TIMING_ALL;
		OSQPost(CMD_Q, (void*)lu8SendMsg);

		OSTimeDlyHMSM(0,0,1,0);
		
		_ProData.MsgType		= 0x3F;
		_ProData.MsgLength		= 1;
		*_ProData.Data.pDataBack = 0x01;	//

		return NO_ERR;


	}
	

}



/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_40				    							                   
**	º¯ Êý  ¹¦ ÄÜ: Ö÷Õ¾Ïò¼¯ÖÐÆ÷Ë÷Òª¹Ì¶¨Ê±¼äµãÊý¾Ý                                                                      			
**	Êä Èë  ²Î Êý: 							         			 	      
**	Êä ³ö  ²Î Êý: none											                                       
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:                                                                																								
**   ÐÞ¸Ä¼ÇÂ¼   :
1. add input para char*path 2012-12-26  
2.  yangfei added 20140219  for Ö§³ÖÈÈ·ÖÅä±í¡¢Ê±¼äÍ¨¶ÏÃæ»ý·¨

    sd¿¨ÖÐÊý¾Ý±£´æ¸ñÊ½:(Ã¿¿éÎª128×Ö½Ú)
    0:ÈÈ±íÊý¾Ý
    1:ÈÈ·ÖÅä±í»ò·§¿ØÊý¾Ý
    2:ÈÈ·ÖÅä±í»ò·§¿ØÊý¾Ý
    3:ÈÈ·ÖÅä±í»ò·§¿ØÊý¾Ý
    ...

    ´ÓµÚ1¿é¿ªÊ¼¶ÁÈ¡
	¸ù¾ÝÉè±¸ÀàÐÍ
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_40(_ProtocolType &_ProData, uint8 *ControlSave)
{
	uint8 Err					= 0x00;
	uint8 Res					= 0x00;
	uint8 Cs					= 0x00;
	uint8 *pTemp = _ProData.Data.pDataBack;
	uint8 *pFirst				= pTemp;
	uint8 ReadTime[6] 			= {0x00};
	uint8 MeterDataTemp[1024] 	= {0x00};					//ÔÝÊ±´æ´¢SD¿¨ÖÐÊý¾Ý
	uint8 *pDataTemp 			= MeterDataTemp;
	uint8 MeterFrameLen			= 0x00;						//¼ìÑé¶ÁÈ¡µ½µÄÊý¾ÝÖ¡ÕýÈ·ÐÔ£¬²¢·µ»ØÊý¾ÝÖ¡×Ö½ÚÊý
	uint8 DataCounter			= 0x00;
	uint8 DataErrCounter		= 0x00;
	uint8 SendReadyFalg 		= 0x00;
	uint16 StartMeterSn			= 0x00;
	uint16 MeterSnMax			= 0x00;
	uint16 FrameDataFileLenMax	= 0x00;						//»ñÈ¡µ±Ç°ÉèÖÃµÄ×î´óÊý¾Ý°üµÄÊý¾ÝÓò×î´ó×Ö½ÚÊý
	uint16 DataBackLen			= 0x00;

	char  DataPath[]="/2012/12/24/1530";
	char  NodePath[]="/2012/12/24/timenode";
	uint8   HexTime[6] 			= {0x00};
	
	LOG_assert_param( ControlSave == NULL );

	MultiFrameMsg_40 *pMultiFrame = NULL;
	pMultiFrame = (MultiFrameMsg_40 *)ControlSave;
	CPU_SR		cpu_sr;
	DataStoreParaType	History_Para;
	
	if(ControlSave == NULL)		while(1);
	
	//»ñÈ¡ÊµÊ±µÄ¿ØÖÆ±äÁ¿Êý¾Ý
	memcpy(ReadTime, _ProData.Data.pData, 6);
	//»ñÈ¡µ±Ç°Êý¾ÝÓò×î´ó×Ö½ÚÊý
	//»ñÈ¡±íµµ°¸ÖÐ±íÐòºÅµÄ×î´óÊý, ÓÉÓÚÊÇÊµÊ±Êý¾Ý£¬Ö±½Ó¶ÁÈ¡ÄÚ´æ±¸·Ý¼´¿É
	OS_ENTER_CRITICAL();
	FrameDataFileLenMax = gPARA_TermPara.DebugPara.FrameLen_Max;
	OS_EXIT_CRITICAL();
	//»ñÈ¡ÉÏ´ÎÉÏ´«µ½ÄÄ¸ö±íÐòºÅ
	StartMeterSn = pMultiFrame->StartMeterSn;
	/*begin:yangfei modified 2012-12-24 find path*/
	BcdTimeToHexTime(ReadTime,HexTime);
	
	GetFilePath(NodePath, HexTime, ARRAY_DAY);  /*»ñÈ¡¹Ì¶¨µãtimenode path*/
	OSMutexPend (FlashMutex, 0, &Err);
	Res = SDReadData(NodePath, (uint8 *)&History_Para, sizeof(History_Para),0);
	OSMutexPost (FlashMutex);
	if(Res!=NO_ERR)
	{
		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
	}
	GetTimeNodeFilePath(DataPath,HexTime,History_Para.TimeNode);
	
	MeterSnMax = History_Para.MeterNums;
	
    pFirst = pTemp;										//ÊÇ·ñÓÐºóÐøÖ¡±êÖ¾£¬×éÖ¡Íêºó²Å¿ÉÒÔÌîÐ´

	pTemp += sizeof(MultiFrameMsg_40)-2;/*Ô¤ÁôÊý¾ÝÍ·Î»ÖÃ*/
		
	DataBackLen += sizeof(MultiFrameMsg_40)-2;
	
    while(!SendReadyFalg)
    {
	OSMutexPend (FlashMutex, 0, &Err);
	Res = SDReadData(DataPath, MeterDataTemp, sizeof(MeterDataTemp),StartMeterSn*128); 
	debug_info(gDebugModule[METER_DATA],"%s %d StartMeterSn =%d MeterSnMax=%d!\r\n",__FUNCTION__,__LINE__,StartMeterSn,MeterSnMax);
	OSMutexPost (FlashMutex);
	if(Res!=NO_ERR)
    	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
		    debug_err(gDebugModule[METER_DATA],"ERROR: %s %d SDSaveData Error Res = %d !",__FUNCTION__,__LINE__,Res );
			goto ERROR;
    	}
    	//Ðè¶ÔRes½øÐÐÅÐ¶Ï
		DataCounter = 0;
		pDataTemp =  MeterDataTemp;

    	while(DataCounter< (1024/METER_FRAME_LEN_MAX))
    	{
			MeterFrameLen = *pDataTemp++;
    		if( (DataBackLen+MeterFrameLen) > FrameDataFileLenMax )/*´Ë´ÎÒªÅÐ¶Ï³¤¶ÈÊÇ·ñÎªÓÐÐ§*/
    			{						
    				SendReadyFalg = 1;
    				break;										//Ô¤´¦Àí½øÐÐÅÐ¶Ï³¤¶ÈÊÇ·ñ³¬¹ýÏÞÖÆ
    			}
    		else
    			{
						if(StartMeterSn < MeterSnMax)
    					{
    					/*begin :yangfei modified 2013-08-29 for µ±×îºóÒ»°üµ½Ê±ÎÞÏÂÒ»°üÊý¾Ý,·ÀÖ¹Ö÷»ú²»Í£µÄÒªÊý¾Ý*/
                              if(StartMeterSn+1==MeterSnMax)
                             	{
                             	debug_info(gDebugModule[METER_DATA],"StartMeterSn+1==MeterSnMax\r\n");
							    *pFirst = 0x00;	
                             	}
					         else
					         	{
					         	*pFirst = 0x01;						//ÈÔÓÐÊý¾ÝÐèÒª´«Êä
					         	}
					  /*end :yangfei modified 2013-08-29 for µ±×îºóÒ»°üµ½Ê±ÎÞÏÂÒ»°üÊý¾Ý*/	
    						
    						Cs = *(pDataTemp+MeterFrameLen);
    						if( Cs == PUBLIC_CountCS(pDataTemp, MeterFrameLen) )
    							{
    								if( (MeterFrameLen>0x00) && (MeterFrameLen<0x80) )
    									{
    										memcpy(pTemp, pDataTemp, MeterFrameLen);
    										pTemp 		+= MeterFrameLen;
    										DataBackLen += MeterFrameLen;
    									}
							      else
    									{
    										DataErrCounter++;
    									}
    							}
    						else
    							{
    								DataErrCounter++;
    							}
							DataCounter++;
    						pDataTemp 	= &MeterDataTemp[DataCounter*METER_FRAME_LEN_MAX];
    						StartMeterSn++;
    					}
    				else
    					{
    						*pFirst = 0x00;
    						SendReadyFalg = 1;
    						break;
    					}
    			}
    	}
    }
	
	memcpy(pMultiFrame->time, ReadTime, 6);
	pMultiFrame->device_type = gPARA_TermPara.DeviceType;
	pMultiFrame->MultiFlag 		= *pFirst;
    pMultiFrame->SendMeterNums 	= StartMeterSn - (pMultiFrame->StartMeterSn);

	memcpy(pFirst, pMultiFrame, sizeof(MultiFrameMsg_40)-2);/*±£´æÊý¾ÝÍ·*/
    
    debug("StartMeterSn =%d  pMultiFrame->StartMeterSn=%d\r\n ",StartMeterSn,pMultiFrame->StartMeterSn);
    
    //¸øÖ÷Õ¾µÄÏìÓ¦Ö¡ ÌîÐ´Êý¾Ý½á¹¹£¬Ö»ÐèÌîÐ´ÏÂÃæÐÅÏ¢£¬ÆäÓàÐÅÏ¢ÔÚCreateº¯ÊýÖÐ²Ù×÷
	_ProData.MsgLength		= DataBackLen;
	_ProData.MsgType		= 0x41;
	ReadDateTime(_ProData.TimeSmybol);
	
	if((DataErrCounter == pMultiFrame->SendMeterNums)&&(DataErrCounter!=0))			//Èç¹ûÊý¾ÝÈ«²¿²»ÕýÈ·
		{
			_ProData.MsgLength		= 1;
			*_ProData.Data.pDataBack = 0x10;					//Òì³£»ØÓ¦
			LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d  Msg_22 Error!",__FUNCTION__,__LINE__ );
			debug_info(gDebugModule[METER_DATA],"WARNING: %s %d  Msg_22 Error DataErrCounter =%d!",__FUNCTION__,__LINE__ ,DataErrCounter);
			goto ERROR;
		}
	
	return NO_ERR;

ERROR:
	_ProData.MsgType		= 0x41;
	_ProData.MsgLength		= 1;
	*_ProData.Data.pDataBack = 0x10;					//Òì³£»ØÓ¦

	return ERR_1;
}







/********************************************************************************************************
**  º¯ Êý  Ãû ³Æ: DEAL_ProcessMsg_42			    							                   
**	º¯ Êý  ¹¦ ÄÜ:Õë¶ÔÒÚÁÖ·§¿ØÆ÷Ð­ÒéµÄ Í¸Ã÷´«ÊäÖ¸Áî¡£                                                                     			
**	Êä Èë  ²Î Êý: 
**	Êä ³ö  ²Î Êý: none											                                      
**  ·µ   »Ø   Öµ: none													                               
**	±¸		  ×¢:                              																									
*********************************************************************************************************/
uint8 DEAL_ProcessMsg_42(_ProtocolType &_ProData)	
{
	uint8 Err = 0x01;
	uint8 lu8OvertimeCnt = 0;
	uint8 DataFrame[128] = {0};
	uint8 lu8DataLen_Vave = 0;
	uint8 lu8DevType = 0;
	uint8 lu8channel = 0;
	uint8 lu8ProtocolType = 0;
	uint8 lu8ForAllorOne = 0;
		
	FeedTaskDog();

	gu8MBusShortTime = 0;

	if(gu8ReadAllMeterFlag == 0){//Ö»ÓÐÔÚ·ÇÈ«³­×´Ì¬²ÅÍ¸´«£¬·ñÔòÌáÊ¾ÕýÃ¦¡£

		lu8ForAllorOne = _ProData.Data.pData[0]; 
		if(lu8ForAllorOne == FORALL){  //Èº·¢
			Err = Communicate_ForAll(_ProData.Data.pData);

			_ProData.MsgLength		= 1; 
			_ProData.MsgType		= 0x43;
			*_ProData.Data.pDataBack = 0x01;  //Èº·¢ÏÂÐÐÉè±¸ÎÞ·µ»Ø£¬¼¯ÖÐÆ÷¹Ì¶¨ÏòÉÏÎ»»ú·µ»Ø³É¹¦¡£
			Err = ReadDateTime(_ProData.TimeSmybol);
			
		}
		else{	//µ¥·¢

		//Ê×ÏÈÇÐ»»µ½¶ÔÓ¦Í¨µÀ¡£
			lu8channel = _ProData.Data.pData[2];
			METER_ChangeChannel(lu8channel);


		//Æä´Î¸ù¾ÝÉè±¸ÀàÐÍ£¬Éè¶¨ÕýÈ·µÄ´®¿Ú²ÎÊý¡£
			lu8DevType =  _ProData.Data.pData[1];
			lu8ProtocolType = _ProData.Data.pData[3];

			FeedTaskDog();

			if(lu8DevType == HEAT_METER_TYPE){  //ÈÈ±í

				(*METER_ComParaSetArray[gMETER_Table[lu8ProtocolType][0]])();
			}
			else if(lu8DevType == TIME_ON_OFF_AREA_TYPE){  //·§¿ØÆ÷¡£
				(*METER_ComParaSetArray[gVALVE_Table[lu8ProtocolType][0]])();  //ÉèÖÃ·§ÃÅ¶ÔÓ¦´®¿Ú²ÎÊý¡£
			}
			else{
				//´ýÐèÒªÊ±²¹³ä¡£
			}

loop:		lu8DataLen_Vave =  _ProData.Data.pData[5];  
			memcpy(DataFrame, &(_ProData.Data.pData[6]),lu8DataLen_Vave);
			FeedTaskDog();
					
			//Err =  ReadData_Communicate(DataFrame, &lu8DataLen_Vave);  //ËùÓÐÈÈ±í¡¢·§Í¸´«¶¼¿ÉÒÔÓÃÕâ¸öº¯ÊýÏÂ·¢½ÓÊÕ¡£
			if(lu8DevType == HEAT_METER_TYPE)
					Err =  ReadData_Communicate(DataFrame, &lu8DataLen_Vave); 
			else if(lu8DevType == TIME_ON_OFF_AREA_TYPE){
					if(lu8ProtocolType == 0x01)   //ÒÚÁÖ·§ÃÅ
						Err =  Yilin_Valve_ReceiveFrame(DataFrame, &lu8DataLen_Vave);

					else
						Err =  ReadData_Communicate(DataFrame, &lu8DataLen_Vave); 

			}
			else{

			}

				FeedTaskDog();

				if(Err != OS_ERR_NONE){
					if(lu8OvertimeCnt < 3){ 
							lu8OvertimeCnt++;
							goto loop;
					}
					else{
						lu8OvertimeCnt = 0;
												
						_ProData.MsgLength		= 2; 
						_ProData.MsgType		= 0x43;
						*_ProData.Data.pDataBack = 0x10;
						*(_ProData.Data.pDataBack+1) = gu8MBusShortTime;
						Err = ReadDateTime(_ProData.TimeSmybol);
											
					}
				}
				else{  //if(err != OS_ERR_NONE)
					lu8OvertimeCnt = 0;
										
					_ProData.MsgLength		= lu8DataLen_Vave + 1; 
					DataFrame[lu8DataLen_Vave] = gu8MBusShortTime;
					memcpy(_ProData.Data.pDataBack,DataFrame,lu8DataLen_Vave+1);
					_ProData.MsgType		= 0x43;
					Err = ReadDateTime(_ProData.TimeSmybol);
																
				}
				
				//METER_ChangeChannel(7);//¶Ï¿ªMBUSÍ¨µÀ¡£
			}
		}
		else{
			_ProData.MsgLength		= 1; 
			_ProData.MsgType		= 0x43;
			*_ProData.Data.pDataBack = 0x12;  //ÌáÊ¾ÕýÃ¦¡£
			Err = ReadDateTime(_ProData.TimeSmybol);

		}

		gu8MBusShortTime = 0; //ÔÚ´Ë´¦½«´Ë±äÁ¿Çå³ý¡£
	
		return Err; 
}


/*****************************************************************************************
**								End of File
*****************************************************************************************/

