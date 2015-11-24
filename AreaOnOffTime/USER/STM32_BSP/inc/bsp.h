/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210B-EVAL Evaluation Board
*
* Filename      : bsp.h
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************
*/

#ifndef  __BSP_H__
#define  __BSP_H__

/*
*********************************************************************************************************
*                                               DEFINE
*********************************************************************************************************
*/
typedef unsigned char  uint8;                                           /*  无符号8位整型变量           */
typedef signed   char  int8;                                            /*  有符号8位整型变量           */
typedef unsigned short uint16;                                          /*  无符号16位整型变量          */
typedef signed   short int16;                                           /*  有符号16位整型变量          */
typedef unsigned int   uint32;                                          /*  无符号32位整型变量          */
typedef unsigned long long  uint64; 
typedef signed   int   int32;                                           /*  有符号32位整型变量          */
typedef float          fp32;                                            /*  单精度浮点数（32位长度）    */
typedef double         fp64;                                            /*  双精度浮点数（64位长度）    */

typedef struct {
	uint8 Sta;
	uint32 DogCnt;
	uint32 DogCntThrd;
} BLT_CTR ;
/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#ifdef __cplusplus
	extern "C" {
	#endif
#include "stm32f10x_it.h"
#include "SPI_MSD_Driver.h"	 
#include "queue.h"
#include "Usart1.h"
#include "Usart2.h"
#include "Usart3.h"
#include "Uart4.h"
#include "Uart5.h"
#include "rx8025.h"
#include "commap.h"
#include "gpioint.h"
#include "TaskDog.h"
#include "Meter_Table.h"
#include "gprs.h"
	#ifdef __cplusplus
	}
	#endif
#include "lcmdrv.h"

/************************************* Up Down Right Left Ok Cancel **************************************/
#define	KEY_CANCEL_PIN		GPIO_Pin_6		//pc
#define KEY_UP_PIN			GPIO_Pin_7		
#define KEY_DOWN_PIN		GPIO_Pin_8
#define	KEY_RIGHT_PIN		GPIO_Pin_9
#define	KEY_LEFT_PIN		GPIO_Pin_11		//pa
#define KEY_OK_PIN			GPIO_Pin_12

#define STA_KEY_CANCEL()	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6)
#define STA_KEY_UP()		GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define STA_KEY_DOWN()		GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)
#define	STA_KEY_RIGHT()		GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)
#define STA_KEY_LEFT()		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11)
#define	STA_KEY_OK()		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12)
#ifdef HW_VERSION_1_01

#define STA_PWR_OFF()	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 0 ? 1 : 0
#define GPIO_SD_CHECK_STATUS()	GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 1 ? 1 : 0
#define GPIO_GPRS_CHECK_STATUS()	GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 1 ? 1 : 0
#define STA_MBUS_OFF()	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 1 ? 1 : 0
#endif
/************************************* LCM Control Lines  ************************************************/
#define LCM_LIGHT			GPIO_Pin_7		//pg
#define LCM_RESET			GPIO_Pin_8

/************************************* RS485 Control Lines  **********************************************/
#if 0
#define	RS485_SEND_DIR()	GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define	RS485_RCV_DIR()		GPIO_ResetBits(GPIOA, GPIO_Pin_8)
#endif
#ifdef   _MBUS_DETACH_
#define	RS485_SEND_DIR()	GPIO_SetBits(GPIOD, GPIO_Pin_3)
#define	RS485_RCV_DIR()		GPIO_ResetBits(GPIOD, GPIO_Pin_3)

#define	RS485_USART4_SEND_DIR()	GPIO_SetBits(GPIOD, GPIO_Pin_6)
#define	RS485_USART4_RCV_DIR()  GPIO_ResetBits(GPIOD, GPIO_Pin_6)
#else
#define	RS485_RCV_DIR()	GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define	RS485_SEND_DIR()		GPIO_ResetBits(GPIOA, GPIO_Pin_8)
#endif
/************************************* GPRS Control Lines  **********************************************/
#define	GPRS_RESET_ENABLE()	GPIO_ResetBits(GPIOE, GPIO_Pin_3)
#define	GPRS_RESET_DISABLE() GPIO_SetBits(GPIOE, GPIO_Pin_3)

#define GPRS_POW_OFF()		GPIO_ResetBits(GPIOE, GPIO_Pin_2)
#define GPRS_POW_ON()		GPIO_SetBits(GPIOE, GPIO_Pin_2)

/************************************* MBUS Channels Channel1,2,3,4,5,6 **********************************/
#define MBUS_Channel_1		GPIO_Pin_4		//pa
#define MBUS_Channel_2		GPIO_Pin_5
#define MBUS_Channel_3		GPIO_Pin_6
#define MBUS_Channel_4		GPIO_Pin_7
#define MBUS_Channel_5		GPIO_Pin_4		//pc
#define MBUS_Channel_6		GPIO_Pin_5
#ifdef HW_VERSION_1_01
#define ENABLE_MBUS_1()		GPIO_SetBits(GPIOA,GPIO_Pin_5);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4 | GPIO_Pin_5);\
							gCurrent_Channel = 1

							
#define ENABLE_MBUS_2()		GPIO_SetBits(GPIOA,GPIO_Pin_4);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4 | GPIO_Pin_5);\
							gCurrent_Channel = 2

#else
#define ENABLE_MBUS_1()		GPIO_SetBits(GPIOA,GPIO_Pin_4);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4 | GPIO_Pin_5);\
							gCurrent_Channel = 1
							
#define ENABLE_MBUS_2()		GPIO_SetBits(GPIOA,GPIO_Pin_5);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4 | GPIO_Pin_5);\
							gCurrent_Channel = 2
#endif							
#define ENABLE_MBUS_3()		GPIO_SetBits(GPIOA,GPIO_Pin_6);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4 | GPIO_Pin_5);\
							gCurrent_Channel = 3
							
#define ENABLE_MBUS_4()		GPIO_SetBits(GPIOA,GPIO_Pin_7);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_4);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4 | GPIO_Pin_5);\
							gCurrent_Channel = 4
							
#define ENABLE_MBUS_5()		GPIO_SetBits(GPIOC,GPIO_Pin_4);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_5);\
							gCurrent_Channel = 5
							
#define ENABLE_MBUS_6()		GPIO_SetBits(GPIOC,GPIO_Pin_5);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4);\
							gCurrent_Channel = 6
							
#define DISABLE_MBUS()		GPIO_ResetBits(GPIOC,GPIO_Pin_5);\
							GPIO_ResetBits(GPIOA,GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);\
							GPIO_ResetBits(GPIOC,GPIO_Pin_4);\
							gCurrent_Channel = 0


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/

 extern void BSP_Init(void);
 extern void BSP_All_ControlLines_Init(void);
 extern void NVIC_Configuration_EXTI(void);
 extern void EXTI_Configuration(void);
 extern void BSP_Key_Init(void);
 extern void BSP_I2C_Init(void);
 extern void NVIC_Configuration_USART(void);
 extern void BSP_USART1_Init(void);
 extern void BSP_USART2_Init(void);
 extern void BSP_USART3_Init(void);
 extern void BSP_UART4_Init(void);
 extern void BSP_UART5_Init(void);
 extern void BSP_SD_SPI_Init(void);
 extern void BSP_FSMC_IO_Init(void);
 extern void BSP_LCM_Init(void);
 extern void BSP_SRAM_Init(void);
 extern void ProBltStaDog(void);
 extern uint8 ProBltSta(void);
 extern void IWDG_Configuration(void);
 extern void METER_ComSet1(void);
 extern void METER_ComSet2(void);
 extern void METER_ComSet3(void);
 extern void METER_ComSet4(void) ;
 extern void METER_ComSet5(void) ;
 extern void METER_ComSet6(void) ;
 extern void METER_ComSet7(void) ;
 extern void BSP_Gprs_CtrInit_Xmz(void);
 /*begin:yangfei added 2013-01-16*/
 extern void CRC_Init(void);
 extern void BKP_Init(void);
 extern void USART3_BAUNDRATE_Init(uint32  BAUNDRATE); 
 extern void BSP_GPIOPolling_Init(void);//muxiaoqing add


 extern void  METER_ComSet6(void); //林晓彬添加
#endif                                                          /* End of module include.                               */
