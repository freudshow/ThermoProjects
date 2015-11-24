/*
  ********************************************************************************************************
  * @file    valve_ADE.h
  * @author  zjjin
  * @version V0.0.0
  * @date    08-10-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		航天德鲁协议阀控器功能实现相关代码，相对于原德鲁阀控协议有些调整
  		为了与原兼容，另加本文件。
  *
  ********************************************************************************************************
  */  


#ifndef _VALVE_ADE_H_
#define _VALVE_ADE_H_

#include "publicfunc.h"
//#include "storeManage.h"
//#include "bsp.h"
#include "meter.h"


#define ADE_SETVALVE_STATUS			0	//下发控制阀开度命令
#define ADE_SETHEAT_VALUE			1	//下发热量数据到阀控器。
#define ADE_READROOM_TEMP			2	//读室内温度
#define ADE_SETTEMP_RANGE			3	//设置室内上下限温度。
#define ADE_SETVALVE_CONTROLTYPE	4	//设置阀门控制方式，使能、禁能、自动、定时等。
#define ADE_SETROOM_TEMP			5	//设置室内温度
#define ADE_READVALVE_STATUS		6	//读阀门状态
#define ADE_SETVALVE_OFFSETTEMP		7	//设置补偿温度。
#define ADE_SETVALVE_HYSTEMP		8	//设置阀控启动温差(滞回温度)
#define ADE_SETTEMP_GATHERPERIOD	9	//设置温控面板温度采集周期。
#define ADE_SETTEMP_UPLOADPERIOD	10	//设置温控面板温度上报周期。
#define ADE_READ_VALVEDATA			11	//通断时间面积法用，读取阀控器数据，包括开阀时间、进回水温度、室内温度。
#define ADE_WR_HEATDATA				12	//通断时间面积法用，下发写入分摊热量值。





extern uint16 gADEVALVE_Table[][2];











extern uint8 ValveContron_ADE(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback);
extern uint8 ADEValveCommunicate(MeterFileType *p_mf,uint8 ADE_Functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback);

#endif  //#ifndef _VALVE_ADE_H_

