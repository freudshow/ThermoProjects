
#ifndef _TERM_CONTROL_H
#define _TERM_CONTROL_H

#include "structprotocol.h"

#define	FIRST_PACK				0				//下发表档案信息时，分包发送的 第一包

extern uint8 PARA_ProcessMsg_02(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_0C(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_13(_ProtocolType &_ProData);
extern uint8 PARA_GetMeterSn(uint8 *pMeterAddr, uint16 *pMeterSn);
extern uint8 PARA_GetMeterSn_ByMeterID(uint16 *pMeterID, uint16 *pMeterSn);
extern uint8 PARA_ReadMeterInfo(uint16 MeterSn, MeterFileType *pMeterFile);
extern uint8 PARA_SaveMeterFile(MeterFileType	*pMeterFile, uint8 StoreNum, uint16 MeterSn);
extern uint8 PARA_ProcessMsg_20(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_24(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_26(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_34(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_3A(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_82(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_83(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_84(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_85(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_50(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_51(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_52(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_53(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_54(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_55(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_56(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_66(_ProtocolType &_ProData);
extern uint8 PARA_ProcessMsg_90(_ProtocolType &_ProData);
extern uint8 DEAL_ProcessMsg_92(_ProtocolType &_ProData, uint8 *ControlSave);
extern uint8 PARA_ProcessMsg_94(_ProtocolType &_ProData);


#endif
