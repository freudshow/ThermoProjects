
#ifndef        _GPIO_INT_H
#define		   _GPIO_INT_H


extern OS_EVENT *GpioIntMbox;
extern uint8 GPIOINTInit(void);
#ifdef   _MBUS_DETACH_
extern OS_EVENT *GpioPwrMbox;
extern uint8 GPIOPwrInit(void);
#endif
#endif
