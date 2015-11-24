
#ifndef				_UART4_H
#define				_UART4_H

extern uint8 UART4RecQueue[];
extern OS_EVENT *UART4RecQueSem;

extern uint16 gIRQCOunter;
extern uint16 gTaskCounter;

//-------------------------------------------------------------------------------------------------------
extern uint8 UART4_QueueCreate(void);

extern void UART4Send(uint8* buf,uint32 n);//供上层调用

extern void UART4Putch(uint8 Data);//供上层调用

extern uint8 UART4Getch(uint8* data,uint16 OutTime);//供上层调用

extern void TaskUART4Rec(void *pdata);

extern void UART4_IRQHandler(void);

#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
