/*=================================Copyright (c)===========================
                             山东华宇空间技术公司
                             
 ==---------------------------文件信息-------------------------------------                                 
  文   件   名: BaseProtocol.h
  创   建   人: 勾江涛
  版   本   号: 0.1
  创  建 日 期: 2012年8月6日
  描        述: 各种协议的帧外层结构，使用的常数
 ==------------------------------------------------------------------------
==========================================================================*/

#ifndef _STRUCT_PROTOCOL_H
#define _STRUCT_PROTOCOL_H

#include <includes.h>

	#ifdef __cplusplus
	extern "C" {
	#endif

#define  FRAME_START  		0x7B           	//帧起始字节
#define  FRAME_END    		0x7D           	//帧结束字节
#define MAX_FRMBUF_LEN      5120     		//最大的帧缓冲区长度 5K

#pragma pack(1)
typedef struct DataStruct
{
 	uint8* pData;
 	uint16 lenData;
 	uint8* pDataBack;
 	uint16 lenDataBack;
}DataField;

typedef  struct
{                
	uint8 _Ver;								//协议版本号
	uint8 SourceAddr[6];		    		//源地址，发送方地址
 	uint8 DestAddr[6];						//目的地址，接收方地址
 	uint8 MsgID;							//消息序列
 	uint16 MsgLength;						//消息体长度
 	uint8 MsgType;							//消息类型
 	uint8 TimeSmybol[6];					//TS 时间标签
 	uint8 HeadCheck;						//消息头效验
 	uint8 ContentCheck;						//消息体效验
 	
   	DataField Data;       					//	数据域
} _ProtocolType;

typedef struct SelfDefineParaStruct
{
	
	uint8  LCDContrastValue;//液晶对比度值
	uint8  ChannelType;//通道类型，0:GPRS,1：CDMA,2:以太网，3:SMS,4:专用网络
	
}SelfDefinePara;

//通讯参数设置
typedef struct CommParaStruct  
{
  	//终端为客户时使用的通信参数GPRS/CDMA
  	uint8  PPIP1[4];         				//主用IP
  	uint8  PPIP2[4];        			 	//备用IP
  	uint16 PPPort;           				//前置机端口 
  	uint16 PPport;           				//前置机备用端口  
  	uint8  APNName[16];      			// APN名称
  	
  	uint8  MasPhNum[8];      			//主站电话号码
  	uint8  SmsNum[8];       				 //短信中心号码
  	
  	uint8  SocketType;       			//GPRS通讯方式：TCP/UDP     0:TCP    1:UDP
  	
  	uint8  HeartCycle;       				//心跳周期
  	uint8  flagCon;          				//需要主站确认的通讯服务
  	
  	uint8  InterfacePw[6];   			//界面密码，6字节ASCII码
  	uint8  Jzqpassword[16];  	 		//集中器密码 
  	
  	//终端以太网通信参数 
  	uint8  TermIP[4];
  	uint8  TermSubMask[4];       		//子网掩码
  	uint8  TermGateWay[4];       		//网关 
  	uint8  UserName[16];  				//登录名
  	uint8  lenUserName;
  	uint8  UserPw[16];    				//登录密码  
  	uint8  AgentSer[4];   				//代理服务器
  	uint16 AgentSerport;    		 	//代理服务器端口
  	uint8  AgentConnMode;        		//代理连接方式
  	uint16 TermPort;         				//终端监听端口
  	uint8  DNSName[40];                     //动态域名muxiaoqing
  	uint8  SwitchIPorDNS;                   //IP地址与域名切换muxiaoqing
  	
	uint8 RelandDelay;						//掉线重新登录延时时间，单位为 Min
	uint8 RelandTimes;						//掉线重新登录次数
  	uint8  HeartFlag;        				//心跳开关
} CommPara;

typedef	struct	GlobalVariableStruct
{
	uint8 addr[5];
	uint8  TermPw[16];      //集中器密码
	uint8  InterfacePw[6]; //界面密码，6字节ASCII码           //IP	
	uint8  PPIP[15];       //前置机IP
	uint8  LenPPIP;        //IP长度
 	uint8  PPPort[5];      //前置机端口 
	uint8  LenPPPort;      //端口号长度 
	uint8  APNName[16];    //APN名称
	uint8  LenAPNName;     //APN名称长度
	uint8  HeartCycle;     //心跳周期
	uint8  SocketType;//0:TCP;1:UDP
	uint8  ChannelType;//通道类型，0:GPRS,1：CDMA,2:以太网，3:SMS,4:专用网络

	uint8  AllowAutoToMas; //允许或禁止主动上传
	
	//以太网参数
	uint8  TermIP[4];
	//uint8  LenTermIP;
	uint8  TermSubMask[4];       //子网掩码
	//uint8  LenTermSubMask;
	uint8  TermGateWay[4];       //网关
	//uint8  LenTermGateWay;
	uint16 TermPort;        //uint8 TermPort[5]; //终端监听端口
	//uint8  LenTermPort;
	uint8  TermMACAddr[6];		//以太网MAC地址
	
	
	//其他参数
	uint8 LCDContrastValue;
	uint8 ErrCode; //不存储
}GlobalPara;

#pragma pack()

	#ifdef __cplusplus
	}
	#endif
#endif
