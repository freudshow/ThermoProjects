/*
  ********************************************************************************************************
  * @file    valve_precise.h
  * @author  zjjin
  * @version V0.0.0
  * @date    06-16-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		济南普赛通讯协议阀控器功能实现相关代码头文件。
  *
  ********************************************************************************************************
  */  

#ifndef _VALVE_PRECISE_H_
#define _VALVE_PRECISE_H_

#include "publicfunc.h"
//#include "storeManage.h"
//#include "bsp.h"
#include "meter.h"



extern uint8 ValveContron_Precise(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback);



#endif  //#ifndef _VALVE_PRECISE_H_

