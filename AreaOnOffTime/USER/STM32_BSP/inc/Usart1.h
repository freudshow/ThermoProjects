
#ifndef				_USART1_H
#define				_USART1_H

extern uint8 USART1RecQueue[];
extern OS_EVENT *Usart1RecQueSem;
//-------------------------------------------------------------------------------------------------------
extern uint8 USART1_QueueCreate(void);

extern void USART1Send(uint8* buf,uint32 n);//供上层调用

extern void USART1Putch(uint8 Data);//供上层调用

extern uint8 USART1Getch(uint8* data,uint16 OutTime);//供上层调用

extern void TaskUsart1Rec(void *pdata);

extern void USART1_IRQHandler(void);
/*begin:yangfei added 2013-01-23*/
extern void Uart_Printf(char *fmt,...);
extern void Uart_Printf_Time(char *fmt,...);
extern void Uart_ReceiveChar (void) ;
extern u8 Uart_GetChar(void);
/*end:yangfei added 2013-01-23*/
#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
