
#ifndef				__BPS_I2C_H
#define				__BPS_I2C_H

//extern uint8 I2C_WriteByte(uint8 SendByte, uint16 WriteAddress, uint8 DeviceAddress);
#ifdef   _MBUS_DETACH_
extern OS_EVENT *I2cMsgSem;
extern OS_EVENT *I2cMsgRetSem;
#endif
extern uint8 I2C_ReadByte(uint8* pBuffer,   uint16 length,   uint16 ReadAddress,  uint8 DeviceAddress);

#endif
/*********************************************************************************************************
**                            End Of File
********************************************************************************************************/
