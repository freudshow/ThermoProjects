
#ifndef				_DATETIME_H
#define				_DATETIME_H

#define ARRAY_SECOND	0
#define ARRAY_MINUTE	1
#define ARRAY_HOUR		2
#define ARRAY_DAY		3
#define ARRAY_MONTH		4
#define ARRAY_YEAR		5


extern uint8 TIME_AddSec(uint8* pDate,uint8 sec); //秒增量
extern uint8 TIME_AddMin(uint8* pDate,uint8 min); //分钟增量

extern uint8 TIME_AddHour(uint8* pDate,uint8 hour); //小时增量

extern uint8 TIME_AddDate(uint8* pDate,uint8 date); //日增量

extern uint8 TIME_AddMon(uint8* pDate,uint8 mon); //月增量
extern uint8 TIME_AddTime(uint8* pDate,uint8 time,uint8 flag);

extern int8 TIME_CmpTime(uint8* pTime2,uint8* pTime1);//精确到分钟 

extern uint8 TIME_SubMin(uint8* pDate,uint8 min); //分钟减法

extern uint8 TIME_SubHour(uint8* pDate,uint8 hour); //小时减法

extern void TIME_BspSystemTimeInit(void);		//系统硬件初始化阶段的系统时间更新

extern void TIME_UpdateSystemTime(void);		//更新系统时间

extern uint8 ReadDateTime(uint8* retbuf);		//读取系统时间

#endif
