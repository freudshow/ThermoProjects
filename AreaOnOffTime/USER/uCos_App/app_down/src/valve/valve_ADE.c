/*
  ********************************************************************************************************
  * @file    valve_ADE.c
  * @author  zjjin
  * @version V0.0.0
  * @date    08-10-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		航天德鲁协议阀控器功能实现相关代码，相对于原德鲁阀控协议有些调整
  		为了与原兼容，另加本文件。
  *
  ********************************************************************************************************
  */  

//#include <includes.h>
#include <string.h>
#include "Valve.h"  
#include "valve_ADE.h"


/*
**************************************************************************************************
uint16 gADEVALVE_Table[][2],航天德鲁阀控器协议，功能区分用。
						gADEVALVE_Table[][0] --- 阀控功能标识号。
						gADEVALVE_Table[][1] ---指令数据长度。

**************************************************************************************************
*/
uint16 gADEVALVE_Table[][2] = {
		0x17A0, 0x04,		//0,下发控制阀开度命令.
		0x25A0, 0x2E,		//1,下发热量数据到阀控器。
		0x20A0, 0x0B,		//2,读室内温度。
		0x21A0, 0x10,		//3,设置室内上下限温度。
		0x22A0, 0x0B,		//4,设置阀门控制方式，使能、禁能、自动、定时等。
		0x23A0, 0x0D,   	//5，设置室内温度。
		0x24A0, 0x04,   	//6，读阀门状态。
		0x27A0, 0x0D,   	//7，设置补偿温度
		0x28A0, 0x0B,   	//8，设置阀控器启动温差
		0x29A0, 0x0B,   	//9，设置温控面板温度采集周期。
		0x30A0, 0x0B,   	//10，设置温控面板温度上报周期。
		0x31A0, 0x0B,   	//11，读取阀控器数据，包括开阀时间、进回水温度、室内温度。
		0x32A0, 0x16,   	//12，下发写入分摊热量值。
		
		
};




/*
  ********************************************************************************************************
  * 函数名称: uint8 ValveContron_ADE(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
  
  * 说    明：航天德鲁阀控器控制函数，在此函数中实现亿林阀控不同控制。
  *					
  * 输入参数： 
  				MeterFileType *p_mf   表参数，含有阀控协议版本、阀控地址等信息。
  				uint8 functype  对阀进行什么样的控制，比如读信息、设置上下限温度等。
				uint8 *p_datain  输入函数可能要使用的数据。
  				uint8 *p_databuf    从阀控中读上来的数据存储指针。
  				uint8 p_datalenback  从阀控中读上来的数据长度。
  * 输出参数:
  				执行是否成功。
  ********************************************************************************************************
  */

uint8 ValveContron_ADE(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
{
	uint8 Err = 0;
	uint8 lu8DataIn[50] = {0};  //注意不要超限溢出。
	uint8 lu8databuf[50] = {0};	//注意不要超限溢出。
	uint8 lu8datalen = 0;
	uint8 lu8BackDataBuf[50] = {0};	//注意不要超限溢出。
	uint8 lu8datalenback = 0;
	uint8 lu8ReadTime[6] = {0};
	CJ188_Format *p_CJ188Data = NULL;

	switch(functype){
		case ReadVALVE_All:{	//对德鲁协议来说，包括读室内温度和阀门状态2方面。
			//先读室内温度。
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			lu8DataIn[7] = 0x11;
			Err = ADEValveCommunicate(p_mf,ADE_READROOM_TEMP,lu8DataIn,lu8databuf,&lu8datalen);

			if(Err == NO_ERR){
				memcpy(lu8BackDataBuf,lu8databuf,3);  //考来温度数据。
				lu8datalenback += 3;
				
			}
			else{
				memset(lu8BackDataBuf, 0xee, 3);	  //温度3个字节
				lu8datalenback += 3;
			}

			//读取阀门状态。
			lu8DataIn[0] = 0x11;
			Err = ADEValveCommunicate(p_mf,ADE_READVALVE_STATUS,lu8DataIn,lu8databuf,&lu8datalen);
			if(Err == NO_ERR){     
					uint16  ValveState = 0;
					ValveState = *(uint16*)lu8databuf;
					if(ValveState == 0x0800||ValveState == 0x9900){
						ValveState = 0x0099;
					}/*全关*/
					else if(ValveState == 0x0400||ValveState == 0x8800){ValveState = 0x0088;}
					else if(ValveState == 0x0200||ValveState == 0x7700){ValveState = 0x0077;}
					else if(ValveState == 0x0100||ValveState == 0x6600){ValveState = 0x0066;}
					else if(ValveState == 0x0000||ValveState == 0x5500){
						ValveState = 0x0055;
					}/*全开*/
					else{
						ValveState = 0x0099;
					}

					lu8BackDataBuf[3] = (uint8)ValveState;  //阀门开度字节

					lu8BackDataBuf[4] = 0;//按照格式补齐。
					lu8BackDataBuf[5] = 0;

					lu8datalenback += 3;
					
			}
			else{
				memset(&lu8BackDataBuf[3], 0xee, 3);								//温度3个字节
				lu8datalenback += 3;
			}

			memcpy(p_databuf,lu8BackDataBuf,lu8datalenback);
			*p_datalenback = lu8datalenback;
			
			break;
		}

		case READROOM_TEMP:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			lu8DataIn[7] = 0x11;
			Err = ADEValveCommunicate(p_mf,ADE_READROOM_TEMP,lu8DataIn,lu8databuf,&lu8datalen);
			if(Err == NO_ERR){
				memcpy(lu8BackDataBuf,lu8databuf,3);  //考来温度数据。
				lu8datalenback += 3;
				
			}
			else{
				memset(lu8BackDataBuf, 0xee, 3);	  //温度3个字节
				lu8datalenback += 3;
			}

			memcpy(p_databuf,lu8BackDataBuf,lu8datalenback);
			*p_datalenback = lu8datalenback;

			break;
		}

		case SETHEAT_DISPLAY:{
	
			break;
		}

		case SETHEAT_VALUE:{
			memcpy(lu8DataIn,p_DataIn,gADEVALVE_Table[ADE_SETHEAT_VALUE][1] - 3);
			//改掉时间为当前时间。航天德鲁阀控协议要求。
			ReadDateTime(lu8ReadTime);
			p_CJ188Data = (CJ188_Format *)lu8DataIn;
			memcpy(p_CJ188Data->RealTime,lu8ReadTime,6);
			p_CJ188Data->RealTime[6] = 0x20;  //固定为20xx年。
			Err = ADEValveCommunicate(p_mf,ADE_SETHEAT_VALUE,lu8DataIn,lu8databuf,&lu8datalen);
			if(Err == NO_ERR){
				//debug_info(gDebugModule[TASKDOWN_MODULE],"%s %d Send HeatMeter data  to valve ok",__FUNCTION__,__LINE__);
			}
			else  {
				//debugX(LOG_LEVEL_ERROR,"%s %d Send HeatMeter data  to valve  failed!\r\n",__FUNCTION__,__LINE__);
			}

			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;

			break;

		}

		case SETROOM_TEMP:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			memcpy(&lu8DataIn[7],p_DataIn,3);
			Err = ADEValveCommunicate(p_mf,ADE_SETROOM_TEMP,lu8DataIn,lu8databuf,&lu8datalen);
			
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETTEMP_RANGE:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			memcpy(&lu8DataIn[7],p_DataIn,6);
			Err = ADEValveCommunicate(p_mf,ADE_SETTEMP_RANGE,lu8DataIn,lu8databuf,&lu8datalen);
			
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETVALVE_STATUS:{
			lu8DataIn[0] =  *(p_DataIn + 0);
			Err = ADEValveCommunicate(p_mf,ADE_SETVALVE_STATUS,lu8DataIn,lu8databuf,&lu8datalen);
			
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETVALVE_CONTROLTYPE:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			if(*(p_DataIn+0) == 0x09){
					lu8DataIn[7]		= 0x55;
			}
			if(*(p_DataIn+0) == 0x0B){
					lu8DataIn[7]		= 0x66;
			}
			if(*(p_DataIn+0) == 0x0C){
					lu8DataIn[7]		= 0x77;
			}
			if(*(p_DataIn+0) == 0x0D){
					lu8DataIn[7]		= 0x88;
			}
			Err = ADEValveCommunicate(p_mf,ADE_SETVALVE_CONTROLTYPE,lu8DataIn,lu8databuf,&lu8datalen);
			
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;


			break;
		}

		case READVALVE_STATUS:{
			//读取阀门状态!!
			lu8DataIn[0] = 0x11;
			Err = ADEValveCommunicate(p_mf,ADE_READVALVE_STATUS,lu8DataIn,lu8databuf,&lu8datalen);
			
			if(Err == NO_ERR){     
					uint16  ValveState = 0;
					ValveState = *(uint16*)lu8databuf;
					if(ValveState == 0x0800||ValveState == 0x9900){
						ValveState = 0x0099;
					}/*全关*/
					else if(ValveState == 0x0400||ValveState == 0x8800){ValveState = 0x0088;}
					else if(ValveState == 0x0200||ValveState == 0x7700){ValveState = 0x0077;}
					else if(ValveState == 0x0100||ValveState == 0x6600){ValveState = 0x0066;}
					else if(ValveState == 0x0000||ValveState == 0x5500){
						ValveState = 0x0055;
					}/*全开*/
					else{
						ValveState = 0x0099;
					}

					lu8BackDataBuf[0] = (uint8)ValveState;  //阀门开度字节
					lu8datalenback += 1;
					
			}

			memcpy(p_databuf,lu8BackDataBuf,lu8datalenback);
			*p_datalenback = lu8datalenback;

			break;
		}

		case READ_VALVEDATA:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			lu8DataIn[7] = 0;
			Err = ADEValveCommunicate(p_mf,ADE_READ_VALVEDATA,lu8DataIn,lu8databuf,&lu8datalen);
			
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case WRITE_DIVHEATDATA:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			//加入当前时间，航天德鲁阀控协议要求。
			ReadDateTime(lu8ReadTime);
			memcpy(&lu8DataIn[7],lu8ReadTime,6);
			lu8DataIn[13] = 0x20; //固定为20XX年。
			
			memcpy(&lu8DataIn[14],p_DataIn,5);  //分摊热量值加入。
			Err = ADEValveCommunicate(p_mf,ADE_WR_HEATDATA,lu8DataIn,lu8databuf,&lu8datalen);
			if(Err == NO_ERR){
				//debug_info(gDebugModule[TASKDOWN_MODULE],"%s %d Send HeatMeter data  to valve ok",__FUNCTION__,__LINE__);
			}
			else  {
				//debugX(LOG_LEVEL_ERROR,"%s %d Send HeatMeter data  to valve  failed!\r\n",__FUNCTION__,__LINE__);
			}

			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETVALVE_OFFSETTEMP:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			memcpy(&lu8DataIn[7],p_DataIn,3);
			Err = ADEValveCommunicate(p_mf,ADE_SETVALVE_OFFSETTEMP,lu8DataIn,lu8databuf,&lu8datalen);
	
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;

			break;
		}

		case SETVALVE_HYSTEMP:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			lu8DataIn[7] = *(p_DataIn + 0);
			Err = ADEValveCommunicate(p_mf,ADE_SETVALVE_HYSTEMP,lu8DataIn,lu8databuf,&lu8datalen);
	
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;


			break;
		}

		case SETTEMP_GATHERPERIOD:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			lu8DataIn[7] = *(p_DataIn + 0);
			Err = ADEValveCommunicate(p_mf,ADE_SETTEMP_GATHERPERIOD,lu8DataIn,lu8databuf,&lu8datalen);
	
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;


			break;
		}

		case SETTEMP_UPLOADPERIOD:{
			memcpy(lu8DataIn,p_mf->ControlPanelAddr,7);
			lu8DataIn[7] = *(p_DataIn + 0);
			Err = ADEValveCommunicate(p_mf,ADE_SETTEMP_UPLOADPERIOD,lu8DataIn,lu8databuf,&lu8datalen);
	
			memcpy(p_databuf,lu8databuf,lu8datalen);
			*p_datalenback = lu8datalen;


			break;
		}

			

		default:
			break;
	}



	return Err;
}


/*
  ********************************************************************************************************
  * 函数名称: uint8 ADEValveCommunicate(MeterFileType *p_mf,uint8 ADE_Functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
  
  * 说    明：航天德鲁阀控器协议通讯实现函数。根据功能ADE_Functype从gADEVALVE_Table中调用相应
  				功能码，如果抄成功，则返回抄阀的数据域全部内容。
  *					
  * 输入参数： 
  				MeterFileType *p_mf   表阀基础数据。
  				uint8 ADE_Functype  对阀进行什么样的控制，比如读信息、设置上下限温度等。
				uint8 *p_datain  输入函数可能要使用的数据。
  				uint8 *p_databuf    从阀控中读上来的数据存储指针。
  				uint8 p_datalenback  从阀控中读上来的数据长度。
  * 输出参数:
  				执行是否成功。
  ********************************************************************************************************
  */
uint8 ADEValveCommunicate(MeterFileType *p_mf,uint8 ADE_Functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
{
	uint8 Err = 0;
	DELU_Protocol	ProtocoalInfo;

	ProtocoalInfo.PreSmybolNum  = 0x02;
 	ProtocoalInfo.MeterType 	= 0x20;
	memcpy(ProtocoalInfo.MeterAddr, p_mf->ValveAddr, 7);
	ProtocoalInfo.ControlCode 	= 0x04;
	ProtocoalInfo.Length		= gADEVALVE_Table[ADE_Functype][1];
	ProtocoalInfo.DataIdentifier= gADEVALVE_Table[ADE_Functype][0];
	ProtocoalInfo.SER = 0;
	memcpy(ProtocoalInfo.DataBuf, p_DataIn, ProtocoalInfo.Length-3);  //考入数据域内容。
						
	Err = METER_DataItem(&ProtocoalInfo);

	if(Err == NO_ERR){
		memcpy(p_databuf, ProtocoalInfo.DataBuf, (ProtocoalInfo.Length-3));
		*p_datalenback = ProtocoalInfo.Length - 3;
	}

	
	return Err;


}

