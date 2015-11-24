/*
  ********************************************************************************************************
  * @file    valve_binihi.c
  * @author  zjjin
  * @version V0.0.0
  * @date    04-20-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		°ÙÅ¯»ãĞ­Òé·§¿ØÆ÷¹¦ÄÜÊµÏÖÏà¹Ø´úÂë¡£
  *
  ********************************************************************************************************
  */  

//#include <includes.h>

#include "Valve.h"  
#include "valve_binihi.h"


#define HEX_TO_BCD(x) ((x/0x0A)*0x10+(x%0x0A))
#define BCD_TO_HEX(x) ((x/0x10)*0x0A+(x%0x10))






/*
  ********************************************************************************************************
  * º¯ÊıÃû³Æ: uint8 ValveContron_Elsonic(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
  
  * Ëµ    Ã÷£ºÒÚÁÖ ·§¿ØÆ÷¿ØÖÆº¯Êı£¬ÔÚ´Ëº¯ÊıÖĞÊµÏÖÒÚÁÖ·§¿Ø²»Í¬¿ØÖÆ¡£
  *					
  * ÊäÈë²ÎÊı£º 
  				MeterFileType *p_mf   ±í²ÎÊı£¬º¬ÓĞ·§¿ØĞ­Òé°æ±¾¡¢·§¿ØµØÖ·µÈĞÅÏ¢¡£
  				uint8 functype  ¶Ô·§½øĞĞÊ²Ã´ÑùµÄ¿ØÖÆ£¬±ÈÈç¶ÁĞÅÏ¢¡¢ÉèÖÃÉÏÏÂÏŞÎÂ¶ÈµÈ¡£
				uint8 *p_datain  ÊäÈëº¯Êı¿ÉÄÜÒªÊ¹ÓÃµÄÊı¾İ¡£
  				uint8 *p_databuf    ´Ó·§¿ØÖĞ¶ÁÉÏÀ´µÄÊı¾İ´æ´¢Ö¸Õë¡£
  				uint8 p_datalenback  ´Ó·§¿ØÖĞ¶ÁÉÏÀ´µÄÊı¾İ³¤¶È¡£
  * Êä³ö²ÎÊı:
  				Ö´ĞĞÊÇ·ñ³É¹¦¡£
  ********************************************************************************************************
  */

uint8 ValveContron_Binihi(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
{
	uint8 Err = 0;
	uint8 lu8dataframe[100] = {0};
	uint8 lu8datalen = 0;
	uint8 lu8databuf[20] = {0xee};
	uint8 lu8datalenback = 0;
	uint8 i =0;
//	uint8 lu8tmp = 0;
	uint8 lu8valvestatus = 0;
	

	
	switch(functype){
		case ReadVALVE_All:{						
			Create_Binihi_Vave_ReadInfo_Frame(p_mf,lu8dataframe, &lu8datalen);//×é½¨»ñÈ¡·§ÃÅÊı¾İÃüÁîÖ¡
			Err = Binihi_ReadData_Communicate(lu8dataframe, &lu8datalen);
			
			if(Err == NO_ERR){	  
				lu8databuf[i++] = 0x00;  //Ğ¡ÊıÎ»¹Ì¶¨Îª0.
				lu8datalenback++;
				lu8databuf[i++] = HEX_TO_BCD(lu8dataframe[13]);
				lu8datalenback++;
				lu8databuf[i++] = 0x00;  //·ûºÅÎ»
				lu8datalenback++;

				lu8databuf[4] = 0; //ÏÈ½«×´Ì¬×Ö½Ú³õÊ¼»¯Îª0 ¡£

				lu8valvestatus = (lu8dataframe[9]>>5) & 0x03;  
				switch(lu8valvestatus){
					case 0x00:{  //½âËø¹Ø·§
						lu8databuf[i++] = 0x99;  //È«¹Ø¡£

					break;
					}
					case 0x01:{ //½âËø¿ª·§¡£
						lu8databuf[i++] = 0x55;  // È«¿ª
					break;
					}
					case 0x02:{ //ÉÏËø¹Ø·§
						lu8databuf[i++] = 0x99;  //È«¹Ø¡£
						lu8databuf[4] |= 0x08;  //Ö¸Ê¾·§ÃÅËø¶¨¡£
					break;
					}
					case 0x03:{ //ÉÏËø¿ª·§
						lu8databuf[i++] = 0x55;  // È«¿ª
						lu8databuf[4] |= 0x08;  //Ö¸Ê¾·§ÃÅËø¶¨¡£
					break;
					}
					default:{
						lu8databuf[i++] = 0x99;  //È«¹Ø¡£
						break;

					}

				}

				lu8datalenback++;
				
				//begin:·§¿Ø×´Ì¬Î»×Ö½Ú´¦Àí¡£
				
				//·§ÃÅÊÇ·ñËø¶¨ÔÚÉÏÃæµÄswitchÖĞÒÑ¾­ÅĞ¶Ï¡£
				//end:·§¿Ø×´Ì¬Î»×Ö½Ú´¦Àí¡

				i++;
				lu8datalenback++;
				lu8databuf[i++] = 0x00;  //Ô¤Áô¡£
				lu8datalenback++;
							
			}				
			else{
				memset(lu8databuf, 0xee, 6);
				lu8datalenback += 6;								
				debug_err(gDebugModule[TASKDOWN_MODULE],"%s %d Read Valve state failed!\r\n",__FUNCTION__,__LINE__);
			}

			memcpy(p_databuf,lu8databuf,lu8datalenback);
			*p_datalenback = lu8datalenback;
			
			break;
		}

		case SETHEAT_DISPLAY:{

			break;
		}

		case SETHEAT_VALUE:{
			Create_Binihi_Vave_SetInfo_Frame(p_mf,p_DataIn,lu8dataframe, &lu8datalen);
			Err = Binihi_ReadData_Communicate(lu8dataframe, &lu8datalen);
			if(Err == NO_ERR){
				debug_info(gDebugModule[TASKDOWN_MODULE],"%s %d Send HeatMeter data  to valve ok",__FUNCTION__,__LINE__);
			}
			else  {
				debugX(LOG_LEVEL_ERROR,"%s %d Send HeatMeter data  to valve  failed!\r\n",__FUNCTION__,__LINE__);
			}

			memcpy(p_databuf,lu8dataframe,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETROOM_TEMP:{
			Err = 1;    //°ÙÅ¯»ãÖ»ÄÜÍ¬Ê±Éè¶¨ÎÂ¶ÈºÍÉÏÏÂÏŞ£¬µ¥ÉèÎÂ¶È±¨´í¡£

			break;
		}

		case SETTEMP_RANGE:{
			Err = 1;   //°ÙÅ¯»ãÖ»ÄÜÍ¬Ê±Éè¶¨ÎÂ¶ÈºÍÉÏÏÂÏŞ£¬µ¥ÉèÉÏÏÂÏŞ±¨´í¡£

			break;
		}

		case SETVALVE_STATUS:{
			Create_Binihi_SetValve_Frame(p_mf,p_DataIn,lu8dataframe, &lu8datalen);
			Err = Binihi_ReadData_Communicate(lu8dataframe, &lu8datalen);

			memcpy(p_databuf,lu8dataframe,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETTEMP_ANDRANGE:{
			Create_Binihi_SetTemp_Frame(p_mf,p_DataIn,lu8dataframe, &lu8datalen);
			Err = Binihi_ReadData_Communicate(lu8dataframe, &lu8datalen);
			
			if(Err == NO_ERR)	{
					debug_info(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature ok ",__FUNCTION__);
				}
			else{
					debug_err(gDebugModule[TASKDOWN_MODULE],"%s Set indoor given temperature failed ",__FUNCTION__);
			}

			memcpy(p_databuf,lu8dataframe,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETVALVE_CONTROLTYPE:{
			if(*(p_DataIn + 0) == 0x09){
				Create_Binihi_SetValve_Frame(p_mf,p_DataIn,lu8dataframe, &lu8datalen);
				Err = Binihi_ReadData_Communicate(lu8dataframe, &lu8datalen);

				memcpy(p_databuf,lu8dataframe,lu8datalen);
				*p_datalenback = lu8datalen;
			}

			break;
		}

		default:
			break;
	}



	return Err;
}

/*
  ******************************************************************************
  * º¯ÊıÃû³Æ£º void Create_Binihi_Vave_SetInfo_Frame(DELU_Protocol *pData, uint8 *pSendFrame, uint8 *plenFrame)
  * Ëµ    Ã÷£º´Ëº¯ÊıÓÃÓÚ½«µ±Ç°ÈÈÁ¿ÖµĞ´Èë°ÙÅ¯»ã·§¿ØÆ÷¡£
  * ²Î    Êı£º 
  ******************************************************************************
  */
void Create_Binihi_Vave_SetInfo_Frame(MeterFileType *p_mf,uint8 *p_DataIn, uint8 *pSendFrame, uint8 *plenFrame)
{
	/*
	{                                                                            
	uint32   DailyHeat;		                ½áËãÈÕÈÈÁ¿	   +0
    uint8     DailyHeatUnit;                                                   +4
	uint32   CurrentHeat;				   µ±Ç°ÈÈÁ¿    +5
    uint8     CurrentHeatUnit;                                                +9
	uint32 	 HeatPower;			      ÈÈ¹¦ÂÊ             +10
    uint8     HeatPowerUnit;                                                  +14
	uint32 	 Flow;			                 Á÷Á¿Á÷ËÙ   +15
    uint8 	 FlowUnit;                                                            +19
	uint32 	AccumulateFlow;			ÀÛ»ıÁ÷Á¿       +20
    uint8 	AccumulateFlowUnit;	                                         +24
                                                                                         
	uint8	WaterInTemp[3];		        ¹©Ë®ÎÂ¶È	 +25
	uint8 	WaterOutTemp[3];				»ØË®ÎÂ¶È +28
    uint8 	AccumulateWorkTime[3];	ÀÛ¼Æ¹¤×÷Ê±¼ä   +31
	uint8	RealTime[7];		                ÊµÊ±Ê±¼ä	  +34
	uint16 	ST;					                    ×´Ì¬ST+41 
	}*/
	uint8   setInform[23]={0x68,0x11,0x11,0x68,0x53,0x00,0x00,0x00,0x53,
						0x60,0x1C,0x10,0x1E,0x00,0x00,0x00,0x2B,0xA1,0x07,0x03,0x01,0x07,0x16};
	
//	uint8 templen	= 0x00;
	uint8 cs		= 0x00;
	uint8 *pTemp;
//    uint16 lenFrame = 0x00;
//    uint16 CheckLen = 0x00;
	uint8 i =0;
	uint8 lu8currentheat[4] = {0x00};
	uint32 lu32currentheat =0;
	uint8 lu8valveaddr[7] = {0x00};
	uint16 lu16valveaddr =0;
	
    LOG_assert_param(p_mf == NULL);
    LOG_assert_param(pSendFrame == NULL);
    LOG_assert_param(plenFrame == NULL);
	
	pTemp = pSendFrame;

	memcpy(lu8valveaddr, &p_mf->ValveAddr[0], 7);				
 	for(i=0;i<7;i++){
 		lu8valveaddr[i] = BCD_TO_HEX(lu8valveaddr[i]);
 	}
 	lu16valveaddr = lu8valveaddr[0] + lu8valveaddr[1]*100 + lu8valveaddr[2]*10000;
 	
 	setInform[5] = lu16valveaddr & 0x00ff;
 	setInform[6] = (lu16valveaddr>>8) & 0x00ff;


	memcpy(lu8currentheat,p_DataIn, 4);			   //µ±Ç°ÈÈÁ¿È¡³ö¡£
	for(i=0;i<4;i++){
		lu8currentheat[i] = BCD_TO_HEX(lu8currentheat[i]);
	}
	lu32currentheat = lu8currentheat[1] + lu8currentheat[2]*100 + lu8currentheat[3]*10000;  //µ±Ç°ÈÈÁ¿µÄÕûÊı²¿·Ö
	lu32currentheat = lu32currentheat*10 + lu8currentheat[0]/10; //Ìí¼ÓĞ¡Êı²¿·Ö£¬²¢ÇÒÀ©´ó10±¶¡£

	setInform[16] = lu32currentheat & 0x00ff;
	setInform[17] = (lu32currentheat>>8) & 0x00ff;
	setInform[18] = (lu32currentheat>>16) & 0x00ff;
	setInform[19] = 0x00;  //ÉèÖÃÆäËûÎŞĞ§£¬Ö»Ğ´Èëµ±Ç°ÈÈÁ¿Öµ¡£

	cs = 0;
	for(i=9;i<21;i++){
		cs += setInform[i];
	}
	cs += setInform[4];
	cs += setInform[8];
	setInform[21] = cs;


	memcpy(pTemp, &setInform[0], 23);
	*plenFrame = 23;
	


}



/*
  ******************************************************************************
  * º¯ÊıÃû³Æ£º void Create_Binihi_SetValve_Frame(DELU_Protocol *pData, uint8 *pSendFrame, uint8 valvestate,uint8 *plenFrame)
  * Ëµ    Ã÷£º´Ëº¯ÊıÓÃÓÚÉèÖÃ°ÙÅ¯»ã·§¿ª¹Ø×´Ì¬£¬
  * ²Î    Êı£º valvestate = 1  Ëø¶¨¿ª·§
  				valvestate = 0 Ëø¶¨¹Ø·§
  				valvestate = 2 Ëø¶¨£¬ÉÏÎ»»ú²»¿ØÖÆ·§ÃÅ
  				valvestate = other£¬½âËø£¬·§ÃÅÓÉÃæ°å¿ØÖÆ¡£
  ******************************************************************************
  */
void Create_Binihi_SetValve_Frame(MeterFileType *p_mf, uint8 *p_DataIn,uint8 *pSendFrame, uint8 *plenFrame)
{
	uint8   setInform[23]={0x68,0x11,0x11,0x68,0x53,0x55,0x01,0x00,0x53,
						0x60,0x1C,0x10,0x1E,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x07,0x16};
	
//	uint8 templen	= 0x00;
	uint8 cs		= 0x00;
	uint8 *pTemp;
//    uint16 lenFrame = 0x00;
//    uint16 CheckLen = 0x00;
	uint8 i =0;
	uint8 lu8valveaddr[7] = {0x00};
	uint16 lu16valveaddr =0;
	
    LOG_assert_param(p_mf == NULL);
    LOG_assert_param(pSendFrame == NULL);
    LOG_assert_param(plenFrame == NULL);
	
	pTemp = pSendFrame;
	
	memcpy(lu8valveaddr, &p_mf->ValveAddr[0], 7);				
 	for(i=0;i<7;i++){
 		lu8valveaddr[i] = BCD_TO_HEX(lu8valveaddr[i]);
 	}
 	lu16valveaddr = lu8valveaddr[0] + lu8valveaddr[1]*100 + lu8valveaddr[2]*10000;
 	
 	setInform[5] = lu16valveaddr & 0x00ff;
 	setInform[6] = (lu16valveaddr>>8) & 0x00ff;


	switch(*(p_DataIn+0)){
		case 0x99:  //¹Ø
		{
			setInform[9] = 0x20;
			break;
		}
		case 0x55:	//¿ª
		{
			setInform[9] = 0x00;
			break;
		}
		case 0x09:  //½âËø£¬·§ÃÅÓÉÎÂ¿ØÃæ°å¿ØÖÆ¡£
		{
			setInform[9] = 0x60;
			break;
		}
		default:
		{
			setInform[9] = 0x00;  //ÆäËûÈ«¿ª·§£¬°²È«µÚÒ»¡£
			break;
		}
			

	}

	setInform[19] = 0x01;  //·§¿Ø×Ö½ÚÓĞĞ§¡£

	cs = 0;
	for(i=9;i<21;i++){
		cs += setInform[i];
	}
	cs += setInform[4];
	cs += setInform[8];
	setInform[21] = cs;


	memcpy(pTemp, &setInform[0], 23);
	*plenFrame = 23;
	


}


/*
  ******************************************************************************
  * º¯ÊıÃû³Æ£º 
  void Create_Binihi_SetTemp_Frame(DELU_Protocol *pData, uint8 *pSendFrame,uint8 *plenFrame)
  
  * Ëµ    Ã÷£º´Ëº¯ÊıÓÃÓÚÉèÖÃ°ÙÅ¯»ã·§ÊÒÄÚÎÂ¶ÈºÍÉÏÏÂÏŞÎÂ¶È¡£
  * ²Î    Êı£º 
  ******************************************************************************
  */
void Create_Binihi_SetTemp_Frame(MeterFileType *p_mf, uint8 *p_DataIn,uint8 *pSendFrame, uint8 *plenFrame)
{
	uint8   setInform[23]={0x68,0x11,0x11,0x68,0x53,0x55,0x01,0x00,0x53,
						0x60,0x1C,0x10,0x1E,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x07,0x16};
	
//	uint8 templen	= 0x00;
	uint8 cs		= 0x00;
	uint8 *pTemp;
//    uint16 lenFrame = 0x00;
//    uint16 CheckLen = 0x00;
	uint8 i =0;
	uint8 lu8valveaddr[7] = {0x00};
	uint16 lu16valveaddr =0;
	
    LOG_assert_param(p_mf == NULL);
    LOG_assert_param(pSendFrame == NULL);
    LOG_assert_param(plenFrame == NULL);
	
	pTemp = pSendFrame;
	
	memcpy(lu8valveaddr, &p_mf->ValveAddr[0], 7);				
 	for(i=0;i<7;i++){
 		lu8valveaddr[i] = BCD_TO_HEX(lu8valveaddr[i]);
 	}
 	lu16valveaddr = lu8valveaddr[0] + lu8valveaddr[1]*100 + lu8valveaddr[2]*10000;
 	
 	setInform[5] = lu16valveaddr & 0x00ff;
 	setInform[6] = (lu16valveaddr>>8) & 0x00ff;


	setInform[10] = BCD_TO_HEX(*(p_DataIn+1));  //Éè¶¨ÎÂ¶È
	
	setInform[11] = BCD_TO_HEX(*(p_DataIn+7)); //Éè¶¨ÏÂÏŞÎÂ¶È

	setInform[12] = BCD_TO_HEX(*(p_DataIn+4)); //Éè¶¨ÉÏÏŞÎÂ¶È¡£




	setInform[19] = 0x02;  //Éè¶¨ÉÏÏÂÏŞÎÂ¶ÈºÍÊÒÄÚÎÂ¶ÈÓĞĞ§¡£

	cs = 0;
	for(i=9;i<21;i++){
		cs += setInform[i];
	}
	cs += setInform[4];
	cs += setInform[8];
	setInform[21] = cs;


	memcpy(pTemp, &setInform[0], 23);
	*plenFrame = 23;
	


}


void Create_Binihi_Vave_ReadInfo_Frame(MeterFileType *p_mf, uint8 *pSendFrame, uint8 *plenFrame)
{
	
	uint8 readinfo[7]={0x10,0x5b,0x34,0x12,0x00,0x00,0x16};//Ä¬ÈÏ±íºÅ:1234
	
//	uint8 templen	= 0x00;
	uint8 cs		= 0x00;
	uint8 *pTemp;
//	uint16 lenFrame = 0x00;
//	uint16 CheckLen = 0x00;
	uint8 i =0;
	uint8 lu8valveaddr[7] = {0};
	uint16 lu16valveaddr = 0;
	LOG_assert_param(p_mf == NULL);
	LOG_assert_param(pSendFrame == NULL);
	LOG_assert_param(plenFrame == NULL);
	
	pTemp = pSendFrame;

	memcpy(lu8valveaddr, p_mf->ValveAddr, 7); 
 	for(i=0;i<7;i++){
 		lu8valveaddr[i] = BCD_TO_HEX(lu8valveaddr[i]);
 	}
 	lu16valveaddr = lu8valveaddr[0] + lu8valveaddr[1] * 100 + lu8valveaddr[2] * 10000;
 	readinfo[2] = lu16valveaddr & 0x00ff;
 	readinfo[3] = (lu16valveaddr >> 8) & 0x00ff;



	
	for(i=1;i<5;i++){
		   cs += readinfo[i];
		}
	readinfo[5] = cs;
	memcpy(pTemp, &readinfo[0], 7);
	*plenFrame = 7;
}


uint8 Binihi_ReadData_Communicate(uint8 *DataFrame, uint8 *DataLen)
{

	 uint8 err;
	 uint8 RetryTimes	  = 0x02;
//	 uint8 data=0;
	 uint8 dev = DOWN_COMM_DEV_MBUS;
     uint8 i=0; 
	 uint8 len=0;
	  
	 uint8 DataBuf[METER_FRAME_LEN_MAX*2];
     LOG_assert_param(DataFrame == NULL);
	 LOG_assert_param(DataLen == NULL);
	
	for(i=0;i<RetryTimes;i++){
		DuQueueFlush(dev);				  //Çå¿Õ»º³åÇø	  
		DuSend(dev, (uint8*)DataFrame,  *DataLen);
		
		OSTimeDly(OS_TICKS_PER_SEC/10);
		
		err = Receive_ReadBinihiParamFrame(dev, DataBuf, 0, &len);

		if(err==NO_ERR){
			 memcpy(DataFrame, &DataBuf[0], len);
			 *DataLen = len;
			 return NO_ERR;
		}
		else{
			OSTimeDlyHMSM(0,0,0,500);
		}
		  
	}           
         
     return 1;
}




uint8 Binihi_VaveCommunicate(uint8 *DataFrame, uint8 *DataLen)
{
	  uint8 err;
//      uint8 *cp = NULL;
	  uint8 RetryTimes	  = 0x02;
	  uint8 dev = DOWN_COMM_DEV_MBUS;
      uint8 i=0;
	  uint8 length=0;
	  uint8   DataBuf[METER_FRAME_LEN_MAX];
      LOG_assert_param(DataFrame == NULL);
	  LOG_assert_param(DataLen == NULL);
	//  LOG_assert_param(pDataLenBack == NULL);
	for(i=0;i<RetryTimes;i++)
	{
		DuQueueFlush(dev);										  //Çå¿Õ»º³åÇø	  
		DuSend(dev, (uint8*)DataFrame,  *DataLen);
		
		//OSTimeDly(OS_TICKS_PER_SEC/10);
		err = Receive_ReadBinihiParamFrame(dev, DataBuf, 0, &length);
		if(err==NO_ERR)
			{
			return NO_ERR;
			}
	}           
         
		 
           return 1;
}




uint8 Receive_ReadBinihiParamFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen)
{
	uint8 data=0;
	uint8 i=0;
	uint8 readinfom[32]={0};
	uint8 len = 0;
	uint8 len1 = 0;
	uint8 Cs=0;
	
    i = 20;

	while(i--){													//ÕÒÖ¡Í·
			if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC))
				{return 1;}
			
			if((data==0x68) || (data==0x5e))	
				{break; }
			
	}

	if(data==0x5e){  //Èç¹û¿ªÊ¼Ã»ÓĞÊÕµ½0x68£¬¶øÊÇÊÕµ½0x5e£¬ËµÃ÷ÊÇÉèÖÃ²ÎÊı·µ»Ø³É¹¦¡£
		readinfom[0] = 0x5e;
		*datalen = 1;
		memcpy(buf, &readinfom[0], 1);
		
	    return 0;  
	}
	else{
		readinfom[0] = 0x68;
		if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC)){
			return 2;
		}
		
		len = data;
		readinfom[1] = len;

		if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC)){
			return 3;
		}
		len1 = data;
		readinfom[2] = len1;

		if(len == len1){
			if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC)){
				return 4;
			}
			readinfom[3] = data;

			for(i=4;i<9;i++){
				if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC)){
					return 6;
				}
				readinfom[i] = data;
			}
					
			for(i=9;i<len+4;i++){
				if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC)){
					return 6;
				}
				readinfom[i] = data;
				Cs	+= data;
			}

			Cs += readinfom[4];
			Cs += readinfom[8];

			if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC)){
				return 7;
			}
			readinfom[len+4] = data;

			if(Cs == data){  //Ğ£ÑéÊÇ·ñÕıÈ·¡£
				if(DuGetch(dev, &data, 2*OS_TICKS_PER_SEC)){
					return 9;
				}
				readinfom[len+5] = data;  //½ØÖ¹·ûºÅ¡£

				*datalen = len + 6;
		   
		  		 memcpy(buf, &readinfom[0], *datalen);
				 
	 		 	 return 0;
	   
				
			}
			else{
				return 8;
			}
			
			
			

		}
		else{
			return 5;
		}

	}
	
	
}	

