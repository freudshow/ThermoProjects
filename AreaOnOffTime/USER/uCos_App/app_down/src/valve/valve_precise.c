/*
  ********************************************************************************************************
  * @file    valve_precise.c
  * @author  zjjin
  * @version V0.0.0
  * @date    06-16-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		济南普赛通讯协议阀控器功能实现相关代码。
  *
  ********************************************************************************************************
  */  

//#include <includes.h>
#include <string.h>
#include "Valve.h"  
#include "valve_precise.h"




/*
  ********************************************************************************************************
  * 函数名称: uint8 ValveContron_Precise(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
  
  * 说    明：济南普赛通讯 阀控器控制函数，在此函数中实现亿林阀控不同控制。
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

uint8 ValveContron_Precise(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
{
	uint8 Err = 0;
	uint8 lu8data[100] = {0};
	uint8 lu8databuf[20] = {0xee};
	uint8 lu8datalenback = 0;

	DELU_Protocol	ProtocoalInfo;

	ProtocoalInfo.PreSmybolNum  = 0x04;
 	ProtocoalInfo.MeterType 	= 0x41;
	memcpy(ProtocoalInfo.MeterAddr, p_mf->ValveAddr, 7);
	ProtocoalInfo.SER = 0;

	
	switch(functype){
		case ReadVALVE_All:{	//
			ProtocoalInfo.ControlCode	= 0x01;
			ProtocoalInfo.Length		= 0x03;
			ProtocoalInfo.DataIdentifier= 0x902D;
						
			Err = METER_DataItem(&ProtocoalInfo);
			if(Err == NO_ERR){
				memcpy(lu8data, ProtocoalInfo.DataBuf, (ProtocoalInfo.Length-3));
				
				lu8databuf[0] = lu8data[18];   //因协议格式，调换顺序。
				lu8databuf[1] = lu8data[19];
				lu8databuf[2] = lu8data[20];

				lu8datalenback += 3;

				lu8databuf[3] = 0x99;  //阀门开度字节,普赛节点控制器没有阀，固定写阀全关。

				lu8databuf[4] = 0;//按照格式补齐。
				lu8databuf[5] = 0;

				lu8datalenback += 3;
				
			}
			else{
				memset(lu8databuf, 0xee, 6);								//温度3个字节
				lu8datalenback += 6;
			}


			memcpy(p_databuf,lu8databuf,lu8datalenback);
			*p_datalenback = lu8datalenback;
			
			break;
		}

		case SETHEAT_DISPLAY:{
	
			break;
		}

		case SETHEAT_VALUE:{
			ProtocoalInfo.ControlCode 	= 0x04;
			ProtocoalInfo.Length		= 0x08;
			ProtocoalInfo.DataIdentifier= 0xA101;
			memcpy(ProtocoalInfo.DataBuf,p_DataIn,5);
			Err = METER_DataItem(&ProtocoalInfo);		
			if(Err == NO_ERR){
				debug_info(gDebugModule[TASKDOWN_MODULE],"%s %d Send HeatMeter data  to valve ok",__FUNCTION__,__LINE__);
			}
			else  {
				debugX(LOG_LEVEL_ERROR,"%s %d Send HeatMeter data  to valve  failed!\r\n",__FUNCTION__,__LINE__);
			}


			break;
		}

		case SETROOM_TEMP:{


			break;
		}

		case SETTEMP_RANGE:{


			break;
		}

		case SETVALVE_STATUS:{


			break;
		}

		default:
			break;
	}



	return Err;
}


