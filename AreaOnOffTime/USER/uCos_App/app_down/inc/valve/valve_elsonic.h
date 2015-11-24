/*
  ********************************************************************************************************
  * @file    valve_elsonic.h
  * @author  zjjin
  * @version V0.0.0
  * @date    04-15-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		亿林协议阀控器功能实现相关代码头文件。
  *
  ********************************************************************************************************
  */  

#ifndef _VALVE_ELSONIC_H_
#define _VALVE_ELSONIC_H_

#include "publicfunc.h"
#include "storeManage.h"
#include "bsp.h"
//#include "meter.h"



extern uint8 ValveContron_Elsonic(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback);
extern void Create_ElsonicVave_ReadInfo_Frame(MeterFileType *p_mf, uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_ElsonicVave_SetInfo_Frame(MeterFileType *p_mf, uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_ElsonicVave_SetHeat_Frame(MeterFileType *p_mf,uint8 *p_DataIn, uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_ElsonicVave_SetRoomTemp_Frame(MeterFileType *p_mf,uint8 *p_DataIn, uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_ElsonicVave_SetRoomTempRange_Frame(MeterFileType *p_mf,uint8 *p_DataIn, uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_ElsonicVave_SetValve_Frame(MeterFileType *p_mf,uint8 *p_DataIn, uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_ElsonicVave_ReadOpenTime_Frame(MeterFileType *p_mf, uint8 *p_DataIn,uint8 *pSendFrame, uint8 *plenFrame);
extern  void Create_ElsonicVave_ReadWaterTemp_Frame(MeterFileType *p_mf, uint8 *p_DataIn,uint8 *pSendFrame, uint8 *plenFrame);
extern uint8 Elsonic_ReadData_Communicate(uint8 *DataFrame, uint8 *DataLen);
extern uint8 Receive_Read_ElsonicParamFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen);




#endif  //#ifndef _VALVE_ELSONIC_H_
