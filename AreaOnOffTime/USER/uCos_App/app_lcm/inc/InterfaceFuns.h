//为液晶提供接口

#ifndef _INTERFACEFUNS_H
#define _INTERFACEFUNS_H

extern uint8 ReadTermAddr(uint8* pAddr);
extern uint8 Read_CommPara();//将通讯参数读取到全局变量
extern uint8 Read_CommPara(uint8 *pData);
extern uint8 Write_CommPara();//重载函数，为了存储全局变量
extern uint8 Write_CommPara(uint8 *pData);
extern uint8 UpdateUPCommParaInG_V(uint8 *pData);//更新全局变量中的上行通信参数

//自定义参数结构的读取和设置
extern uint8 Read_SelfDefinePara();
extern uint8 Read_SelfDefinePara(uint8* pData);
extern uint8 Set_SelfDefinePara(uint8* pData);
extern uint8 Set_SelfDefinePara();//重载函数，为了存储全局变量

extern uint8 SelfMeterIni(void);    
//读取和设置MAC地址
extern uint8 Set_Mac(uint8* pData);//设置以太网MAC地址
extern uint8 Read_Mac(uint8* pData);//读取以太网MAC地址
extern uint8 SetHostAddr(uint8 *pAddr);
extern uint8 SetTermAddr(uint8 *pAddr);
extern uint8 SetTimeNode(uint8 *pTimeNode, uint8 NodeIndex);
extern uint8 SetIP(uint8 *pAddr);  
extern uint8 SetHostPort(uint16 Port); 
extern uint8 SaveDeviceType(uint8 DeviceType);

 #endif
