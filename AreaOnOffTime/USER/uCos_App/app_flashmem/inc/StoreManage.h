/*******************************************Copyright (c)*******************************************
                                                          
**  ÎÄ   ¼ş   Ãû: StoreManage.h
**  ´´   ½¨   ÈË: ¹´½­ÌÎ
**  ´´ ½¨  ÈÕ ÆÚ: 2012.08.09 
**  Ãè        Êö: ´æ´¢·ÖÅäÍ·ÎÄ¼ş
**	±¸		  ×¢: ±¾ÏµÍ³µÄ´æ´¢½éÖÊÎªSD¿¨£¬ÈİÁ¿Îª2GBÒÔÉÏ£¬±£Ö¤ÁË¾ø¶Ô¹»µÄ´æ´¢¿Õ¼ä¡£SD¿¨ÒÔ512×Ö½ÚÎªÒ»¿é£¬
**				  ÔÚÇı¶¯³ÌĞòÖĞ×öÁË¿ç¿é·ÃÎÊ´¦Àí£¬ËùÒÔ·ÖÅäÊ±²»±Ø¹ı¶à¿¼ÂÇ¿ç¿éÎÊÌâ¡£
**				  ´æ´¢¿Õ¼ä½Ï´ó£¬Ã¿¸öÏà¹Ø²ÎÊıºÍÊı¾İ¾ù·ÖÅä 1MByte ¿Õ¼ä
*****************************************************************************************************/
#ifndef _STOREMANGE_H
#define _STOREMANGE_H

#define TIME_NODE_NUM_MAX		24							//ÉèÖÃ³­±íÊ±¼ä½ÚµãµÄ×î´ó¸öÊı

#define PACK_MAX				900



/********************************¼¯ÖĞÆ÷²ÎÊı´æ´¢·ÖÅä -- End********************************************/

/********************************ÈÈ¼ÆÁ¿±íÊı¾İ´æ´¢·ÖÅä -- Start****************************************/

#define METET_FILE_PER_LEN_STORE	64				//Ã¿¸ö±íµµ°¸Êµ¼ÊÕ¼ÓĞ25¸ö×Ö½Ú£¬´æ´¢Ê±Õ¼ÓĞ64×Ö½Ú¿Õ¼ä£¬ÆäÓàÔ¤Áô
#define METER_NUM_MAX				600								//¼¯ÖĞÆ÷ÏÂ¿É½ÓµÄÈÈ¼ÆÁ¿±íµÄ×î´óÊıÁ¿
#define METER_PER_CHANNEL_NUM		200								//Ã¿¸öÍ¨µÀµÄ×î´ó±íÊıÁ¿
#define METER_FRAME_LEN_MAX			128								//Êı¾İÖ¡×î´ó³¤¶È, ¸ÃÊı¾İ±ØĞë±£Ö¤Îª2µÄN´Î·½
#define STORE_DAYS_PER_MONTH		31								//Ã¿ÔÂ´æ´¢31ÌìµÄÊı¾İ£¬¸ÃÔÂÈôÃ»31ºÅµÈ£¬×öÈßÓà´¦Àí
#define METERS_DATA_LEN				0x12E00							//Ã¿Ò»¸öÊ±¼ä½Úµã£¬´æ´¢µÄÊı¾İ£¬Õ¼151¸öÊı¾İ¿é

#define METER_CHANNEL_NUM  7 

#define METER_DATA_TEMP			0xA00000							//´Ó10M¿ªÊ¼´æ´¢Êı¾İ
																	//´æ´¢ËùÓĞÈÈ¼ÆÁ¿±í£¬ÎÂ¿ØÃæ°å£¬·§ÃÅ×´Ì¬µÈĞèÒªÔİÊ±´æ´¢µÄÊı¾İ
																	//¶ÔÆä´æ´¢×îºÃÓĞÒ»¶¨µÄÊı¾İ¸ñÊ½
																	//ÏûÏ¢ÀàĞÍ(1×Ö½Ú) + Ö¸ÁîÀàĞÍ(1×Ö½Ú) + ×Ü×Ö½ÚÊı(2×Ö½Ú) + Êı¾İ + Êı¾İ + Êı¾İ....



#define	METERS_DATA_STORE_PUBLIC	0x1400000						//ÓÃÓÚ´æ´¢·ÇÖ¸¶¨ÔÂµÄÊı¾İ, ´Ó20MµØÖ·¿ªÊ¼

//METER_DATA_STORE_ADDR,Êı¾İ´æ´¢µÄ»ùµØÖ·£¬ÒÔ2012Äê1ÔÂÎª»ù×¼															
#define METERS_DATA_STORE_ADDR		0x1E00000						//ÓÃÓÚ¹©Å¯¼¾´æ´¢, ´Ó30MµØÖ·¿ªÊ¼

//Í¨¶ÏÊ±¼äÃæ»ı·¨£¬ÈÈÁ¿·ÖÌ¯Ëã·¨ÓĞÉÏÎ»»úÍê³É£¬ÉÏÎ»»ú·¢ËÍ·ÖÌ¯Öµµ½¼¯ÖĞÆ÷£¬¼¯ÖĞÆ÷½«·ÖÌ¯Öµ
//±£´æµ½³­·§Êı¾İµÄºóÃæ£¬ÎªÁË±ÜÃâ¸²¸ÇÒÑ¾­³­·§±£´æµÄĞÅÏ¢£¬ĞèÒª´Ó128×Ö½Ú´æ´¢Î»ÖÃµÄ
//ÆğÊ¼Î»ÖÃÆ«ÒÆÒ»¶¨×Ö½Ú²Å¿ÉÒÔ£¬TA_HEATVALVE_SAVEOFFSETºê¶¨Òå¾ÍÊÇÕâ¸ö×÷ÓÃ¡£
#define TA_HEATVALVE_SAVEOFFSET		80  //ÉÏÃæ×¢ÊÍ¡£Öµ±ØĞëĞ¡ÓÚ128-sizeof(HeatValue_Format).

#pragma	pack(1)
//Êı¾İ´æ´¢·ÖÅä£ºÃ¿Ìì×î´ó´æTIME_NODE_NUM_MAX¸öµãµÄÊı¾İ£¬¸ÃÈÕµÄ³­±íÊ±¼äµã´æ´¢ÓÚ³­±íÊ±¼äµãµÄµÚÒ»¸öÊ±¼ä½ÚµãµÄ²ÎÊıÇø£¬Ö»ÓĞÃ¿ÈÕµÚÒ»¸öµã
//µÄÊ±¼ä½ÚµãÊı¾İ ÊÇÓĞĞ§µÄ£¬
//±¾½á¹¹ÌåÎªÃ¿Ò»¸ö³­±íÊ±¼ä½ÚµãµÄÊı¾İ´æ´¢µÄ²ÎÊıÇø£¬ÔÚÃ¿¸ö³­±íÊ±¼äµã£¬¿ªÊ¼³­±íÇ°ĞèÒª¶ÔÆä½øĞĞ³õÊ¼»¯
typedef struct{
		uint8 	DataRecordTime[6];								//Êı¾İ´æ´¢¼ÇÂ¼µÄÊ±¼ä½Úµã ÄêÔÂÈÕÊ±·ÖÃë£¬·ÖÃëÒ»°ãÓÃ²»ÉÏ
		//uint8   ReadCmplFlag[75];								//¶Á±íÊı¾İÍê³É±êÖ¾ 75*8=600
		uint16	MeterNums;										//±¾Ê±¼ä½Úµã¹²´æ´¢ÁË¶àÉÙ¸ö±í
		uint16  TimeNode[TIME_NODE_NUM_MAX];					//ÔÚÃ¿ÌìÆğÊ¼Ê±¼äµã£¬¸Ã²ÎÊıÓĞĞ§£¬´æ´¢¸ÃÈÕµÄÊ±¼ä½ÚµãË÷Òı
		uint16 	TimeNodeNum;//¹²ÉèÖÃÁË¼¸¸ö¶¨Ê±µã¡£	//ÈôÔÚ¸ÃÈÕÄÚ£¬ÖØĞÂÉèÖÃÁËÊ±¼ä½Úµã£¬Ğè¸üĞÂ¸Ã²ÎÊı£¬
																//¸ÃÈÕÒÔÇ°µÄÊı¾İÎŞĞ§,ÒÔ±£Ö¤Ã¿ÈÕÖ»´æ´¢TIME_NODE_NUM_MAX¸öµãµÄÊı¾İ
		//uint8	DataIdle[128-6-2-2*TIME_NODE_NUM_MAX - 2];
		uint8	DataIdle[118 - 2*TIME_NODE_NUM_MAX];		//DataIdle[128-6-2-2*TIME_NODE_NUM_MAX - 2];ÎªÁËÊı¾İÖ¡ÒÔ¿é¶ÔÆë£¬Ìí¼Ó´Ë¿ÕÊı¾İ
	}DataStoreParaType;											//ÒÔÉÏÎªÒ»¸öÊ±¼ä½ÚµãµÄ²ÎÊı¡£¹²128×Ö½Ú
typedef struct{
		uint16 WriteFlag;					//¸ÃÇøÓòÊÇ·ñ±»Ğ´¹ıÊı¾İ
		uint16 LCDContrast;				//LCD¶Ô±È¶È
	}LCDContrastPara;	//muxiaoqing add
typedef struct{
		uint16 WriteFlag;					//¸ÃÇøÓòÊÇ·ñ±»Ğ´¹ıÊı¾İ
		uint16 MeterStoreNums;				//¹²´æ´¢¶àÉÙ¿é±íµµ°¸ĞÅÏ¢
#ifdef DEMO_APP
        
        uint16 DEMOWriteFlag;					//¸ÃÇøÓòÊÇ·ñ±»Ğ´¹ıÑİÊ¾°æ±¾Êı¾İ
#endif		
	}MeterFilePara;

typedef struct{
		uint16 MeterID;						//ÈÈ±í±àºÅ£¬Î¨Ò»£¬µ«¿ÉÄÜ²»Á¬Ğø¡£
		uint8 MeterAddr[7];					//ÈÈ¼ÆÁ¿±íµØÖ·
		uint8 Manufacturer;					//³§ÉÌ´úÂë
		uint8 ProtocolVer;					//Ğ­Òé°æ±¾
		uint8 EquipmentType;				//Éè±¸ÀàĞÍ
		uint8 ChannelIndex;					//Í¨µÀºÅ
		uint8 ValveProtocal;				//·§¿ØÆ÷Ğ­ÒéºÅ£¬ÓÃÓÚ¼æÈİ¶à³§¼Ò²»Í¬·§¿ØÆ÷¡£
		uint8 ValveAddr[7];					//·§ÃÅµØÖ·
		uint8 ControlPanelAddr[7];			//¿ØÖÆÃæ°åµØÖ·
		uint8 BulidID;						//Â¥ºÅ
		uint8 UnitID;						//µ¥ÔªºÅ
		uint16 RoomID;						//·¿¼äºÅ
		uint16 Reserved1;					//Ò»ÏÂÔ¤Áô8×Ö½Ú¡£
		uint16 Reserved2;
		uint16 Reserved3;
		uint16 Reserved4;
	}MeterFileType;							//¹²40¸ö×Ö½Ú


typedef struct
{
	uint8  user_id;/*ÓÃ»§±àºÅ*/
	uint16	address;  /*Éè±¸µØÖ·*/
	uint16	area;/*ÓÃ»§Ãæ»ı*/
	uint8  data_valid;/*Êı¾İÊÇ·ñÓĞĞ§  ÓĞĞ§Îª0xA5£¬ÎŞĞ§Îª0x00*/
	uint32 total_energy;/*(BCDÂë)ÓÃ»§µÄÄÜÁ¿ÀÛ¼ÆÖµ,ºó2Î»ÎªĞ¡ÊıÎ»¡£µ¥Î»:kwh*/
	uint32 open_time;/*(BCDÂë)¿ª·§Ê±¼ä£¬ºó2Î»ÎªĞ¡ÊıÎ»¡£µ¥Î»:h*/
	uint32 proportion_energy;/*(BCDÂë)µ±Ç°Ê±¼ä¶ÎÄÚµÄÄÜÁ¿·ÖÌ¯Öµ*/
	uint8  open_percent;/*µ±Ç°Ê±¼ä¶ÎÄÚµÄ·§ÃÅ¿ªÆô±ÈÀı(HEXÂë 0-100) */
	uint16 forward_temperature;/*(BCDÂë)½øË®ÎÂ¶È£¬ºó2Î»ÎªĞ¡ÊıÎ»¡£µ¥Î»:ÉãÊÏ¶È*/
	uint16 return_temperature;/*(BCDÂë)»ØË®ÎÂ¶È£¬ºó2Î»ÎªĞ¡ÊıÎ»¡£µ¥Î»:ÉãÊÏ¶È*/
	uint16 room_temperature;/*(BCDÂë)·¿¼äÎÂ¶È£¬ºó2Î»ÎªĞ¡ÊıÎ»¡£µ¥Î»:ÉãÊÏ¶È*/
	uint8  state;/*bit7 ´ú±íÎŞÏß¹ÊÕÏ£¬1¹ÊÕÏ 0Õı³£ 
								  bit6 ´ú±íÇ··Ñ×´Ì¬ 1Ç··Ñ 0Õı³£  
								 bit5 ´ú±í³äÖµ×´Ì¬ 1³äÖµ 0Õı³£	
								 bit4 ´ú±í¿ª¹Ø»ú×´Ì¬£¬1¿ª»ú£¬0¹Ø»ú; 
								 bit3 ´ú±íËø¶¨ 0²»Ëø£¬1Ëø¶¨;
								 bit2 ´ú±í·§ÃÅ¶Â×ª¹ÊÕÏ£¬1¹ÊÕÏ£¬0Õı³£; 
								 bit1 ´ú±íNTC¹ÊÕÏ		 1¹ÊÕÏ£¬0Õı³£;
								 bit0 ´ú±í·§ÃÅ×´Ì¬£¬1·§¿ª£¬0·§¹Ø*/	
	uint8 channel;/*Í¨µÀºÅ*/

}TimeAreaArith;


typedef struct{
		uint16 FrameLen_Max;				//¼¯ÖĞÆ÷Ã¿´ÎÉÏ·¢Êı¾İÖ¡µÄ×î´ó³¤¶È£¬Êı¾İ°üÓÉ¶à¸öÊı¾İÖ¡×é³É£¬Ä¬ÈÏÊı¾İ°ü×î´ó500Byte
		uint8  FrameToFrameTime;			//Ö¡¼ä Ê±¼ä ¼ä¸ô µ¥Î»Îª Ãë 
	}DebugSetPara;

typedef struct{
	uint16 WriteFlag;						//¸ÃÇøÓòÊÇ·ñ±»Ğ´¹ıÊı¾İ
	uint8 TermAddr[6];
	uint8 HostAddr[6];
	DebugSetPara DebugPara;
	/*begin:yangfei added for add support time_on_off*/
	uint8 DeviceType;
	/*end:yangfei added for add support time_on_off*/
	}TermParaSaveType;
	
typedef struct{
	uint16 WriteFlag;						//¸ÃÇøÓòÊÇ·ñ±»Ğ´¹ıÊı¾İ
	uint8 HostIPAddr[4];					//Ö÷Õ¾IPµØÖ·
	uint16 HostPort;						//Ö÷Õ¾¶Ë¿Ú
	uint8 HeartCycle;						//ĞÄÌøÖÜÆÚ£¬µ¥Î»Îª Min
	uint8 RelandDelay;						//µôÏßÖØĞÂµÇÂ¼ÑÓÊ±Ê±¼ä£¬µ¥Î»Îª Min
	uint8 RelandTimes;						//µôÏßÖØĞÂµÇÂ¼´ÎÊı
	}UpCommParaSaveType;

typedef struct{
	uint16 WriteFlag;						//¸ÃÇøÓòÊÇ·ñ±»Ğ´¹ıÊı¾İ
	uint8 MeterReplenishTimes;					//²¹³­±í´ÎÊı£¬·¶Î§0-9.Ä¬ÈÏ²¹³­1´Î¡£
	uint16 MeterInterval;						//²¹³­±íÊ±¼ä¼ä¸ô£¬µ¥Î»ms.·¶Î§200-9999ms.
	uint8 ValveReplenishTimes;					//²¹³­·§´ÎÊı£¬·¶Î§0-9.Ä¬ÈÏ²¹³­1´Î¡£
	uint16 ValveInterval;						//²¹³­·§Ê±¼ä¼ä¸ô£¬µ¥Î»ms.·¶Î§200-9999ms.
	}ReplenishReadMeterSaveType;



typedef struct{
		uint16 MeterSn;
		uint8 HeatMeterAddr[7];				//ÈÈ¼ÆÁ¿±íµØÖ·
	}MeterChannelType;

typedef struct{
		uint8 HeatMeterAddr[7];				//ÈÈ¼ÆÁ¿±íµØÖ·
		uint8 ValveAddr[7];					//·§ÃÅµØÖ·
	}ValveControlType;

typedef struct{
		uint8 HeatMeterAddr[7];				//ÈÈ¼ÆÁ¿±íµØÖ·
		uint8 ControlPanelAddr[7];			//ÎÂ¿ØÃæ°åµØÖ·
	}ControlPanelType;
	
typedef struct{								//³­±í×´Ì¬ĞÅÏ¢
	uint8 TimingStartTime[6];
	uint8 TimingEndTime[6];
	uint16 TimingMeterNums;
	uint16 ReadCmplNums;
	}TimingState;

typedef struct{					//GPRS APN½ÓÈëµãÑ¡ÔñÉèÖÃ¡£
	uint16 WriteFlag;
	uint8 GprsAPNSet;
	}GprsAPNSelect;



#pragma pack()


/********************************ÈÈ¼ÆÁ¿±íÊı¾İ´æ´¢·ÖÅä -- End****************************************/	

extern uint16 gPARA_MeterNum;										//µ±Ç°±íµµ°¸×ÜÊıÁ¿
extern MeterFileType gPARA_MeterFiles[METER_NUM_MAX];				//ÄÚ´æÖĞ´æ´¢µ±Ç°±íµµ°¸ĞÅÏ¢
extern uint8 gPARA_SendIndex_Prevous;								//±¸·İÉÏÒ»°üÊı¾İ°üµÄĞòÁĞºÅ
extern uint16 gPARA_TimeNodes[TIME_NODE_NUM_MAX];
extern uint16 gPARA_TimeNodesNum;


extern uint16 gPARA_MeterChannel[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];			//6¸öÍ¨µÀÈÈ¼ÆÁ¿±íĞÅÏ¢, MeterSn
extern uint8   gPARA_Meter_Failed[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];	/*7¸öÍ¨µÀÊ§°Ü¼ÇÂ¼*/
extern uint16 gPARA_MeterChannelNum[METER_CHANNEL_NUM];								//Ã¿¸öÍ¨µÀ±íÊıÁ¿

extern uint16 gPARA_ConPanelChannel[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];		//6¸öÍ¨µÀÃ¿¸öÍ¨µÀµÄÎÂ¿ØÃæ°åĞÅÏ¢, MeterSn
extern uint16 gPARA_ConPanelChannelNum[METER_CHANNEL_NUM];							//Ã¿¸öÍ¨µÀ¼ÓÔØµÄÎÂ¿ØÃæ°åÊıÁ¿

extern uint16 gPARA_ValveConChannel[METER_CHANNEL_NUM][METER_PER_CHANNEL_NUM];		//6¸öÍ¨µÀÃ¿¸öÍ¨µÀµÄ·§ÃÅĞÅÏ¢, MeterSn
extern uint16 gPARA_ValveConChannelNum[METER_CHANNEL_NUM];							//Ã¿¸öÍ¨µÀ¼ÓÔØµÄ·§ÃÅÊıÁ¿

extern uint16 gPARA_HeatMeterNum; //¼¯ÖĞÆ÷ÖĞÈÈ±í×ÜÊı¡£
extern uint16 gPARA_ValveNum; //¼¯ÖĞÆ÷ÖĞ·§¿ØÆ÷×ÜÊı¡£
extern uint16 gu16ReadHeatMeterNum; //³­±í¹ı³ÌÖĞ£¬ÒÑ¾­³­µÄÈÈ±íÊıÁ¿¼ÆÊı¡£
extern uint16 gu16ReadValveNum;	  //³­±í¹ı³ÌÖĞ£¬ÒÑ¾­³­µÄ·§¿ØÆ÷ÊıÁ¿¼ÆÊı¡£


extern TermParaSaveType gPARA_TermPara;								//¼¯ÖĞÆ÷»ù±¾²ÎÊı
extern UpCommParaSaveType	gPARA_UpPara;							//GPRSÉÏĞĞÍ¨Ñ¶²ÎÊı
extern CommPara uPCommPara;//muxiaoqing
extern ReplenishReadMeterSaveType gPARA_ReplenishPara;

extern GlobalPara gStore_CommPara;
extern uint8 gLOG_SystemLog[1024];									//´æ´¢ÏµÍ³ÈÕÖ¾
extern OS_EVENT *LogQueSem; 										//²úÉúĞÂÈÕÖ¾µÄĞÅºÅÁ¿
extern OS_EVENT *LogWriteSem;										//ÇëÇóĞ´ÈëÈÕÖ¾µÄĞÅºÅÁ¿


/********************³­±íĞÅÏ¢²é¿´ÓëÂÖÏÔÓÃµ½µÄ±äÁ¿****************************/
extern uint8 gu8ReadAllMeterFlag;
extern uint8 gu8ReadDataStore[METER_FRAME_LEN_MAX];

/********************MBUS¶ÌÂ·±£»¤ÓÃµ½µÄÈ«¾Ö±äÁ¿*********************************/
extern uint8 gu8MBusShortRising; //MBUS¶ÌÂ·Ö¸Ê¾Òı½ÅÊÇ·ñ³öÏÖÉÏÉıÑØ£¬0-Ã»ÓĞ£¬1-³öÏÖÉÏÉıÑØ¡£
extern uint8 gu8MBusShortFlag[METER_CHANNEL_NUM];  //±ê¼Ç¶ÔÓ¦MBUSÍ¨µÀÍ¨µÀ¶ÌÂ·,1-¶ÌÂ·£¬0-²»¶ÌÂ·¡£
extern uint8 gu8NowMBUSChannel;    //¼ÇÂ¼µ±Ç°MBUSÍ¨µÀ¡

extern uint8 gu8MBusShortTime;

extern uint8 gu8ReadValveFail;
	

extern uint8 gu8ErrFlag;
extern uint16 gu16ErrOverTime;

extern char  gcYesterdayPath[];

extern char  gTAHeatVluePath[];//ÓÃÓÚ¼ÇÂ¼Í¨¶ÏÊ±¼äÃæ»ı·¨±¾³­±íÊ±¼ä¶Î¸÷»§·ÖÌ¯ÈÈÁ¿´æ·ÅÂ·¾¶¡£




extern uint8 STORE_GetAddrOfMeterData(uint8 *pTime, uint16 MeterSn, uint32 *AddrOut, uint8 Flag);
extern uint8 PARA_ReadMeterInfo(uint16 MeterSn, MeterFileType *pMeterFile);
extern uint8 STORE_CalcTime(uint8 *pReadTime, uint32 *pTimeOffSet, uint8 Flag);
extern uint8 StoreYesterday(void);
extern uint8 STORE_GetTimeNodeInfo(uint16 InTime, uint8 *pNodeIndex, int8 *pNodeOffset);
extern uint8 STORE_FindTimeNodeIndex(uint16 InTime, uint16 *pTimeNode, uint8 *pNodeIndexOut);
extern 	uint8 FindTimeNodeIndex_Forward(uint16 InTime, DataStoreParaType *datastorePara, uint8 *pNodeIndexOut,uint8 *pYesterday);
extern uint8 STORE_InitTimeNodePara(uint8 *pDateTime, uint8 NodeIndex);
extern uint8 STORE_FindDayStartAddr(uint8 *pReadTime, uint32 *pAddrOut);

extern void PARA_InitGlobalVar(void);
extern void GetVipDataFormSD(void);


#endif
