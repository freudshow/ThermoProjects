/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: SysLog.H
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年9月10日 
**  描        述: 日志记录系统运行信息
**	修 改  记 录:   	
*****************************************************************************************************/
#ifndef _SYSLOG_H
#define _SYSLOG_H

extern uint32 gLOG_TimeCounter;
extern uint8 gLOG_LogLevelFlag;

extern void LOG_QueueCreate(void);
extern uint8 LOG_WriteSysLog(const uint8 *pInLog, uint16 LogLen);
extern uint8 LOG_WriteSysLog_Format(uint8 Flag, const char *format, ...);
extern void TaskLog(void *pdata);
extern void LOG_assert_failed(uint8* file, uint32 line);
extern void LOG_HandleLogBuf(void);
extern void LOG_HandleEXTIBuf(const uint8 *pInLog,uint8 index);
typedef struct{	
	    uint16 WriteFlag;					//该区域是否被写过数据
	    uint16 LOGStoreNums;				//共存储多少块LOG信息
	}ERRLogFilePara;	

typedef struct{	
		uint8 LOGsting[96];					//字符串信息
		uint8 LOGLevel;//日志等级
		uint8 EquipmentType;				//设备类型预留
	}ERRLogFileType;							
typedef struct{	
	    uint16 WriteFlag;					//该区域是否被写过数据
	    uint16 LOGStoreNums;				//共存储多少块LOG信息
	}ERRLogMBUSFilePara;	

typedef struct{	
		uint8 LOGsting[96];					//字符串信息
		uint8 LOGLevel;//日志等级
		uint8 EquipmentType;				//设备类型预留
	}ERRLogMBUSFileType;	

#endif
