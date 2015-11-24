/*******************************************Copyright (c)*******************************************
**									山东华宇空间技术公司(西安分部)                                **                            
**  文   件   名: Fatsd.c														  				  **
**  创   建   人: yangfei																	  	  **
**	版   本   号: 0.1																		      **
**  创 建  日 期: 2012年9月20日 													     	      **
**  描        述: 项目控制									      						          **
**	修 改  记 录:   							  												  **
****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>
#include <stdio.h>
#include "ff.h"
#include "diskio.h"
#include "fat_sd.h"
#include "HZTable.h"

/**************************************************************************************************/
extern uint8  gSystemTime[6];
extern MSD_CARDINFO CardInfo;
extern uint16 gVersion;
//extern uint8 gDebugModule[20];	

FATFS Fatfs;            /* File system object */
FIL Fil;                /* File object */
BYTE Buff[512];         /* File read buffer */

//char string[1024];
const uint8 gMonthDay[13]={0,31,29,31,30,31,30,31,31,30,31,30,31};
extern uint8 autotestmode ;//muxiaoqing test
extern uint8 manualtestmode ;

void die (                  /* Stop with dying message */
	FRESULT rc          /* FatFs return value */
)
{
	switch(rc)
	{
		case FR_DISK_ERR: 
		if((!autotestmode)&&(!manualtestmode))
			{
			debug( "%-20s\r\n", "FR_DISK_ERR");
										LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %-20s\r\n", "FR_DISK_ERR");
			}							
										break;
		case FR_INT_ERR:  debug( "%-20s\r\n", "FR_INT_ERR");
										LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %-20s\r\n", "FR_INT_ERR");
										break;
		case FR_NOT_READY:  debug( "%-20s\r\n", "FR_NOT_READY");break;
		case FR_NO_FILE:  debug( "%-20s\r\n", "FR_NO_FILE");
										LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %-20s\r\n", "FR_NO_FILE");
										break;
		case FR_NO_PATH:  debug( "%-20s\r\n", "FR_NO_PATH");
										LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %-20s\r\n", "FR_NO_PATH");
										break;
		case FR_INVALID_NAME:  debug( "%-20s\r\n", "FR_INVALID_NAME");break;
		case FR_DENIED:  debug( "%-20s\r\n", "FR_DENIED");break;
		case FR_EXIST:  debug( "%-20s\r\n", "FR_EXIST");
									LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %-20s\r\n", "FR_EXIST");
									break;
		case FR_INVALID_OBJECT:  debug( "%-20s\r\n", "FR_INVALID_OBJECT");break;
		case FR_WRITE_PROTECTED:  debug( "%-20s\r\n", "FR_WRITE_PROTECTED");break;
		case FR_INVALID_DRIVE:  debug( "%-20s\r\n", "FR_INVALID_DRIVE");break;
		case FR_NOT_ENABLED:  debug( "%-20s\r\n", "FR_NOT_ENABLED");break;
		case FR_NO_FILESYSTEM:  debug( "%-20s\r\n", "FR_NO_FILESYSTEM");break;
		case FR_MKFS_ABORTED:  debug( "%-20s\r\n", "FR_MKFS_ABORTED");break;
		case FR_TIMEOUT:  debug( "%-20s\r\n", "FR_TIMEOUT");break;
		case FR_LOCKED:  debug( "%-20s\r\n", "FR_LOCKED");break;
		case FR_NOT_ENOUGH_CORE:  debug( "%-20s\r\n", "FR_NOT_ENOUGH_CORE");break;
		case FR_TOO_MANY_OPEN_FILES:  debug( "%-20s\r\n", "FR_TOO_MANY_OPEN_FILES");
																LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR: %-20s\r\n", "FR_TOO_MANY_OPEN_FILES");
																break;
		case FR_INVALID_PARAMETER:  debug( "%-20s\r\n", "FR_INVALID_PARAMETER");break;
		default:  break;
	}
}

/********************************************************************************************************
**    函 数  名 称: GetFilePath			    							                   
**	函 数  功 能: 根据时间获取抄表时间点数据文件路径                                                             			
**	输 入  参 数:char* path,
**                                 uint8 * time :Hex time
**                                 int16* TimeNodes
**	输 出  参 数: none											                                       
**    返   回   值: none													                               
**	备		  注: char  path[] = "/2012/12/24/1530";
**						  
**    作者: yangfei 2012-12-25
*********************************************************************************************************/
void GetTimeNodeFilePath(char* path,uint8 * time,uint16* TimeNodes)
{
	uint8 NodeIndex ;
	uint16 InTime ;
	
	InTime = ((uint16)HexToBcd(time[ARRAY_HOUR])<<8) + HexToBcd(time[ARRAY_MINUTE]);
	STORE_FindTimeNodeIndex(InTime,TimeNodes, &NodeIndex);/*find Node*/
	time[ARRAY_HOUR] = BcdToHex(TimeNodes[NodeIndex]>>8);
	time[ARRAY_MINUTE]	= BcdToHex(TimeNodes[NodeIndex]&0xff);

	path[3] = DEC_TO_ASCII(time[ARRAY_YEAR]/10);
	path[4] = DEC_TO_ASCII(time[ARRAY_YEAR]%10);
	path[6] = DEC_TO_ASCII(time[ARRAY_MONTH]/10);
	path[7] = DEC_TO_ASCII(time[ARRAY_MONTH]%10);
	path[9] = DEC_TO_ASCII(time[ARRAY_DAY]/10);
	path[10] = DEC_TO_ASCII(time[ARRAY_DAY]%10);
	path[12] = DEC_TO_ASCII(time[ARRAY_HOUR]/10);
	path[13] = DEC_TO_ASCII(time[ARRAY_HOUR]%10);
	path[14] = DEC_TO_ASCII(time[ARRAY_MINUTE]/10);
	path[15] = DEC_TO_ASCII(time[ARRAY_MINUTE]%10);
}


/********************************************************************************************************
**    函 数  名 称: GetFilePath			    							                   
**	函 数  功 能: 根据时间获取最近前一个抄表时间点数据文件路径                                                             			
**	输 入  参 数:char* path,
**                                 uint8 * time :Hex time
**                                 int16* TimeNodes
**	输 出  参 数: none											                                       
**    返   回   值: none													                               
**	备		  注: char  path[] = "/2012/12/24/1530";
**						  
**    作者: yangfei 2012-12-25
*********************************************************************************************************/
void GetTimeNodeFilePath_Forward(char* path,uint8 * time, DataStoreParaType *datastorePara)
{
	char  lcYesterdayPath[] = "/2015/06/09/1200";
	char  YesterdayPath[] = "/2015/06/09/yesterday";
	

	uint8 Res = 0;	
	uint8 Err = 0;
	uint8 NodeIndex ;
	uint16 InTime ;
	uint8 lu8Yesterday = 0;  //是否是昨天。1-昨天。
	
	InTime = ((uint16)HexToBcd(time[ARRAY_HOUR])<<8) + HexToBcd(time[ARRAY_MINUTE]);
	FindTimeNodeIndex_Forward(InTime,datastorePara, &NodeIndex,&lu8Yesterday);    /*find Node*/

	if(lu8Yesterday == 1){    //此处待完善，如果濉板"昨天"牵扯到月份甚至年份的相关处理。
		GetFilePath(YesterdayPath,time,ARRAY_DAY); //time是否应该先转换为BCD码?

		OSMutexPend (FlashMutex, 0, &Err);
		Res = SDReadData(YesterdayPath, (uint8*)path, sizeof(lcYesterdayPath),0);
		OSMutexPost (FlashMutex);

	}
	else{
		path[3] = DEC_TO_ASCII(time[ARRAY_YEAR]/10);
		path[4] = DEC_TO_ASCII(time[ARRAY_YEAR]%10);
		path[6] = DEC_TO_ASCII(time[ARRAY_MONTH]/10);
		path[7] = DEC_TO_ASCII(time[ARRAY_MONTH]%10);
		path[9] = DEC_TO_ASCII(time[ARRAY_DAY]/10);
		path[10] = DEC_TO_ASCII(time[ARRAY_DAY]%10);

	}

		
	time[ARRAY_HOUR] = BcdToHex((datastorePara->TimeNode[NodeIndex])>>8);
	time[ARRAY_MINUTE]	= BcdToHex((datastorePara->TimeNode[NodeIndex])&0xff);


	path[12] = DEC_TO_ASCII(time[ARRAY_HOUR]/10);
	path[13] = DEC_TO_ASCII(time[ARRAY_HOUR]%10);
	path[14] = DEC_TO_ASCII(time[ARRAY_MINUTE]/10);
	path[15] = DEC_TO_ASCII(time[ARRAY_MINUTE]%10);
}


/********************************************************************************************************
**    函 数  名 称: GetFilePath			    							                   
**	函 数  功 能: 根据时间获取抄表时间点数据文件路径                                                             			
**	输 入  参 数:char* path,
**                                 uint8 * time
**                                 int16* TimeNodes
**	输 出  参 数: none											                                       
**    返   回   值: none													                               
**	备		  注: char  path[] = "/2012/12/24/timenode";
**						  
**    作者: yangfei 2012-12-25
*********************************************************************************************************/
void GetTimeNode(uint8 * time,uint16* TimeNodes)
{
	char path[]="/2012/12/24/timenode";
	uint8 Res					= 0x00;
	GetFilePath(path,time,ARRAY_DAY);
	
	OSMutexPend (FlashMutex, 0, &Res);
	Res = SDReadData(path, TimeNodes, 48,8);
	OSMutexPost (FlashMutex);
	if(Res!=NO_ERR)
    	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s SDReadData Error!",__FUNCTION__ );
    	}
}
/****************************************************************************************************
**	函 数  名 称: void GetFilePath(char* path,uint8 time)
**	函 数  功 能: 根据时间获取文件路径
**	输 入  参 数:uint8 time -- 需转发的数据帧;            
**	输 出  参 数: char* path -- 文件路径       
**    返   回   值: void
**	备		  注: //char  path[]="/2012/12/24/1530";
*****************************************************************************************************/
void GetFilePath(char* path,uint8* time,int ARRAY)
{
	path[3] = DEC_TO_ASCII(time[ARRAY_YEAR]/10);
	path[4] = DEC_TO_ASCII(time[ARRAY_YEAR]%10);
	if(ARRAY == ARRAY_MONTH)
	{
		path[6] = DEC_TO_ASCII(time[ARRAY_MONTH]/10);
		path[7] = DEC_TO_ASCII(time[ARRAY_MONTH]%10);
	}
	else if(ARRAY == ARRAY_DAY)
	{
		path[6] = DEC_TO_ASCII(time[ARRAY_MONTH]/10);
		path[7] = DEC_TO_ASCII(time[ARRAY_MONTH]%10);
		path[9] = DEC_TO_ASCII(time[ARRAY_DAY]/10);
		path[10] = DEC_TO_ASCII(time[ARRAY_DAY]%10);
	}
	else if(ARRAY == ARRAY_HOUR)
	{
		path[6] = DEC_TO_ASCII(time[ARRAY_MONTH]/10);
		path[7] = DEC_TO_ASCII(time[ARRAY_MONTH]%10);
		path[9] = DEC_TO_ASCII(time[ARRAY_DAY]/10);
		path[10] = DEC_TO_ASCII(time[ARRAY_DAY]%10);
		path[12] = DEC_TO_ASCII(time[ARRAY_HOUR]/10);
		path[13] = DEC_TO_ASCII(time[ARRAY_HOUR]%10);
		path[14] = DEC_TO_ASCII(time[ARRAY_MINUTE]/10);
		path[15] = DEC_TO_ASCII(time[ARRAY_MINUTE]%10);
	}
}

/********************************************************************************************
**    函 数  名 称: uint8 SDSaveData(const char* path, const void* buffer, uint16 NumByteToWrite,uint16 MeterSn)			    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
uint8 SDSaveData(const char* path, const void* buffer, uint16 NumByteToWrite,uint32 off)
{
	FRESULT rc;				/* Result code */
	UINT bw;
	
	rc = f_open(&Fil, path, FA_WRITE | FA_OPEN_ALWAYS);
	if (rc) die(rc);

	f_lseek(&Fil,off);  //指针指向0ff
	//f_lseek(&Fil,Fil.fsize);  //指针指向文件尾
		
    rc = f_write(&Fil,buffer,NumByteToWrite, &bw);
    if (rc) 
	{
		die(rc);
		return ERR_1  ;
    }
	/*begin:yangfei modified 2013-01-08*/	
	//f_sync(&Fil);	
	rc = f_close(&Fil);
	if (rc) die(rc);
	/*end:yangfei modified 2013-01-08*/
	return  NO_ERR;
}
/********************************************************************************************
**    函 数  名 称: SDSaveUpdateData			    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
uint8 SDSaveUpdateData(const char* path, const void* buffer, uint16 NumByteToWrite,uint32 off)
{
	FRESULT rc;				/* Result code */
	UINT bw;
	
	rc = f_open(&Fil, path, FA_WRITE | FA_CREATE_ALWAYS);
	if (rc) die(rc);

	f_lseek(&Fil,off);  //指针指向0ff
	//f_lseek(&Fil,Fil.fsize);  //指针指向文件尾
		
    rc = f_write(&Fil,buffer,NumByteToWrite, &bw);
    if (rc) 
	{
		die(rc);
		return ERR_1  ;
    }
	/*begin:yangfei modified 2013-01-08*/	
	//f_sync(&Fil);	
	rc = f_close(&Fil);
	if (rc) die(rc);
	/*end:yangfei modified 2013-01-08*/
	return  NO_ERR;
}
/********************************************************************************************
**    函 数  名 称: uint8 SDSaveData(const char* path, const void* buffer, uint16 NumByteToWrite,uint16 MeterSn)			    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
uint8 SDSaveLog(const char* path, const void* buffer, uint16 NumByteToWrite,uint32 off)
{
	FRESULT rc;				/* Result code */
	UINT bw;
	off = off;
	rc = f_open(&Fil, path, FA_WRITE | FA_OPEN_ALWAYS);
	if (rc) 
	{
		die(rc);
		return  ERR_1;
	}
	f_lseek(&Fil,Fil.fsize);  //指针指向文件尾
	if (rc) 
	{
		die(rc);
		return ERR_2;
	}	
    rc = f_write(&Fil,buffer,NumByteToWrite, &bw);
    if (rc) 
	{
		die(rc);
		return ERR_3  ;
    }
	/*begin:yangfei modified 2013-01-08*/	
	//f_sync(&Fil);	
	rc = f_close(&Fil);
	if (rc) die(rc);
	/*end:yangfei modified 2013-01-08*/
	return  NO_ERR;
}
/********************************************************************************************
**    函 数  名 称: uint8 SDReadData(const char* path, const void* buffer, uint16 NumByteToRead,uint32 off)		    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
uint8 SDReadData(const char* path, void* buffer, uint16 NumByteToRead,uint32 off)
{
	FRESULT rc;				/* Result code */
	UINT br;
	
	rc = f_open(&Fil, path, FA_READ );
	if (rc) 
	{
	{
	//	ShowLineChar_HZ(4,10,&ClockErr[0],7);//集中器管理与维护
	//	delay(0x00200000);
	}
		die(rc);
		return  ERR_1;
	}

	rc = f_lseek(&Fil,off);  //指针指向0ff
	if (rc) 
	{
		die(rc);
		return ERR_2;
	}
	rc = f_read(&Fil,buffer,NumByteToRead, &br);	
    if (rc) 
	{
		die(rc);
		return  ERR_3;
    }
	/*begin:yangfei modified 2013-01-08*/	
	//f_sync(&Fil);	
	rc = f_close(&Fil);
	
	if (rc)
		{
		die(rc);
		}
	/*end:yangfei modified 2013-01-08*/
	return  NO_ERR;
}
/********************************************************************************************
**    函 数  名 称: void mountSd(void)				    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	  如果FAT表破坏则进行格式化                                                                                 																									
********************************************************************************************/
void mountSd(void)
{
	FRESULT res;
    
    f_mount(0, &Fatfs);
    res = f_open(&Fil, "/test", FA_READ );
	if (res == FR_NO_FILESYSTEM) 
	{
		res=f_mkfs(0, 0,4096);/*format SDCard*/
       if ( res!=FR_OK) 
    	{ 
    		die(res);
            debug("f_mkfs error\r\n");
    		return;
    	}
	}
  else
    {
    die(res);
    debug("%s  %d res=%d\r\n",__FUNCTION__,__LINE__,res);
    }
       
	res=f_mount(0, &Fatfs);
	if ( res!=FR_OK) 
	{ 
		die(res);
		debug("%s  %d res=%d\r\n",__FUNCTION__,__LINE__,res);
		return;
	}
}

/********************************************************************************************
**    函 数  名 称: void MeterDataPro(void)		    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: BcdTime  							         			 	       
**	输 出  参 数: HexTime											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void BcdTimeToHexTime(uint8* BcdTime,uint8* HexTime)
{
	HexTime[ARRAY_YEAR] = BcdToHex(BcdTime[ARRAY_YEAR]);
	HexTime[ARRAY_MONTH] = BcdToHex(BcdTime[ARRAY_MONTH]);
	HexTime[ARRAY_DAY] = BcdToHex(BcdTime[ARRAY_DAY]);
	HexTime[ARRAY_HOUR] = BcdToHex(BcdTime[ARRAY_HOUR]);
	HexTime[ARRAY_MINUTE] = BcdToHex(BcdTime[ARRAY_MINUTE]);
	HexTime[ARRAY_SECOND] = BcdToHex(BcdTime[ARRAY_SECOND]);
}

/********************************************************************************************
**    函 数  名 称: MakeDir    							                       
**	函 数  功 能: 开机目录初始化                                                     			
**	输 入  参 数: Month  							         			 	       
**	输 出  参 数: none										                                       
**    返   回   值    : none													                               
**	备		     注: 	                           																									
********************************************************************************************/
void MakeDir(char* dir)
{
    FRESULT res;
    res=f_mkdir(dir);  
    if ( res!=FR_OK&& res!=FR_EXIST) 
	{ 
	   die(res);
	}
}
uint8 MakeFile(const char* path)
{
	FRESULT res;
	res = f_open(&Fil, path, FA_WRITE | FA_OPEN_ALWAYS);
	if ( res!=FR_OK) 
    	{ 
    		debug( "creat new File Error!\r\n");
		    return;
        }
	res = f_close(&Fil);
	if (res) die(res);
	return res;
}
/********************************************************************************************
**    函 数  名 称: MakeDirInit	    							                       
**	函 数  功 能: 开机目录初始化                                                     			
**	输 入  参 数: Month  							         			 	       
**	输 出  参 数: none										                                       
**    返   回   值    : none													                               
**	备		     注: 	                           																									
********************************************************************************************/
void MakeDirInit(uint8 Month)
{
	FRESULT res;
	uint8 Err 		= 0x00;
	uint8 SystemTime[6]={0};
	char YearPath[] = "/2012";
	char MonthPath[] = "/2012/12";
	char DayPath[] = "/2012/12/26";
	CPU_SR			cpu_sr;	
	static int DirInitFlag = FALSE;
	LOG_assert_param( Month > 12 );

	OS_ENTER_CRITICAL();
	memcpy(SystemTime,gSystemTime,6);
	OS_EXIT_CRITICAL();
	if((DirInitFlag == FALSE)||(Month == 1))   /*开机初始化目录创建当年当月及本月所有目录*/
	{
		DirInitFlag = TRUE;
		
		GetFilePath(YearPath,SystemTime, ARRAY_YEAR);
		OSMutexPend (FlashMutex,0,&Err);
		res=f_mkdir(YearPath);  /*creat year path*/
		OSMutexPost (FlashMutex);
		if ( res!=FR_OK&& res!=FR_EXIST) 
		{ 
		  // LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR:res=%d %s %d",res, __FUNCTION__,__LINE__);
		  return;
		}
	}
	
		GetFilePath(MonthPath,SystemTime, ARRAY_MONTH);/*creat month path*/
		OSMutexPend (FlashMutex,0,&Err);
		res=f_mkdir(MonthPath);  
		OSMutexPost (FlashMutex);
		if ( res!=FR_OK&& res!=FR_EXIST) 
		{ 
		  // LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR:res=%d %s %d",res, __FUNCTION__,__LINE__);
		  return;
		}

		OSMutexPend (FlashMutex,0,&Err);
		for( ;SystemTime[ARRAY_DAY]<=gMonthDay[SystemTime[ARRAY_MONTH]];SystemTime[ARRAY_DAY]++) /*creat day path*/
		{
			GetFilePath(DayPath,SystemTime, ARRAY_DAY);
			res=f_mkdir(DayPath); 
			if ( res!=FR_OK&& res!=FR_EXIST) 
			{ 
			  // LOG_WriteSysLog_Format(LOG_LEVEL_ERROR, "ERROR:res=%d %s %d",res, __FUNCTION__,__LINE__);
			   //return;
			   return;
			}
		}
		OSMutexPost (FlashMutex);
	
}

void test_path(void)
{
  uint16 vertion=0x0201;
 
  uint8 UpdatePath[5] ="1234";
  UpdatePath[3] =  DEC_TO_ASCII(vertion&0x0f);
  UpdatePath[2] = DEC_TO_ASCII(vertion>>4&0x0f);
  UpdatePath[1] = DEC_TO_ASCII(vertion>>8&0x0f);
  UpdatePath[0] = DEC_TO_ASCII(vertion>>12&0x0f);
 
  debug("%s \r\n",UpdatePath);
   
}
/********************************************************************************************************
**    函 数  名 称: void UartCMDHelp(void)				    							                       
**	函 数  功 能: help                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
*********************************************************************************************************/
void UartCmdHelp(u8 argc,void **argv)
{
	u8 i=0;
	debug("You entered the parameter: ");
	for ( i=0; i<argc; i++ )
	{
		debug( argv[i] );
		debug( "  " );
	}
	debug( "\r\n" );
	debug("\r\nCMD support:\r\n");
	debug("help:\r\n");
	debug("cls:\r\n");
	debug("ls:\r\n");
	debug("pwd:\r\n");
	debug("cd:\r\n");
	debug("cat:\r\n");
	debug("mkdir:\r\n");
	debug("rm:\r\n");
	debug("mv:\r\n");
	debug("sdwr:\r\n");
	debug("new:\r\n");
	debug("time:\r\n");
    debug("set:\r\n");
    debug("reset:\r\n");
    debug("channel:\r\n");
  
	debugX(LOG_LEVEL_ERROR,"LOG_LEVEL_ERROR  test\r\n");
	debugX(LOG_LEVEL_INFO,"LOG_LEVEL_INFO  test\r\n");
    test_path();
}

void UartCmdCls(u8 argc,void **argv)
{
       Uart_ClearScreen();	
}
/********************************************************************************************
**    函 数  名 称: void UartCmdcd(u8 argc,void **argv)			    							                       
**	函 数  功 能:                                                       			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdcd(u8 argc,void **argv)
{
	FRESULT rc;
	char path[50];
     if(argc == 1)
      {
      debug("need a param  \r\n");
      return;
      }
	rc=f_chdir(argv[1]);  
	if ( rc!=FR_OK) 
	{
		die(rc);
		debug("%s %d f_chdir Error!\r\n",__FUNCTION__,__LINE__);
		return;
	}
	debug("f_chdir succesed!\r\n");	   	
	f_getcwd(path, 50);
	debug("path = %s \r\n",path);	
}
/********************************************************************************************
**    函 数  名 称: void CMDmkdir(void)				    							                       
**	函 数  功 能:                                                       			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdmkdir(u8 argc,void **argv)
{
	FRESULT rc;
    if(argc == 1)
      {
        debug( "need a param\r\n");
        debug("Example:mkdir dirname  \r\n");
      }
    else
      {
        rc=f_mkdir(argv[1]);  
    	if ( rc!=FR_OK&& rc!=FR_EXIST) 
    	{ 
    	   debug( "Make Dir Error!\r\n");
    	   die(rc);
    	   return;
    	}
      }
}
/********************************************************************************************
**    函 数  名 称: void CMDls(void)				    							                       
**	函 数  功 能: 此函数可以显示当前路径下的所有文件和文件夹                                                      			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdls(u8 argc,void **argv)
{
	FRESULT rc;				/* Result code */
	DIR dir;				/* Directory object */
	FILINFO fno;			/* File information object */
	char path[50];                 /*currunt path*/
	/*begin:yangfei added 2013-01-07 for null pointer*/
	#if _USE_LFN
	char lfname[50];
	fno.lfname=lfname;
    fno.lfsize = 50;
	#endif
	/*end:yangfei added 2013-01-07  */
	f_getcwd(path, 50);
	rc = f_opendir(&dir, path);
	if (rc) die(rc);

	for (;;) {
		rc = f_readdir(&dir, &fno);		/* Read a directory item */
		if (rc || !fno.fname[0]) break;	/* Error or end of dir */
		if (fno.fattrib & AM_DIR)
		{
            if(fno.lfname[0])
              {
              debug("   <dir>  %s\r\n", fno.lfname);
              }
            else
              {
              debug("   <dir>  %s\r\n", fno.fname);
              }   
       	}
		else
		{
			
            if(fno.lfname[0])
              {
                debug("%8lu  %s\r\n", fno.fsize, fno.lfname);
              }
			else
			  {
			    debug("%8lu  %s\r\n", fno.fsize, fno.fname);
			  }
		}
	}
	if (rc) die(rc);

	//debug("("__TIME__","__DATE__")\r\n");
	//debug("%s  %s  %d \r\n", __FILE__,__FUNCTION__,__LINE__);
}
/********************************************************************************************
**    函 数  名 称: void CMDmv(void)				    							                       
**	函 数  功 能: 此函数可以移动或重命名一个文件或文件夹                                                       			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdmv(u8 argc,void **argv)
{
	FRESULT res;
    if(argc != 3)
      {
      debug("mv  path_old  path_new \r\n");
      return;
      }
	res=f_rename(argv[1],argv[2]); //改变文件名称。
	if ( res!=FR_OK) 
	{ 
	    debug( "Rename File Error!\r\n");
		die(res);
		return;
	}
}

/*

*/
void UartCmdCp(u8 argc,void **argv)
{
	FRESULT rc;
    UINT br,bw;
    FIL SrcFil,DestFil;  
    int i;
    uint8 Err = 0;
    if(argc != 3)
      {
      debug("cp  src_file  dest_file \r\n");
      return;
      }
	memset(Buff,0,sizeof(Buff));
    UartCmdtime(0,0);
	OSMutexPend (FlashMutex,0,&Err);
    rc = f_open(&SrcFil, argv[1], FA_READ);
	if (rc) 
	  {
	  die(rc);return;
	  }
	rc = f_open(&DestFil, argv[2], FA_WRITE | FA_OPEN_ALWAYS);
	if (rc)
	  {
	  die(rc);return;
	  }
	//f_lseek(&Fil,Fil.fsize);  //指针指向文件尾
	//debug("文件指针位置：%d \r\n",Fil.fptr);
	for(i=0;i<=SrcFil.fsize/sizeof(Buff);i++)  
	{
      rc = f_read(&SrcFil,Buff,sizeof(Buff), &br);
	    if (rc) 
		{
			die(rc);return;
	    }
      rc = f_write(&DestFil,Buff,br, &bw);
	    if (rc) 
		{
			die(rc);return;
	    }
	}
	OSMutexPost (FlashMutex);
	rc = f_close(&SrcFil);
	if (rc) die(rc);
	rc = f_close(&DestFil);
	if (rc) die(rc);
    UartCmdtime(0,0);
}
/********************************************************************************************
**    函 数  名 称: void UartCmdSDWrite(u8 argc,void **argv)				    							                       
**	函 数  功 能: 此函数可以移动或重命名一个文件或文件夹                                                       			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdSDWrite(u8 argc,void **argv)
{
	FRESULT rc;				/* Result code */
	//DIR dir;				/* Directory object */
	//FILINFO fno;			/* File information object */
	UINT bw,  i;
	uint8 Err = 0;
	memset(Buff,100,512);

	OSMutexPend (FlashMutex,0,&Err);
	
	rc = f_open(&Fil, argv[1], FA_WRITE | FA_OPEN_ALWAYS);
	if (rc) die(rc);
	//f_lseek(&Fil,Fil.fsize);  //指针指向文件尾

	debug("文件指针位置：%d \r\n",Fil.fptr);

	#if 1
	for(i=0;i<10;i++)  /* 1 M 数据*/
	{
	    rc = f_write(&Fil,Buff,sizeof(Buff), &bw);
	    if (rc) 
		{
			die(rc);
			return;
	    }
	}
	f_sync(&Fil);
	#endif
	
	OSMutexPost (FlashMutex);
	
	rc = f_close(&Fil);
	if (rc) die(rc);

}
/********************************************************************************************
**    函 数  名 称: void CMDpwd(void)				    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdpwd(u8 argc,void **argv)
{
	char path[50];
	FRESULT res;
	res = f_getcwd(path, 50);
	if ( res!=FR_OK) 
	{ 
	    debug( "f_getcwd Error!\r\n");
		die(res);
		return;
	}
	debug("path = %s\r\n",path);	
}
/********************************************************************************************
**    函 数  名 称: void UartCmdcat(void)				    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdcat(u8 argc,void **argv)
{
	FRESULT rc;				/* Result code */
	UINT  br;
	uint8 Err 			= 0;
	debug("\nOpen an existing file (message.txt).\r\n");	
     if(argc == 1)
      {
       debug("need a param  \r\n");
      debug("Example:cat filename  \r\n");
      return;
      }
	OSMutexPend (FlashMutex,0,&Err);
	rc = f_open(&Fil, argv[1], FA_READ); 
	if (rc) 
	{
		die(rc);
		return;
	}
	debug("\nType the file content.\r\n");
	for (;;) {
		rc = f_read(&Fil, Buff, sizeof(Buff), &br);	// Read a chunk of file 
		if (rc || !br) break;			// Error or end of file 
		//for (i = 0; i < br; i++)		// Type the data 
			//putchar(Buff[i]);
			//Buff[sizeof(Buff)-1] = 0;
			debug("%s",Buff);
		#if 0
			if(br == sizeof(Buff))
			{
			   	debug("%s",Buff);
			        debug("\b");
			}
			else
			{
				for (i = 0; i < br; i++)	
			       {debug("%s",Buff[i]);}
			}
		#endif
	}
	if (rc) die(rc);
	OSMutexPost (FlashMutex);
	debug("\r\nClose the file.\r\n");
	rc = f_close(&Fil);
	if (rc) die(rc);
}
/********************************************************************************************
**    函 数  名 称: void UartCmdrm(void)				    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdrm(u8 argc,void **argv)
{
	FRESULT rc;
    if(argc == 1)
      {
      debug("need a param  \r\n");
      debug("Example:rm filename  \r\n");
      return;
      }
	rc=f_unlink(argv[1]);  //只能删除文件和空目录。
	if ( rc != FR_OK) 
	{ 
		debug( "Delete File Error!\r\n");
		die(rc);
		return;
	}
	debug( "File Deleted!\r\n");
}
/********************************************************************************************
**    函 数  名 称: void UartCmdrm(void)				    							                       
**	函 数  功 能: currut path                                                         			
**	输 入  参 数: none  							         			 	       
**	输 出  参 数: none											                                       
**    返   回   值    : none													                               
**	备		     注: 	                                                                                   																									
********************************************************************************************/
void UartCmdnew(u8 argc,void **argv)
{
	FRESULT res;
    if(argc == 1)
      {
      debug("need a param  \r\n");
      debug("Example:new filename  \r\n");
      return;
      }
	res = f_open(&Fil, argv[1], FA_WRITE | FA_OPEN_ALWAYS);
	if ( res!=FR_OK) 
    	{ 
    		debug( "creat new File Error!\r\n");
		return;
        }
        debug( "creat new file %s OK!\r\n",argv[1]);
	res = f_close(&Fil);
	if (res) die(res);
}


void Uart_ClearScreen(void)
{
    debug ( "\r\n\n\n\n\n\n\n\n\n\n");
	debug ( "\r\n\n\n\n\n\n\n\n\n\n");
	debug( "\r\n\n\n\n\n\n\n\n\n\n");
  
    MSD_GetCardInfo(&CardInfo);
    //debug_debug(gDebugModule[SD_MODULE],"cardinfo.Capacity=%x %x",cardinfo.Capacity,cardinfo.BlockSize);
}
void UartCmdtime(u8 argc,void **argv)
{
    CPU_SR		cpu_sr;
	uint8 SystemTime[6] = {0};
   
    OS_ENTER_CRITICAL();
	memcpy(SystemTime, gSystemTime, 6);
	OS_EXIT_CRITICAL();
    Uart_Printf("current time:20%d-%d-%d %d:%d:%d\r\n",SystemTime[ARRAY_YEAR],SystemTime[ARRAY_MONTH],
      SystemTime[ARRAY_DAY],SystemTime[ARRAY_HOUR],SystemTime[ARRAY_MINUTE],SystemTime[ARRAY_SECOND]);
		//AddrChange();
	/**/
  //UART5Send("yangfei\r\n",10);
}

void UartCmdSetDebugLevel (u8 argc,void **argv)
{
   uint8 Module,DebugLevel;
   if(argc != 3)
      {
         Uart_Printf("TASKUP_MODULE    0\r\n");
         Uart_Printf("FATFS_MODULE    1\r\n");
         Uart_Printf("SD_MODULE    2\r\n");
         Uart_Printf("GPRS_MODULE    3\r\n");
         Uart_Printf("TASKDOWN_MODULE    4\r\n");
         Uart_Printf("UPDATE_MODULE    5\r\n"); 
         Uart_Printf("ALL_MODULE    6\r\n"); 
         //Uart_Printf("gLOG_LogLevelFlag    9  erro=1 warn=2 info=4 debug=8\r\n"); 
		 Uart_Printf("METER_DATA    7\r\n"); 
		 Uart_Printf("EN13757    8\r\n");
		 Uart_Printf("TIME_AREA    9\r\n");
        
         Uart_Printf("print level:\r\n"); 
          
         Uart_Printf("KERN_WARNING	4	\r\n");          
         Uart_Printf("KERN_INFO	6\r\n"); 
         Uart_Printf("KERN_DEBUG	 7 \r\n"); 
         
         Uart_Printf("format:set <MODULE> <level> example:set  3 7  \r\n");
      }
    else
      {
        //gDebugLevel = *(uint8*)argv[1]-'0';
        //Uart_Printf( "gDebugLevel=%d\r\n",gDebugLevel);	  
        Module = *(uint8*)argv[1]-'0';
        DebugLevel = *(uint8*)argv[2]-'0';

		debug("Module = %d\r\n",Module);
		#if 0
        if(Module == 9)
          {
            gLOG_LogLevelFlag = DebugLevel;
            Uart_Printf( "gDebugLevel=%d\r\n",gLOG_LogLevelFlag);
          }
		#endif
        gDebugModule[Module]=DebugLevel ;
        debug_debug(gDebugModule[TASKUP_MODULE],"TASKUP_MODULE print ok\r\n");
        debug_debug(gDebugModule[FATFS_MODULE],"FATFS_MODULE print ok\r\n");	
        debug_debug(gDebugModule[SD_MODULE],"SD_MODULE print ok\r\n");	
        debug_debug(gDebugModule[GPRS_MODULE],"GPRS_MODULE print ok\r\n");
        debug_debug(gDebugModule[TASKDOWN_MODULE],"TASKDOWN_MODULE print ok\r\n");	
        debug_debug(gDebugModule[UPDATE_MODULE],"UPDATE_MODULE print ok\r\n");	
        debug_debug(gDebugModule[ALL_MODULE],"ALL_MODULE print ok\r\n");	
      }
}
void UartCmdReset(u8 argc,void **argv)
{
        NVIC_SETFAULTMASK();
	    NVIC_GenerateSystemReset();
}

void UartCmdSetBKP(u8 argc,void **argv)
{
     BKP_WriteBackupRegister(BKP_DR3, 0x00);
}
/*

*/
void UartCmdTest(u8 argc,void **argv)
{
     uint8 argc1=0xff;
     if(argc != 2)
      {
         Uart_Printf("zigbee    0\r\n");
         Uart_Printf("485: BaudRate:2400  EVEN  1\r\n");
    
      }
     else
      {
      argc1 = *(uint8*)argv[1]-'0';
       //uint8 SendBuff[]={0xFB,0xFB,0x7B,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x34,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x02,0x13,0x10,0x27,0x03,0x13,0x2E,0x00,0xFD,0xFD};

    //UpDevSend(UP_COMMU_DEV_ZIGBEE,  SendBuff,  sizeof(SendBuff)); 
        if(argc1 == 1)
          {
          uint8 SendBuff[]={0xfe, 0xfe, 0x68, 0x20, 0x20, 0x58, 0x35, 0x72, 0x69, 0x55, 0x66, 0x04, 0x04, 0xa0, 0x17, 00, 0x99, 0x23, 0x16};
          UpDevSend(UP_COMMU_DEV_485,  SendBuff,  sizeof(SendBuff)); 
          }
      }
   
}
/*

*/
void UartCmdVersion(u8 argc,void **argv)
{
     Uart_Printf("\r\nVersion %d.%d%d",(gVersion>>8)&0x0f,(gVersion>>4)&0x0f,gVersion&0x0f);
     Uart_Printf("\r\nCompile time:("__TIME__","__DATE__")\r\n");
}

void UartSetChannel(u8 argc,void **argv)
{
      uint8 argc1=0xff;
      
      if(argc != 2)
      {
         Uart_Printf("open channel  1:channel  1\r\n");
      }
     else
      {
        argc1 = *(uint8*)argv[1]-'0';
 
        if(argc1>=1&&argc1<=6)
          {
           //切换Mbus通道
	     METER_ChangeChannel(argc1);		
            Uart_Printf("METER_ChangeChannel  to %d\r\n",argc1);
          }
        else
          {
          Uart_Printf("Error:METER_ChangeChannel = %d \r\n",argc1);
          }
      }
       #if 0
       {
       // uint8 DataBuf[]={0x0C,0x0F,0x00,0x01,0x00,0x00,0x8C,0x10,0x0F,0x00,0x00,0x00,0x00,0x8C,0x20,0x14,0x00,0x00,0x00,0x00
         // ,0x0C,0x14,0x00,0x00,0x00,0x00,0x3C,0x2B,0xBD,0xEB,0xDD,0xDD,0x3B,0x3B,0xBD,0xEB,0xDD,0x0A,0x5A,0x23,0x02,0x0A,0x5E,0x17,0x02};
        uint8 DataBuf[]={0x0a,0x0F,0x24,0x03,0x00,0x00,   
          0x0C,0x14,0x75,0x04,0x00,0x00,
          0x0B,0x3B,0x78,0x56,0x34,
          0x0A,0x5A,0x23,0x02,
          0x0A,0x5E,0x17,0x02};
        uint8 err=0;
        METER_Data_To_CJ188Format(6,DataBuf,&err);
          {
             uint8 DataBuf[]={0x0c,0x06,0x24,0x03,0x00,0x00,   
          0x0d,0x14,0x75,0x04,0x00,0x00,
          0x0d,0x3B,0x78,0x56,0x34,
          0x0A,0x5A,0x23,0x02,
          0x0A,0x5E,0x17,0x02};
        uint8 err=0;
        METER_Data_To_CJ188Format(6,DataBuf,&err);
        }

         {
           uint8 DataBuf[]={0x0c,0xfb,0x0e,0x24,0x03,0x00,0x00,   
          0x0C,0x14,0x75,0x04,0x00,0x00,
          0x0B,0x3B,0x78,0x56,0x34,
          0x0A,0x5A,0x23,0x02,
          0x0a,0x5E,0x17,0x02};
        uint8 err=0;
        METER_Data_To_CJ188Format(6,DataBuf,&err);
         }
         {
           uint8 DataBuf[]={0x0c,0xfb,0x81,0x72,0x24,0x03,0x00,0x00,   
          0x0C,0x14,0x75,0x04,0x00,0x00,
          0x0B,0x3B,0x78,0x56,0x34,
          0x0a,0x5A,0x23,0x02,
          0x0A,0x5E,0x17,0x02};
        uint8 err=0;
        METER_Data_To_CJ188Format(6,DataBuf,&err);
         }

        debug("METER_Data_To_CJ188Format err = %d \r\n",err);
       }
       #endif
   
       {
       // uint8 DataBuf[]={0x0C,0x0F,0x00,0x01,0x00,0x00,0x8C,0x10,0x0F,0x00,0x00,0x00,0x00,0x8C,0x20,0x14,0x00,0x00,0x00,0x00
         // ,0x0C,0x14,0x00,0x00,0x00,0x00,0x3C,0x2B,0xBD,0xEB,0xDD,0xDD,0x3B,0x3B,0xBD,0xEB,0xDD,0x0A,0x5A,0x23,0x02,0x0A,0x5E,0x17,0x02};
        uint8 DataBuf[]={0x0C, 0x78, 0x00, 0x00, 0x0, 0x00, 
                                    02, 0xFD, 0x0E, 0x04, 0x00,
                                    02, 0x59, 0xB8, 0x0A, 
                                    02, 0x5D, 0xBE, 0x0A, 
                                    02, 0x61, 0x06, 0x80, 0x01, 0x71, 0x05,
                                    04, 0x3B, 0x00, 0x01, 0x0, 0x00, 
                                    0x14, 0x3B, 0x16, 0x00, 0x00, 0x00,
                                    0x54, 0x3B, 0x00, 0x0, 0x00, 0x00, 
                                    0x94, 0x01, 0x3B, 0x00, 0x00, 0x00, 0x00, 
                                    04, 0x2B, 0x00, 0x01, 0x00, 0x00, 
                                    0x14, 0x2B, 0x03, 0x00, 0x00, 0x00, 
                                    0x54, 0x2B, 0x00, 0x00, 0x00, 0x00, 
                                    0x94, 0x01, 0x2B, 0x00, 0x00, 0x00, 0x00, 
                                    04, 0x14, 0x2B, 0x00, 0x00, 0x00, 
                                    0x44, 0x14, 0x23, 0x00, 0x00, 0x00, 
                                    0x84, 0x01, 0x14, 0x00, 0x00, 0x00, 0x00, 
                                    0x42, 0xEC, 0x7E, 0xA1, 0x11, 
                                    0x82, 0x01, 0xEC, 0x7E, 0x00, 0x00, 
                                    0x34, 0x26, 0x00, 0x00, 0x00, 0x00, 
                                    04, 0x26, 0xCF, 0x22, 0x0, 0x00, 
                                    04, 0x6D, 0x38, 0x0F, 0x9A, 0x16, 
                                    0x04, 0x06, 0x0, 0x01, 0x00, 0x00,
                                    0,0,0,0,0,0,0
                                    };
        uint8 err=0;
        METER_Data_To_CJ188Format(7,DataBuf,100,&err);
   
        }
       #if 0
        {
          uint32  result=0;
          uint8  data[4]={0x78,0x56,0x34,0x12};
          
           result= HexToBcdUint32(5678) ;

           debug("result = %x \r\n",result);

           result= BcdToHex_16bit1(0x12345678) ;

           debug("result = %d \r\n",result);

           result= BcdToHex_16bit1((uint32)data[0]) ;

           debug("result = %d \r\n",result);

           
          
         }
       #endif
       #if 0
      {
       int ret=0;
        char  meterAddr[4]={0x73,0x60,0x72,0x44};
        DELU_Protocol ProtocoalInfo;
        memcpy(ProtocoalInfo.MeterAddr, meterAddr,4);	
         ret=HYDROMETER(&ProtocoalInfo,0);
        debug("ret = %d\r\n",ret);
      }
       #endif
    
  
}






