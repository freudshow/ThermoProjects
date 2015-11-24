/*
  ********************************************************************************************************
  * @file    Valve.c
  * @author  zjjin
  * @version V0.0.0
  * @date    04-15-2015
  * @brief   
  ********************************************************************************************************
  * @attention
  *		为在集中器中实现多厂家不同协议阀门的兼容，在表基础信息中加入了
  *	阀控协议版本，各厂家阀控器相关程序在本文件中。
  *
  ********************************************************************************************************
  */  
//#include <includes.h>

#include "Valve.h"
#include "valve_elsonic.h"
#include "valve_binihi.h"
#include "valve_delu.h"
#include "valve_joyo.h"
#include "valve_lcarbo.h"
#include "valve_precise.h"
#include "valve_ADE.h"






/*
**************************************************************************************************
uint8 gVALVE_Table[][2],阀控器多协议支持，
						gVALVE_Table[][0] --- 阀控串口参数设置；
						gVALVE_Table[][1] --- 阀控器协议版本号。

**************************************************************************************************
*/
uint8 gVALVE_Table[][2] = {
		COMSET_1, VALVE_DELU,		//0,德鲁阀控(COMSET_x待定)，协议版本为0.       
		COMSET_5, VALVE_ELSONIC,	//1,亿林阀控，协议版本为1.
		COMSET_5, VALVE_JOYO,		//2,京源阀控,协议版本为2.
		COMSET_7, VALVE_BINIHI,		//3,百暖汇阀控，协议版本为3.
		COMSET_1, VALVE_LCARBO,		//4,琅卡博阀控(COMSET_x待定)，协议版本为4。
		COMSET_1, VALVE_PRECISE,    //5，济南普赛通讯，协议版本为5.
		COMSET_1, VALVE_ADE,		//6,航天德鲁自研阀控器协议版本为6.

};




/*
  ********************************************************************************************************
  * 函数名称：uint8 ValveContron(MeterFileType *p_mf,uint8 functype,uint8 *p_databuf,uint8 p_datalenback)
  * 说    明： 阀控器控制函数，在此函数中实现不同协议阀控的兼容，
  *					同时在此函数中实现针对阀门不同功能的控制。
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

uint8 ValveContron(MeterFileType *p_mf,uint8 functype,uint8 *p_DataIn,uint8 *p_databuf,uint8 *p_datalenback)
{
	uint8 Err = 0;
	uint8 i = 0;
	uint8 lu8ReadTimes = 0;
	uint16 lu16Second = 0;
	uint16 lu16ms = 0;
	
	lu8ReadTimes = gPARA_ReplenishPara.ValveReplenishTimes + 1;  //共抄次数。
	lu16Second = gPARA_ReplenishPara.ValveInterval / 1000;   
	lu16ms = gPARA_ReplenishPara.ValveInterval % 1000;
	FeedTaskDog();

	switch(gVALVE_Table[p_mf->ValveProtocal][1]){    //根据阀控协议号，执行相应厂家协议。

		case VALVE_DELU:{
			(*METER_ComParaSetArray[gVALVE_Table[p_mf->ValveProtocal][0]])();  //设置德鲁阀门对应串口参数。
				OSTimeDlyHMSM(0,0,0,200);
			if(lu8ReadTimes > 2)  //因为德鲁协议每次里面重复3次，限定为2，最多6次。
				lu8ReadTimes = 2;
			
			for(i=0;i<lu8ReadTimes;i++){
				Err = ValveContron_Delu(p_mf,functype,p_DataIn,p_databuf,p_datalenback);
				if(Err == NO_ERR){
					break;
				}
				else{
					OSTimeDlyHMSM(0,0,lu16Second,lu16ms);
				}
			}

			break;
		}
		
		case VALVE_ELSONIC:{
			(*METER_ComParaSetArray[gVALVE_Table[p_mf->ValveProtocal][0]])();  //设置亿林阀门对应串口参数。
				OSTimeDlyHMSM(0,0,0,100);
			for(i=0;i<lu8ReadTimes;i++){
				Err = ValveContron_Elsonic(p_mf,functype,p_DataIn,p_databuf,p_datalenback);
				if(Err == NO_ERR){
					break;
				}
				else{
					OSTimeDlyHMSM(0,0,lu16Second,lu16ms);
				}
			}
			
			break;
		}
		
		case VALVE_JOYO:{
			(*METER_ComParaSetArray[gVALVE_Table[p_mf->ValveProtocal][0]])();  //设置京源阀门对应串口参数。
				OSTimeDlyHMSM(0,0,0,100);
			for(i=0;i<lu8ReadTimes;i++){
				Err = ValveContron_Joyo(p_mf,functype,p_DataIn,p_databuf,p_datalenback);
				if(Err == NO_ERR){
					break;
				}
				else{
					OSTimeDlyHMSM(0,0,lu16Second,lu16ms);
				}
			}
						
			break;
		}
		
		case VALVE_BINIHI:{
			(*METER_ComParaSetArray[gVALVE_Table[p_mf->ValveProtocal][0]])();  //设置百暖汇阀门对应串口参数。
				OSTimeDlyHMSM(0,0,0,100);
			for(i=0;i<lu8ReadTimes;i++){
				Err = ValveContron_Binihi(p_mf,functype,p_DataIn,p_databuf,p_datalenback);
				if(Err == NO_ERR){
					break;
				}
				else{
					OSTimeDlyHMSM(0,0,lu16Second,lu16ms);
				}
			}


			break;
		}
		
		case VALVE_LCARBO:{

			break;
		}

		case VALVE_PRECISE:{
			(*METER_ComParaSetArray[gVALVE_Table[p_mf->ValveProtocal][0]])();  //设置普赛通讯阀门对应串口参数。
				OSTimeDlyHMSM(0,0,0,100);
			if(lu8ReadTimes > 2)  //因为协议每次里面重复3次，限定为2，最多6次。
				lu8ReadTimes = 2;
			
			for(i=0;i<lu8ReadTimes;i++){
				Err = ValveContron_Precise(p_mf,functype,p_DataIn,p_databuf,p_datalenback);
				if(Err == NO_ERR){
					break;
				}
				else{
					OSTimeDlyHMSM(0,0,lu16Second,lu16ms);
				}
			}

			break;
		}

		case VALVE_ADE:{
			(*METER_ComParaSetArray[gVALVE_Table[p_mf->ValveProtocal][0]])();  //设置普赛通讯阀门对应串口参数。
				OSTimeDlyHMSM(0,0,0,100);
			if(lu8ReadTimes > 2)  //因为协议每次里面重复3次，限定为2，最多6次。
				lu8ReadTimes = 2;
			
			for(i=0;i<lu8ReadTimes;i++){
				Err = ValveContron_ADE(p_mf,functype,p_DataIn,p_databuf,p_datalenback);
				if(Err == NO_ERR){
					break;
				}
				else{
					OSTimeDlyHMSM(0,0,lu16Second,lu16ms);
				}
			}

			break;

		}
		
		default:
			Err = ERR_1;
			break;
			
	}



	return Err;
}


/********************************************************************************************************
**  函 数  名 称: uint8 cmpAddr(uint8 *p_addr1,uint8 *p_addr2)				    							                       
**	函 数  功 能: 比较2个表地址是否相同。
**	输 入  参 数:
**	输 出  参 数:表地址相同返回0，不同返回1.							                                   
**	备		  注:                                                                                   																									
*********************************************************************************************************/
/*
uint8 cmpAddr(uint8 *p_addr1,uint8 *p_addr2)
{
	uint8 i = 0;

	for(i=0;i<7;i++){  //表地址、阀地址都是7个字节。
		if(*(p_addr1+i) != *(p_addr2+i)){
			return 1;  //2个表地址不同，返回1.
		}

	}

	return 0;


}
*/






