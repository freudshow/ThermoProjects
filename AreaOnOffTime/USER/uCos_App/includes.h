/****************************************Copyright (c)****************************************************
**                                      
**                                 山东华宇空间技术公司(西安分部) 
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               includes.h
** Descriptions:            includes.h for ucos
**
**--------------------------------------------------------------------------------------------------------

** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/


#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

#ifdef __cplusplus
	extern "C" {
	#endif

/* Includes ------------------------------------------------------------------*/
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <math.h>
//#include  <assert.h>
#include  <uCOS-II\Source\ucos_ii.h>
#include  <uCOS-II\Ports\ARM-Cortex-M3\RealView\os_cpu.h>
#include  <uC-CPU\ARM-Cortex-M3\RealView\cpu.h>
#include  <lib_def.h>
#include  <lib_mem.h>
#include  <lib_str.h>
#include  <stm32f10x.h>
#include  <stm32f10x_conf.h>
/*begin:yangfei added 2013-01-17*/
#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_crc.h"
/*end:yangfei added 2013-01-17*/
#include "userconfig.h"
#include "bsp.h"
#include "StructProtocol.h" 
#include "public.h"
#include "app_down.h"
#include "app_flashmem.h"
#include "SysLog.h"
/*begin:yangfei added 2012-12-15*/
#include "fat_sd.h"
#include "task_uart_shell.h"
/*end:yangfei added 2012-12-15*/

#include "Valve.h"  
#include "valve_elsonic.h"
#include "valve_joyo.h"
#include "valve_binihi.h"



#ifdef __cplusplus
	}
	#endif

#ifdef  LOG_FULL_ASSERT
#define LOG_assert_param(expr) ((expr) ? LOG_assert_failed((uint8_t *)__FILE__, __LINE__) : (void)0 )
#else
#define LOG_assert_param(expr) ((void)0)
#endif /* LOG_FULL_ASSERT */

#define     TIME_AREA_FUC           0x01
#define		LOG_LEVEL_ERROR			0x01
#define		LOG_LEVEL_WARN			0x02
#define		LOG_LEVEL_INFO			    0x04
#define		LOG_LEVEL_DEBUG			0x08

#define		LOG_ALL		(LOG_LEVEL_ERROR|LOG_LEVEL_WARN|LOG_LEVEL_INFO|LOG_LEVEL_DEBUG)
#define		LOG_NONE				0x00
/*begin:yangfei added*/
#define       TASKUP_MODULE    0    /*集中器与主站的通信数据*/
#define       FATFS_MODULE    1
#define       SD_MODULE    2
#define       GPRS_MODULE    3
#define       TASKDOWN_MODULE    4  /*集中器与热表的通信数据*/
#define       UPDATE_MODULE    5
#define       ALL_MODULE    6
#define       METER_DATA      7
#define       EN13757      8
#define       TIME_AREA      9
#define       HEAT_COST      10





#define	KERN_EMERG	0	/* system is unusable			*/
#define	KERN_ALERT	1	/* action must be taken immediately	*/
#define	KERN_CRIT	    2	/* critical conditions			*/
#define	KERN_ERR	    3	/* error conditions			*/
#define	KERN_WARNING	4	/* warning conditions			*/
#define	KERN_NOTICE	5	/* normal but significant condition	*/
#define	KERN_INFO	6	/* informational			*/
#define	KERN_DEBUG	7	/* debug-level messages			*/

/*begin:yangfei added*/
extern uint8 gDebugLevel ;
extern uint8 gDebugModule[20];	
//#define DEBUG   LOG_LEVEL_ERROR
#ifdef DEBUG
#define debug(fmt,args...)   if(gDebugLevel  >= 1) Uart_Printf(fmt,##args)
#define debugX(level,fmt,args...)   if(gDebugLevel >= level) Uart_Printf(fmt,##args);

#define debug_emerg(dev, fmt, args...)		\
	if(dev>=KERN_EMERG) debug(fmt,##args);
#define debug_alert(dev, fmt, args...)		\
	if(dev>=KERN_ALERT) debug(fmt,##args);
#define debug_crit(dev, fmt, args...)		\
	if(dev>=KERN_CRIT) debug(fmt,##args);
#define debug_err(dev, fmt, args...)		\
	if(dev>=KERN_ERR) Uart_Printf_Time(fmt,##args);
#define debug_warn(dev, fmt, args...)		\
	if(dev>=KERN_WARNING) Uart_Printf_Time(fmt,##args);
#define debug_notice(dev, fmt, args...)		\
	if(dev>=KERN_NOTICE) Uart_Printf_Time(fmt,##args);
#define debug_info(dev, fmt, args...)		\
	if(dev>=KERN_INFO) Uart_Printf_Time(fmt,##args);
#define debug_debug(dev, fmt, args...)		\
   if(dev>=KERN_DEBUG) Uart_Printf_Time(fmt,##args);
#else
#define debug(fmt,args...)   
#define debugX(level,fmt,args...)  
#define debug_emerg(dev, fmt, args...)
#define debug_alert(dev, fmt, args...)
#define debug_crit(dev, fmt, args...)
#define debug_err(dev, fmt, args...)	
#define debug_warn(dev, fmt, args...)
#define debug_notice(dev, fmt, args...)	
#define debug_info(dev, fmt, args...)	
#define debug_debug(dev, fmt, args...)	
#endif
/*end  :yangfei added*/

#endif

/*********************************************************************************************************
      										END FILE
*********************************************************************************************************/
