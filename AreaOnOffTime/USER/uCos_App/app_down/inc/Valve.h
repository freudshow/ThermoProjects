/*
  ********************************************************************************************************
  * @file    Valve.h
  * @author  zjjin
  * @version V0.0.0
  * @date    04-15-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		为在集中器中实现多厂家不同协议阀门的兼容，在表基础信息中加入了
  *	阀控协议版本，各厂家阀控器相关程序在Valve.c中，本文件是Valve.c文件的头文件。
  *
  ********************************************************************************************************
  */  
#ifndef _VALVE_H_
#define _VALVE_H_

#include "publicfunc.h"
#include "StoreManage.h"
#include "Bsp.h"


//begin:定义阀控器协议版本。
#define VALVE_DELU			0       //德鲁阀控协议。
#define VALVE_ELSONIC		1		//亿林阀控协议。
#define	VALVE_JOYO			2		//北京京源阀控协议。
#define VALVE_BINIHI		3		//百暖汇阀控协议。
#define VALVE_LCARBO		4		//琅卡博阀控协议。
#define VALVE_PRECISE		5		//济南普赛通讯协议。
#define VALVE_ADE			6		//航天德鲁自研阀控器协议。



//end:定义阀控器协议版本。

//begin:定义阀控操作功能号。
#define ReadVALVE_All				0		//读阀控的所有信息
#define READROOM_TEMP				1		//读室内温度
#define READVALVE_STATUS			2		//读阀门开度状态
#define READTEMP_RANGE				3		//读温度上下限。
#define SETHEAT_DISPLAY				4		//设置温控面板显示热量值。
#define SETHEAT_VALUE				5		//设置温控面板显示的热量值。
#define SETROOM_TEMP				6		//设置室内温度
#define SETTEMP_RANGE				7		//设置室内上下限温度。
#define SETVALVE_STATUS				8		//设置阀开度状态。
#define SETVALVE_LOCK				9		//锁定阀,锁定阀时要保证阀门全开。
#define SETTEMP_ANDRANGE			10		//同时设定房间温度和温度上下限范围。
#define SETVALVE_CONTROLTYPE		11		//设置阀门控制方式，使能、禁能、自动、定时等。
#define READVALVE_OPENTIME			12		//读取阀的阀开时间。
#define READVALVE_WATERTEMP			13		//读阀的进回水温度。

#define SETVALVE_OFFSETTEMP			14		//设置补偿温度。
#define SETVALVE_HYSTEMP			15		//设置阀控启动温差(滞回温度)
#define SETTEMP_GATHERPERIOD		16		//设置温控面板温度采集周期。
#define SETTEMP_UPLOADPERIOD		17		//设置温控面板温度上报周期。
#define READ_VALVEDATA				18		//通断时间面积法用，读取阀控器数据，包括开阀时间、进回水温度、室内温度。
#define WRITE_DIVHEATDATA			19		//通断时间面积法用，向阀控器下发分摊热量值。



//end:定义阀控操作功能号。

extern uint8 gVALVE_Table[][2];



extern uint8 ValveContron(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback);
//extern uint8 cmpAddr(uint8 *p_addr1,uint8 *p_addr2);




#endif  //  #ifndef _VALVE_H_

