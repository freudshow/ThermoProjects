 /*=================================Copyright (c)===========================
                             山东华宇空间技术公司
                             
 ==---------------------------文件信息-------------------------------------                                 
  文   件   名: BaseProtocol.h
  创   建   人: 勾江涛
  版   本   号: 0.1
  创  建 日 期: 2012年8月6日
  描        述: 各种协议的基类
 ==------------------------------------------------------------------------
==========================================================================*/

#ifndef _BaseProtocol_H
#define _BaseProtocol_H

class BaseProtocol{
	
	protected:
 	  
 	BaseProtocol(){ frameStart = FRAME_START;frameEnd = FRAME_END;} 
	public : 
 		uint16 minFrameLen;      //协议最短长度
		uint16 FrameLen;      
		uint8 frameStart;
		uint8 frameEnd;	  
		//计算帧校验和
		//uint8 CountCS(uint8* _data,uint16 _len);//累加校验
		//uint8 CountCS(uint8* _data,uint16 _len,uint8 cs);//累加校验
		//uint16 CRCCheck(uint8* _data,uint16 _len);  //CRC校验
		//uint16 CRCCheck_Table(uint8 *puchMsg, uint16 usDataLen,uint16 CRC); /* puchMsg:要进行CRC校验的消息  usDataLen消息中字节数 */ 
		uint8 flagFir; //帧头标志
	   	uint8 flagFin; //帧尾标志
		//static void  Reversal(uint8* pStr,uint16 len) ;  //颠倒字符串
		
		//void   Reversalmemcpy(uint8 *destin, uint8 *source, uint16 n);//从源地址拷贝N个字节反向存到目的地址 
	    
	
	      
		//组帧函数，子类中实现（纯虚函数）  
		virtual void CreateFrame(uint8* _data,uint8* _sendFrame,uint16 &lenFrame) = 0;    
		virtual uint8 CreateHeartFrm(uint8* HeartFrm,uint8 flag)=0;
		//按照协议接收数据帧函数，子类中实现（纯虚函数）
		 
		virtual uint8 ReceiveFrame(uint8* _receFrame,uint16 &lenFrame)=0;
		virtual uint8 AnalysisFrame(uint8* _receBuf,uint16 &lenBuf)=0;
		virtual uint8 IfFrameHead(uint8 bHead)=0;
		

};

#endif
