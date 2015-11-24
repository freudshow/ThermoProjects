/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: uart0pro.h
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年8月10日 
**  描        述: 下行串口抄读数据处理
**	修 改  记 录:   	
*****************************************************************************************************/
#ifndef				_UART0PRO_H
#define				_UART0PRO_H				

#include "meter.h"

extern OS_EVENT *DownUartRequestQ;//申请使用Mbus的消息队列
extern OS_EVENT *DownUartAnswerMbox_UAF;
extern OS_EVENT *DownUartAnswerMbox_CLK;
extern OS_EVENT *DownUartAnswerMbox_MBUS;
extern OS_EVENT *DownUartAnswerMbox_LCM;

extern uint8* 	Uart0SendThenReceive_PostPend(uint8* buf,uint8* n);
extern OS_EVENT* GetDownUartAnswMbox(uint8 TaskPrio);

#endif
