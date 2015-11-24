
#ifndef				_UART5_H
#define				_UART5_H

extern uint8 UART5RecQueue[];
extern OS_EVENT *UART5RecQueSem;

//-------------------------------------------------------------------------------------------------------
extern uint8 UART5_QueueCreate(void);

extern void UART5Send(uint8* buf,uint32 n);//供上层调用

extern void UART5Putch(uint8 Data);//供上层调用

extern uint8 UART5Getch(uint8* data,uint16 OutTime);//供上层调用

extern void TaskUART5Rec(void *pdata);

extern void UART5_IRQHandler(void);

#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
