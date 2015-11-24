/*******************************************Copyright (c)*******************************************
         							山东华宇空间技术公司(西安分部)                                                 
**  文   件   名: tasks_up.h
**  创   建   人: 勾江涛
**  创 建  日 期: 2012.08.07 
**  描        述: 与主站的通信任务
**  修 改  记 录:   	
*****************************************************************************************************/
#ifndef _TASKS_UP_H
#define _TASKS_UP_H

#define  BLK_NUM                         	10    		//内存块数，也是消息队列的长度，可接收帧的最大个数
/*begin:yangfei added 2013-03-15  for 增加buff 缓存大小*/
#define  RECE_BUF_LEN                    300 	//256    		//每块内存长度，也是可接收帧的最大长度
/*end:yangfei added 2013-03-15  for 增加buff 缓存大小*/
//定义内存分配
extern OS_MEM* CommTxBuffer;
extern uint32 CommTxPart[BLK_NUM][RECE_BUF_LEN];
extern OS_EVENT *CommTxPartSem;								//与内存分配绑定的信号量

extern void* MsgGrp[BLK_NUM];               				//定义消息指针数组
extern OS_EVENT *Str_Q;

extern void* UpSend_Q_Grp[BLK_NUM];
extern OS_EVENT *UpSend_Q;

#pragma	pack(1)
typedef struct {
	uint8   GprsGvc;		// TRUE OR  FALSE，为TRUE时，表示对GPRS的管理，下面几个参数无效
    uint8   Device;			// 发送数据的设备，GPRS, Zigbee, RS485
    uint16  ByteNum;		// 发送字节数
    uint8   buff[1];		//发送数据
} UP_COMM_MSG;

#pragma pack()

void Createtasks_up(void);
void CreateMboxs_up(void);


#endif
/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/
