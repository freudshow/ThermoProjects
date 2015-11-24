

#ifndef  __USERCONFIG_H__
#define  __USERCONFIG_H__


#define   _MBUS_DETACH_

/************************************* IWDT Config  ******************************************************/
//		  调试时，将WDT_TYPE 设为0 ，以防止看门狗影响调试。
//        调试完毕，务必将 WDT_TYPE 设为1，WDT_TASK_MONIT_EN设为1 ，以使能看门狗

#define   WDT_TYPE  				1				// 0:不使用看门狗 ; 1:使用内狗
#define   WDT_TASK_MONIT_EN   		1		//看门狗监视任务 使能监视 或不使能
#define   WDT_TASK_FEED_EN  			1			//使能各任务 喂任务狗  或不使能

/************************************* UART Config  ******************************************************/
/*begin:yangfei modified 2013-01-30*/
//#define	DEBUG_BAUNDRATE		9600					//usart1
#define	DEBUG_BAUNDRATE		115200		
/*end:yangfei modified 2013-01-30*/
#define	DEBUG_CHECK			USART_Parity_No

//#define	MBUS_BAUNDRATE		2400					//usart2
//#define	MBUS_CHECK			USART_Parity_Even
#if 0
#define	GPRS_BAUNDRATE		57600					//usart3
#else
#define	GPRS_BAUNDRATE		115200					//usart3
#endif
#define	GPRS_CHECK			USART_Parity_No

#define	ZIGBEE_BAUNDRATE	9600					//uart4
#define	ZIGBEE_CHECK		USART_Parity_No

#define	RS485_BAUNDRATE		1200					
#define	RS485_CHECK			USART_Parity_Even

#define MBUS_DEFAULT_PARA 	WAN_HUA	


/*begin:yangfei added 2014-02-24 for support 时间通断面积法热分配表法*/
/*设备类型*/
#define HEAT_METER_TYPE 0x20      /*热计量表*/
#define HEATCOST_METER_TYPE 0xA0  /*电子式热分配表0xA0*/
#define TIME_ON_OFF_AREA_TYPE 0xB0/*时间通断面积法0XB0*/
/*end:yangfei added 2014-02-24 for support 时间通断面积法热分配表法*/

/************************************* GPRS Config  ******************************************************/

#if   TEST_SELF_CHECK > 0  //自检程序有效时的GPRS模块检查周期
#define  GPRS_CHECK_CYCLE	 			3
#else

//GPRS模块检查周期 时间单位为秒, 最小为10秒,最大为60秒  ,一般情况下不要更改，保持为 10 
#define  GPRS_CHECK_CYCLE	 			10
#endif



#if   TEST_SELF_CHECK > 0
#define  GPRS_SELF_CHECK_CYCLE	 		5*GPRS_CHECK_CYCLE
#else
//GPRS模块自检周期 时间单位为秒,必须为GPRS_CHECK_CYCLE的整数倍,程序以此时间为周期对GPRS模块的状态进行查询，并记录，作为是否需要重启的依据。
//最小为GPRS_CHECK_CYCLE秒,最大为5*GPRS_CHECK_CYCLE，调试时可设置很大，
//如100*GPRS_CHECK_CYCLE，以防止数据对本地口通信有影响 
#define  GPRS_SELF_CHECK_CYCLE	 		5*GPRS_CHECK_CYCLE
#endif



#if   TEST_SELF_CHECK > 0
#define  GPRS_HEART_FRM_TIME			6
#else

//GPRS心跳帧时间间隔,时间单位为秒,最小为2*GPRS_CHECK_CYCLE,
//注意,实际的心跳帧发送时间是以GPRS_CHECK_CYCLE为时间单位的,可能最多会慢GPRS_CHECK_CYCLE
#define  GPRS_HEART_FRM_TIME		60//30	
#endif


//接收帧看门狗的溢出值,时间单位为秒,在心跳帧间隔到达后,经过这个时间若仍没有收到任何帧,则重启GPRS模块
#define  GPRS_HEART_FRM_REC_OUTTIME		GPRS_HEART_FRM_TIME*4

/************************************* LOG Config  ******************************************************/
#define	LOG_FULL_ASSERT

//#define	LOG_INIT	(LOG_LEVEL_ERROR|LOG_LEVEL_WARN)
#define	LOG_INIT			LOG_ALL
#define PARA_READ_ALLOW //muxiaoqing add参数读取
#define HW_VERSION_1_01 //muxiaoqing add to control HW feature
#define SW_VERSION  1401//软件版本号版本，14.00版,从zjjin温控计量一体化改进而来，通断时间面积法初始版本为14.00.
#define HW_VERSION  101  // 硬件版本号版本，1.00版。

#define DEMO_APP   //muxiaoqing 0718

#define METERDATALEN	40    //每条热表地址信息长度，当前为40字节。

#endif

/*********************************************************************************************************
      								END OF FILE
*********************************************************************************************************/

