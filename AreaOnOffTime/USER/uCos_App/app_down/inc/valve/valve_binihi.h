/*
  ********************************************************************************************************
  * @file    valve_binihi.h
  * @author  zjjin
  * @version V0.0.0
  * @date    04-20-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		百暖汇协议阀控器功能实现相关代码头文件。
  *
  ********************************************************************************************************
  */  

#ifndef _VALVE_BINIHI_H_
#define _VALVE_BINIHI_H_

#include "publicfunc.h"
#include "storeManage.h"
#include "bsp.h"
//#include "meter.h"



extern uint8 ValveContron_Binihi(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback);
extern void Create_Binihi_Vave_SetInfo_Frame(MeterFileType *p_mf,uint8 *p_DataIn, uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_Binihi_SetValve_Frame(MeterFileType *p_mf, uint8 *p_DataIn,uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_Binihi_SetTemp_Frame(MeterFileType *p_mf, uint8 *p_DataIn,uint8 *pSendFrame, uint8 *plenFrame);
extern void Create_Binihi_Vave_ReadInfo_Frame(MeterFileType *p_mf, uint8 *pSendFrame, uint8 *plenFrame);
extern uint8 Binihi_ReadData_Communicate(uint8 *DataFrame, uint8 *DataLen);
extern uint8 Binihi_VaveCommunicate(uint8 *DataFrame, uint8 *DataLen);
extern uint8 Receive_ReadBinihiParamFrame(uint8 dev, uint8 *buf, uint16 Out_Time, uint8 *datalen);



#endif  //#ifndef _VALVE_BINIHI_H_

