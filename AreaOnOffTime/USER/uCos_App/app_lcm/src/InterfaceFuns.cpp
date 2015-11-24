/*******************************************Copyright (c)*******************************************
									山东华宇空间技术公司(西安分部)                                                          
**  文   件   名: interfacefuns.cpp
**  创   建   人: 勾江涛
**	版   本   号: 0.1
**  创 建  日 期: 2012年9月10日 
**  描        述: 日志记录系统运行信息
**	修 改  记 录:   	
*****************************************************************************************************/
/********************************************** include *********************************************/
#include <includes.h>

#include "InterfaceFuns.h"
#include "app_flashmem.h" 
/****************************************************************************************************
**	函 数  名 称: Reversal
**	函 数  功 能: 撤销函数
**	输 入  参 数: uint8* pStr
**				  uint16 len
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
void Reversal(uint8* pStr,uint16 len)
{
    uint8* pTemp = new uint8[len];
    uint32 i;
    for(i=0;i<len;i++) pTemp[i] = pStr[len-i-1];
    
    memcpy(pStr,pTemp,len);
    delete(pTemp);
    
 
}
/****************************************************************************************************
**	函 数  名 称: ReadTermAddr
**	函 数  功 能: 读取项目地址
**	输 入  参 数: uint8* pAddr
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 ReadTermAddr(uint8* pAddr)//抄取终端地址
{
	//ReadF248(pAddr);
	return 0;
}

/****************************************************************************************************
**	函 数  名 称: Read_CommPara
**	函 数  功 能: 将通讯参数读取到全局变量中
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Read_CommPara()//将通讯参数读取到全局变量中
{
    CommPara uPCommPara;
    Read_CommPara((uint8*)&uPCommPara);        
    UpdateUPCommParaInG_V((uint8*)&uPCommPara);
    return 0;  	
}

/****************************************************************************************************
**	函 数  名 称: Read_CommPara
**	函 数  功 能: 将通讯参数读取到全局变量中
**	输 入  参 数: uint8* pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Read_CommPara(uint8 *pData)
{
    uint8 result;
    //CommPara* pUPCommPara=(CommPara*)pData;
	
 	return result;
}
/****************************************************************************************************
**	函 数  名 称: Write_CommPara
**	函 数  功 能: 通讯参数的写入
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Write_CommPara()//重载函数，为了存储全局变量
{
/*
	uint8 err;
   CommPara uPCommPara;
   Read_CommPara((uint8*)&uPCommPara); //&uPCommPara.PPIP1[0]
   uint8 Ip[4]; 
   err = ConverASCIPToHex(&gStore_CommPara.PPIP[0],&Ip[0]);//将ASC码的IP转换为4字节的16进制
   memcpy(uPCommPara.PPIP1,Ip,4);
   uPCommPara.PPPort = AsciiToBcd_16bit(&gStore_CommPara.PPPort[0]);
   err = ConverASCIPToHex(&gStore_CommPara.PPIP[0],&Ip[0]);//将ASC码的IP转换为4字节的16进制
   memcpy(uPCommPara.PPIP1,Ip,4);
   uPCommPara.PPPort = AsciiToBcd_16bit(&gStore_CommPara.PPPort[0]);
   memcpy(uPCommPara.APNName,gStore_CommPara.APNName,gStore_CommPara.LenAPNName);
   memset(&uPCommPara.APNName[gStore_CommPara.LenAPNName],0x00,16-gStore_CommPara.LenAPNName);
   uPCommPara.SocketType = gStore_CommPara.SocketType;
   memcpy(uPCommPara.Jzqpassword,gStore_CommPara.TermPw,16);
   uPCommPara.HeartCycle =gStore_CommPara.HeartCycle ;//得到心跳周期  
   
   //以太网参数
   //err = ConverASCIPToHex(&gStore_CommPara.TermIP[0],&Ip[0]);//将ASC码的IP转换为4字节的16进制
   //memcpy(uPCommPara.TermIP,Ip,4);
   memcpy(uPCommPara.TermIP, &gStore_CommPara.TermIP[0], 4);
   
   //err = ConverASCIPToHex(&gStore_CommPara.TermSubMask[0],&Ip[0]);//将ASC码的IP转换为4字节的16进制
   //memcpy(uPCommPara.TermSubMask,Ip,4);
   memcpy(uPCommPara.TermSubMask, &gStore_CommPara.TermSubMask[0], 4);
   
   //err = ConverASCIPToHex(&gStore_CommPara.TermGateWay[0],&Ip[0]);//将ASC码的IP转换为4字节的16进制
   //memcpy(uPCommPara.TermGateWay,Ip,4); 
   memcpy(uPCommPara.TermGateWay, &gStore_CommPara.TermGateWay[0], 4);
   
   //uPCommPara.TermPort = AsciiToBcd_16bit(&gStore_CommPara.TermPort[0]);
   uPCommPara.TermPort = gStore_CommPara.TermPort;
     
   Write_CommPara((uint8*)&uPCommPara);
   */
   return 0;
       
}
/****************************************************************************************************
**	函 数  名 称: Write_CommPara
**	函 数  功 能: 通讯参数的写入
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Write_CommPara(uint8 *pData)
{
    uint8 result;
    CommPara* pUPCommPara=(CommPara*)pData;
    /*
    //设置心跳
    afn04f1 f1;
    OSMutexPend(FlashMutex,0,&err);
	result=FlashReadBytes(AFN04F1,sizeof(afn04f1),(uint8*)&f1,0);
	OSMutexPost(FlashMutex);       
	f1.HeartbeatCyc=pUPCommPara->HeartCycle;
	f1.flagCon=pUPCommPara->flagCon;        
    OSMutexPend(FlashMutex,0,&err);
     result = AppFlashWriteBytes(AFN04F1,sizeof(afn04f1),(uint8*)&f1);
	 OSMutexPost(FlashMutex); 
	 
	 
     afn04f8 f8;//UDP TCP
    OSMutexPend(FlashMutex,0,&err);
	result=FlashReadBytes(AFN04F8,sizeof(afn04f8),(uint8*)&f8,0);
	OSMutexPost(FlashMutex);   
    f8.ConnectMode=pUPCommPara->SocketType;
    OSMutexPend(FlashMutex,0,&err);
     result = AppFlashWriteBytes(AFN04F8,sizeof(afn04f8),(uint8*)&f8);
	 OSMutexPost(FlashMutex); 	 

   afn04f3 f3;//主站（前置机）
    OSMutexPend(FlashMutex,0,&err);
	result=FlashReadBytes(AFN04F3,sizeof(afn04f3),(uint8*)&f3,0);
	OSMutexPost(FlashMutex);       
    memcpy(f3.Ip1,pUPCommPara->PPIP1,4);
    memcpy(f3.Ip2,pUPCommPara->PPIP2,4); 
    f3.Port1=pUPCommPara->PPPort;
    f3.Port2=pUPCommPara->PPport;
    memcpy(f3.Apn,pUPCommPara->APNName,16); 
    OSMutexPend(FlashMutex,0,&err);
     result = AppFlashWriteBytes(AFN04F3,sizeof(afn04f3),(uint8*)&f3);
	 OSMutexPost(FlashMutex); 
	 
    TermPW pw; 
     OSMutexPend(FlashMutex,0,&err);
	result=FlashReadBytes(TERMPW,sizeof(TermPW),(uint8*)&pw,0);
	OSMutexPost(FlashMutex);          
    memcpy(pw.InterfacePw,pUPCommPara->InterfacePw,6);  
    memcpy(pw.Jzqpassword,pUPCommPara->Jzqpassword,16);  
     
     OSMutexPend(FlashMutex,0,&err);
	result=AppFlashWriteBytes(TERMPW,sizeof(TermPW),(uint8*)&pw);
	OSMutexPost(FlashMutex); 
	
    //以太网参数
    afn04f7 f7;
    OSMutexPend(FlashMutex,0,&err);
	result=FlashReadBytes(AFN04F7,sizeof(afn04f7),(uint8*)&f7,0);
	OSMutexPost(FlashMutex); 
    memcpy(f7.Ip,pUPCommPara->TermIP,4);
    memcpy(f7.SubnetMark,pUPCommPara->TermSubMask,4); 
    memcpy(f7.Gateway,pUPCommPara->TermGateWay,4);
    f7.Port=pUPCommPara->TermPort;
    OSMutexPend(FlashMutex,0,&err);
	result=AppFlashWriteBytes(AFN04F7,sizeof(afn04f7),(uint8*)&f7);
	OSMutexPost(FlashMutex);
	*/	
	UpdateUPCommParaInG_V((uint8*)pUPCommPara);
	         
    return result;   
}
/****************************************************************************************************
**	函 数  名 称: UpdateUPCommParaInG_V
**	函 数  功 能: 更新全局变量中的上行通信参数
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 UpdateUPCommParaInG_V(uint8 *pData)//更新全局变量中的上行通信参数
{
  	uint8  TempBuf[16];
	uint8  len = 0;
    CommPara* pUPCommPara=(CommPara*)pData;
   	if(pUPCommPara->SocketType == 1)
   	
	gStore_CommPara.SocketType = pUPCommPara->SocketType;// 得到GPRS的通讯方式：TCP/UDP
	else
	gStore_CommPara.SocketType = 0;       //0:TCP    1:UDP


	memcpy(gStore_CommPara.TermPw,pUPCommPara->Jzqpassword,16);
	if(pUPCommPara->InterfacePw[0] == 0xff)
	memset(gStore_CommPara.InterfacePw,'6',6);
	else
	memcpy(gStore_CommPara.InterfacePw,pUPCommPara->InterfacePw,6);//界面密码
	gStore_CommPara.HeartCycle = pUPCommPara->HeartCycle;//得到心跳周期
	
	len = ConverHexToASCIP(pUPCommPara->PPIP1,TempBuf);//主IP   转换后为正向
	memcpy(gStore_CommPara.PPIP,TempBuf,len);//
	gStore_CommPara.LenPPIP = len;
	
	len = Hex_BcdToAscii(pUPCommPara->PPPort,gStore_CommPara.PPPort);
	Reversal(gStore_CommPara.PPPort,len);		
	gStore_CommPara.LenPPPort = len;	
	
	for(len = 0;len<16;len++)   //计算APN名称的长度Len
	{
	   if(pUPCommPara->APNName[len] == 0x00 || pUPCommPara->APNName[len] == 0x0D )
	   break;
	}
	memcpy(gStore_CommPara.APNName,pUPCommPara->APNName,len);

	gStore_CommPara.LenAPNName = len;
	
	//更新全局变量--以太网IP

	memcpy(gStore_CommPara.TermIP,&pUPCommPara->TermIP[0],4);

	//更新全局变量--子网掩码

	memcpy(gStore_CommPara.TermSubMask,&pUPCommPara->TermSubMask[0],4);
	

	memcpy(gStore_CommPara.TermGateWay,&pUPCommPara->TermGateWay[0],4);
	

	gStore_CommPara.TermPort = pUPCommPara->TermPort;
	return 0;
}

/****************************************************************************************************
**	函 数  名 称: Read_SelfDefinePara
**	函 数  功 能: 自定义参数的读取和设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Read_SelfDefinePara()//从FLASH更新全局变量
{
    uint8 result;
  /*  SelfDefinePara para;
    
    OSMutexPend(FlashMutex,0,&err);
	result=FlashReadBytes(SELFDEFINEPARA,sizeof(SelfDefinePara),(uint8*)&para,0);
	OSMutexPost(FlashMutex);
	
	//memcpy(gStore_CommPara.TermMACAddr, para.TermMACAddr, 6);//MAC地址
	gStore_CommPara.LCDContrastValue = para.LCDContrastValue;//LCD参数S
	//通道类型选择
	if(para.ChannelType > 4)
	{
		para.ChannelType = 0;
	}
	gStore_CommPara.ChannelType = para.ChannelType;
	*/return result;
}
/****************************************************************************************************
**	函 数  名 称: Read_SelfDefinePara
**	函 数  功 能: 自定义参数的读取和设置
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Read_SelfDefinePara(uint8* pData)
{
	
	uint8 result;
   /* SelfDefinePara* pSelfPara=(SelfDefinePara*)pData;
    //SelfDefinePara para;
    OSMutexPend(FlashMutex,0,&err);
	result=FlashReadBytes(SELFDEFINEPARA,sizeof(SelfDefinePara),(uint8*)pSelfPara,0);
	OSMutexPost(FlashMutex);
	
	//memcpy(gStore_CommPara.TermMACAddr, pSelfPara->TermMACAddr, 6);
	//gStore_CommPara.LCDContrastValue = pSelfPara->LCDContrastValue;
	*/
	return result;
}

/****************************************************************************************************
**	函 数  名 称: Read_SelfDefinePara
**	函 数  功 能: 自定义参数的读取和设置
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Set_SelfDefinePara(uint8* pData)
{
	
	uint8 result;
  /*  OSMutexPend(FlashMutex,0,&err);
	result=AppFlashWriteBytes(SELFDEFINEPARA,sizeof(SelfDefinePara),pData);
	OSMutexPost(FlashMutex);
	*/
	return result;
}

/****************************************************************************************************
**	函 数  名 称: Read_SelfDefinePara
**	函 数  功 能: 自定义参数的读取和设置
**	输 入  参 数: none
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Set_SelfDefinePara()//存储全局变量
{
    uint8 result;
	/*
    SelfDefinePara para;
    result = Read_SelfDefinePara((uint8*)&para);
    if(result == FALSE)
    {
    	return result;
    }
	para.LCDContrastValue = gStore_CommPara.LCDContrastValue;
	para.ChannelType = gStore_CommPara.ChannelType;
	result = Set_SelfDefinePara((uint8*)&para);
	*/
	return result;
}	

/****************************************************************************************************
**	函 数  名 称: Set_Mac
**	函 数  功 能: 设置以太网MAC地址
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Set_Mac(uint8* pData)//设置以太网MAC地址
{
	
	uint8 result;
/*	IOandCommCfg para;
    OSMutexPend(FlashMutex,0,&err);
	result = FlashReadBytes(IOANDCOMMCFG,sizeof(IOandCommCfg),(uint8*)&para,0);
	OSMutexPost(FlashMutex);
	if(result == FALSE)
    {
    	return result;
    }
	memcpy(&para.MAC[0],pData,6);
    OSMutexPend(FlashMutex,0,&err);
	result = AppFlashWriteBytes(IOANDCOMMCFG,sizeof(IOandCommCfg),(uint8*)&para);
	OSMutexPost(FlashMutex);
	*/
	return result;
}
/****************************************************************************************************
**	函 数  名 称: Read_Mac
**	函 数  功 能: 读取以太网MAC地址
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 Read_Mac(uint8* pData)//读取以太网MAC地址
{
	
    uint8 result;
/*	IOandCommCfg para;
    OSMutexPend(FlashMutex,0,&err);
	result = FlashReadBytes(IOANDCOMMCFG,sizeof(IOandCommCfg),(uint8*)&para,0);
	OSMutexPost(FlashMutex);
	
    if(result == FALSE)
    {
    	return result;
    }
	memcpy(pData, &para.MAC[0], 6);
	*/

	return result;
}

/****************************************************************************************************
**	函 数  名 称: setHostAddr
**	函 数  功 能: 设置主站地址
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 SetHostAddr(uint8 *pAddr)
{
	uint8 Err = NO_ERR;
	TermParaSaveType TermPara;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	TermPara = gPARA_TermPara;
	OS_EXIT_CRITICAL();
	
	TermPara.WriteFlag = 0xAA55;
	memcpy(TermPara.HostAddr, pAddr, 6);
	
	OSMutexPend (FlashMutex,0,&Err);
	#if 0
    Err =AppSdWrRdBytes(TERMINAL_PARA, sizeof(TermParaSaveType), (uint8 *)&TermPara.WriteFlag, SD_WR);
	#else
	Err = SDSaveData("/TERMINAL_PARA", &TermPara, sizeof(TermParaSaveType), 0);
	#endif
    OSMutexPost (FlashMutex);
    
    OS_ENTER_CRITICAL();
	gPARA_TermPara = TermPara;
	OS_EXIT_CRITICAL();

	return Err;
}

/****************************************************************************************************
**	函 数  名 称: SetTermAddr
**	函 数  功 能: 
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 SetTermAddr(uint8 *pAddr)
{
	uint8 Err = NO_ERR;
	TermParaSaveType TermPara;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	TermPara = gPARA_TermPara;
	OS_EXIT_CRITICAL();
	
	TermPara.WriteFlag = 0xAA55;
	memcpy(TermPara.TermAddr, pAddr, 6);
	
	OSMutexPend (FlashMutex,0,&Err);
	#if 0
    Err =AppSdWrRdBytes(TERMINAL_PARA, sizeof(TermParaSaveType), (uint8 *)&TermPara.WriteFlag, SD_WR);
	#else
	Err = SDSaveData("/TERMINAL_PARA", &TermPara, sizeof(TermParaSaveType), 0);
	#endif
    OSMutexPost (FlashMutex);
    
    OS_ENTER_CRITICAL();
	gPARA_TermPara = TermPara;
	OS_EXIT_CRITICAL();

	return Err;
}
/****************************************************************************************************
**	函 数  名 称: SetTimeNode
**	函 数  功 能: 
**	输 入  参 数: uint8 *pData
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 SetTimeNode(uint8 *pTimeNode, uint8 NodeIndex)
{
	uint8 Err = 0x00;
	//uint8 ReadTime[6] = {0x00};
	uint16 TimeNodeArry[TIME_NODE_NUM_MAX+1] = {0x00};		//第一个数组元素，用来存储到SD卡表示该参数被写入过
	//uint32 LogicAddr	= 0x00;
	//DataStoreParaType  DataPara;
	/*begin:yangfei modified 2012-12-24 find path*/
	char  NodePath[] = "/2012/12/24/timenode";
	uint8 SystemTime[6] = {0};
	/*end   :yangfei modified 2012-12-24*/
	CPU_SR		cpu_sr;
	
	NodeIndex--;
	memset((uint8 *)TimeNodeArry, 0xFF, sizeof(TimeNodeArry));
	TimeNodeArry[0] = 0xAA55;
	
	OS_ENTER_CRITICAL();
	memcpy(&gPARA_TimeNodes[NodeIndex], pTimeNode, 2);
	#if  0
	memcpy((uint8 *)&TimeNodeArry[1], (uint8 *)gPARA_TimeNodes, sizeof(TimeNodeArry));
	#endif
	memcpy((uint8 *)&TimeNodeArry[1], (uint8 *)gPARA_TimeNodes, sizeof(gPARA_TimeNodes));
	memcpy(SystemTime, gSystemTime, 6);
	OS_EXIT_CRITICAL();

	//存储到SD卡中
	OSMutexPend (FlashMutex,0,&Err);
	#if 0
    Err =AppSdWrRdBytes(TIME_NODE_ADDR, (TIME_NODE_NUM_MAX+1)*sizeof(uint16), (uint8 *)TimeNodeArry, SD_WR);
	#else
	Err = SDSaveData("/TIME_NODE_ADDR", TimeNodeArry, sizeof(TimeNodeArry), 0);
	#endif
    OSMutexPost (FlashMutex);
	if(Err!=NO_ERR)
    	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING:Err =%d %s %d SDSaveData Error!",Err,__FUNCTION__,__LINE__ );
    	}
	/*begin:yangfei modified 2012-12-24 find path*/	
	#if 0
    Err =AppSdWrRdBytes(LogicAddr, sizeof(DataStoreParaType), (uint8 *)&DataPara, SD_RD);
    memcpy((uint8 *)DataPara.TimeNode, (uint8 *)&TimeNodeArry[1], TIME_NODE_NUM_MAX*sizeof(uint16));
    Err =AppSdWrRdBytes(LogicAddr, sizeof(DataStoreParaType), (uint8 *)&DataPara, SD_WR);
	#else
	GetFilePath(NodePath, SystemTime, ARRAY_DAY);
	OSMutexPend (FlashMutex, 0, &Err);
	Err = SDSaveData(NodePath, &TimeNodeArry[1], (TIME_NODE_NUM_MAX)*sizeof(uint16),8);
	OSMutexPost (FlashMutex);
	if(Err!=NO_ERR)
    	{
    		LOG_WriteSysLog_Format(LOG_LEVEL_WARN, "WARNING: %s %d SDSaveData Error!",__FUNCTION__,__LINE__ );
    	}
	/*end:yangfei changed 2012-12-28*/
	#endif
 
	if(Err != NO_ERR)		Err = NO_ERR;
	
	return Err;
}
/****************************************************************************************************
**	函 数  名 称: SetIP
**	函 数  功 能: 
**	输 入  参 数: uint8 *pAddr  
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 SetIP(uint8 *pAddr)
{
	uint8 Err = NO_ERR;
	UpCommParaSaveType TermPara;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	TermPara = gPARA_UpPara;
	OS_EXIT_CRITICAL();
	
	TermPara.WriteFlag = 0xAA55; 
	memcpy(TermPara.HostIPAddr, pAddr, 4);
	
	OSMutexPend (FlashMutex,0,&Err);	
	Err = SDSaveData("/UP_COMM_PARA", &TermPara, sizeof(UpCommParaSaveType), 0);
    OSMutexPost (FlashMutex);
    
    OS_ENTER_CRITICAL();
	gPARA_UpPara = TermPara;
	OS_EXIT_CRITICAL();

	return Err;
}
/****************************************************************************************************
**	函 数  名 称: SetPort
**	函 数  功 能: 
**	输 入  参 数: uint8 *pAddr  
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 SetHostPort(uint16 Port)
{
	uint8 Err = NO_ERR;
	UpCommParaSaveType TermPara;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	TermPara = gPARA_UpPara;
	OS_EXIT_CRITICAL();
	
	TermPara.WriteFlag = 0xAA55; 
//	memcpy(TermPara.HostPort, *Port, 16);
	TermPara.HostPort =  Port;
	OSMutexPend (FlashMutex,0,&Err);
    #if 0
    Err =AppSdWrRdBytes(UP_COMM_PARA, sizeof(UpCommParaSaveType), (uint8 *)&TermPara, SD_WR);
	#else
	Err = SDSaveData("/UP_COMM_PARA", &TermPara, sizeof(UpCommParaSaveType), 0);
	#endif
    OSMutexPost (FlashMutex);
    
    OS_ENTER_CRITICAL();
	gPARA_UpPara = TermPara;
	OS_EXIT_CRITICAL();

	return Err;
}

/****************************************************************************************************
**	函 数  名 称: SaveDeviceType
**	函 数  功 能: 保存设备类型
**	输 入  参 数: uint8 DeviceType
**	输 出  参 数: 无
**  返   回   值: 无
**	备		  注: 
*****************************************************************************************************/
uint8 SaveDeviceType(uint8 DeviceType)
{
	uint8 Err = NO_ERR;
	TermParaSaveType TermPara;
	CPU_SR			cpu_sr;
	
	OS_ENTER_CRITICAL();
	TermPara = gPARA_TermPara;
	OS_EXIT_CRITICAL();
	
	TermPara.WriteFlag = 0xAA55;
	TermPara.DeviceType = DeviceType;
	
	OSMutexPend (FlashMutex,0,&Err);
	Err = SDSaveData("/TERMINAL_PARA", &TermPara, sizeof(TermParaSaveType), 0);
    OSMutexPost (FlashMutex);
    
    OS_ENTER_CRITICAL();
	gPARA_TermPara = TermPara;
	OS_EXIT_CRITICAL();

	return Err;
}

/********************************************************************************************************
**                                               End Of File										   **
********************************************************************************************************/

