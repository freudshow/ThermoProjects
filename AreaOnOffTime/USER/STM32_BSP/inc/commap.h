
#ifndef				_COM_MAP_H
#define				_COM_MAP_H
/*begin:yangfei modified 2013-10-14 for 增加zigbee现有的每次100个字节，增加至每次120个字节*/
//#define		ZIGBEE_SEND_BYTES	100
//#define		ZIGBEE_SEND_BYTES	      200
/*end:yangfei modified 2013-10-14 for 增加zigbee现有的每次100个字节，增加至每次120个字节*/
#define		UP_COMMU_DEV_ARRAY	6

//=============上行通信设备ID ，这些常量的值直接作为全局数组的下标使用，因此不能随便修改
#define  UP_COMMU_DEV_ZIGBEE	       0
#define  UP_COMMU_DEV_485		1
#define  UP_COMMU_DEV_232		2
#define  UP_COMMU_DEV_GPRS		3
#define  UP_COMMU_DEV_AT		       4
#define  UP_COMMU_DEV_ATIPD		5

//============================

//===============下行通信设备ID
#define		DOWN_COMM_DEV_MBUS		0
#define		DOWN_COMM_DEV_485		1
#define	    DOWN_COMM_DEV_RESERVE	2
#define		DOWN_COMM_DEV_485_PLC		3
//============================================================================

extern uint8 UpcomInit(void);
extern uint8 UpGetStart(uint8 dev, uint16 OutTime);
extern uint8 UpGetch(uint8 dev,uint8* data,uint16 OutTime);
extern uint8 UpQueueFlush(uint8 dev);
extern uint8 UpRecQueueWrite(uint8 dev,uint8* buf,uint32 n);
extern void UpDevSend(uint8 dev,uint8 *Data, uint32 n);

extern uint8 DuGetch(uint8 dev,uint8* data,uint16 OutTime);
extern void ChangeDuBps(uint8 dev,uint32 bps);
extern void DuSend(uint8 dev,uint8* buf,uint32 n);
extern uint8 DuQueueFlush(uint8 dev);   //清空缓冲区


extern uint16 TestUpRec(uint8 dev,uint8* buff,uint16 max);
extern uint16 TestDownRec(uint8 dev,uint8* buff,uint16 max);


#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
