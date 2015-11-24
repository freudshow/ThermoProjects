/**********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                            (c) Copyright 2007-2008; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
**********************************************************************************************************/

/**********************************************************************************************************
*
*                                        BOARD SUPPORT PACKAGE
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210E-EVAL Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : BAN
**********************************************************************************************************/

/**********************************************************************************************************
*                                             INCLUDE FILES
**********************************************************************************************************/

#include <includes.h>

/**********************************************************************************************************
*                                           LOCAL CONSTANTS
**********************************************************************************************************/

/**********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************/


/**********************************************************************************************************
*                                            LOCAL TABLES
**********************************************************************************************************/


/**********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
**********************************************************************************************************/

/**********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
**********************************************************************************************************/


/**********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
**********************************************************************************************************/


/**********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
**********************************************************************************************************/
 #ifdef   _MBUS_DETACH_
 #define ClockSpeed             200000
 #endif

void  BSP_Init (void)
{
	uint8 err = 0;
	uint8 i = 0;

  ErrorStatus HSEStartUpStatus;                    //定义外部高速晶体启动状态枚举变量
  RCC_DeInit();                                    //复位RCC外部设备寄存器到默认值
  RCC_HSEConfig(RCC_HSE_ON);                       //打开外部高速晶振
  HSEStartUpStatus = RCC_WaitForHSEStartUp();      //等待外部高速时钟准备好
  if(HSEStartUpStatus == SUCCESS)                  //外部高速时钟已经准别好
  {	
    RCC_HCLKConfig(RCC_SYSCLK_Div1);               //配置AHB(HCLK)时钟等于==SYSCLK
    RCC_PCLK2Config(RCC_HCLK_Div1);                //配置APB2(PCLK2)钟==AHB时钟
    RCC_PCLK1Config(RCC_HCLK_Div2);                //配置APB1(PCLK1)钟==AHB1/2时钟
         
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);  //配置PLL时钟 == 外部高速晶体时钟 * 9 = 72MHz
    RCC_PLLCmd(ENABLE);                                   //使能PLL时钟
   
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)    //等待PLL时钟就绪
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);            //配置系统时钟 = PLL时钟
    while(RCC_GetSYSCLKSource() != 0x08)                  //检查PLL时钟是否作为系统时钟
    {
    }
  }


    BSP_All_ControlLines_Init();
    BSP_Key_Init();
#ifdef HW_VERSION_1_01	
	BSP_GPIOPolling_Init();
#endif
	EXTI_Configuration();
    NVIC_Configuration_EXTI();
    BSP_I2C_Init();
    BSP_USART1_Init();
    BSP_USART2_Init();
    BSP_USART3_Init();
    BSP_UART4_Init();
    BSP_UART5_Init();
    NVIC_Configuration_USART();
    BSP_SD_SPI_Init();
    //BSP_FSMC_IO_Init();
    BSP_LCM_Init();
   // BSP_SRAM_Init();

	for(i=0;i<5;i++){   //如果SD卡初始化失败，最多初始化5次。
    	err = MSD_Init();
		if(err == NO_ERR)
			break;
	}
	
	Init8025();
	TIME_BspSystemTimeInit();
	UpcomInit();
	lcd_initial();
	BSP_Gprs_CtrInit_Xmz();
	/*begin:yangfei added 2012-12-17*/
	CRC_Init();
	BKP_Init();
   // mountSd();                    /* 挂载SD  卡*/
    RS485_RCV_DIR(); /*set 485*/
	#ifdef   _MBUS_DETACH_
    RS485_USART4_RCV_DIR(); /*set USART4 ym20130704*/
	#endif
    //RS485_SEND_DIR(); /*set 485*/
	/*end   :yangfei added 2012-12-17*/ 

}
/**
  * @brief  init BKP register
  * @note   None
  * @param  None
  * @retval None
  * @author:yangfei
  * @date:2013-01-16
  */
void BKP_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR |RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
}
/****************************************************************************************************
*                                            void CRC_Init(void)
*
* Description : Enable CRC RCC.
*
* Argument(s) : none.
*
* Return(s)   : NONE.
*
* Caller(s)   : BSP_Init.
*
* Author:yangfei
*
* Note(s)     : none.
**************************************************************************************************/
void CRC_Init(void)
{
	/* Enable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
	CRC_ResetDR();
}


/**********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
**********************************************************************************************************/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    RCC_ClocksTypeDef  rcc_clocks;


    RCC_GetClocksFreq(&rcc_clocks);

    return ((CPU_INT32U)rcc_clocks.HCLK_Frequency);
}

/*******************************************************************************
* Function Name  : ProBltSta
* Description    : Configures the watch dog.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint8 ProBltSta(void) //本函数被任意按键发生时调用
{
	uint8 ret;
	CPU_SR		cpu_sr;	

	OS_ENTER_CRITICAL();
	gBltCtr.DogCnt = 0;
	if(gBltCtr.Sta==TRUE){
		ret = TRUE; //表示背光处在打开状态
	}
	else{
		LCM_LIGHT_ENABLE; //打开液晶背光
		gBltCtr.Sta=TRUE;
		ret = FALSE;
	}
	
	OS_EXIT_CRITICAL();
	
	return ret;
}

/*******************************************************************************
* Function Name  : ProBltStaDog
* Description    : Configures the watch dog.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void ProBltStaDog(void) //本函数被启动任务每1秒调用一次
{
	//OS_ENTER_CRITICAL(); 由于本函数被最高优先级任务调用,所以这里省掉关中断

	if(gBltCtr.Sta==TRUE){
		gBltCtr.DogCnt++;
		if(gBltCtr.DogCnt > gBltCtr.DogCntThrd){
			LCM_LIGHT_DISABLE; //关闭液晶背光
			gBltCtr.Sta=FALSE;
		}
	}
	
	//OS_EXIT_CRITICAL();
}
/*******************************************************************************
* Function Name  : IWDG_Configuration
* Description    : Configures the watch dog.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void IWDG_Configuration(void)
{
	/* IWDG timeout equal to (Reload / (40KHz(LSI) / IWDG_Prescaler)) ms 
     (the timeout may varies due to LSI frequency dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: 40KHz(LSI) / IWDG_Prescaler */
    IWDG_SetPrescaler(IWDG_Prescaler_256);

    /* Set counter reload value to Reload */
    IWDG_SetReload(0x199);

	#if	WDT_TYPE == 1  //内狗
	IWDG_Enable();
	#endif
    /* Reload IWDG counter */
    IWDG_ReloadCounter();
}

/*******************************************************************************
* Function Name   : NVIC_Configuration_EXTI
* Description        : Configures NVIC and Vector Table base location.
* Input                    : None
* Output                 : None
* Return                 : None
*******************************************************************************/
void NVIC_Configuration_EXTI(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;
	/*begin:yangfei modifed 2013-01-07 for change VectTab*/
	#ifdef VECT_TAB_SRAM
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else
	 /* Set the Vector Table base location at 0x08000000 */
   NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	#endif
	/*end:yangfei modifed 2013-01-07*/
   /* Configure the NVIC Preemption Priority Bits */  
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
#ifdef HW_VERSION_1_01
NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;		//通道设置为外部中断
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//中断响应优先级3
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 		//打开中断
NVIC_Init(&NVIC_InitStructure); 						//初始化

NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;		//通道设置为外部中断
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//中断响应优先级3
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 		//打开中断
NVIC_Init(&NVIC_InitStructure); 						//初始化

NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;		//通道设置为外部中断
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		//中断响应优先级3
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 		//打开中断
NVIC_Init(&NVIC_InitStructure); 						//初始化

NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;		//通道设置为外部中断
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//中断响应优先级3
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 		//打开中断
NVIC_Init(&NVIC_InitStructure); 						//初始化

NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;		//通道设置为外部中断
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//中断响应优先级3
NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 		//打开中断
NVIC_Init(&NVIC_InitStructure); 						//初始化

#else
   /* Enable the exti Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;      //通道设置为外部中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	   //中断响应优先级3
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化
   
   /* Enable the exti Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;      //通道设置为外部中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	   //中断响应优先级3
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
#endif   
   NVIC_Init(&NVIC_InitStructure); 						   //初始化
}

/*******************************************************************************
* Function Name  : EXTI_Configuration
* Description    : Configures the different EXTI lines.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 

  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource7);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource8);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9);
#ifdef HW_VERSION_1_01
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3);
	
	/*****************gjt add 140409 *******************/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
	
	EXTI_ClearITPendingBit(EXTI_Line0 | EXTI_Line1 | EXTI_Line3| EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9 | EXTI_Line11 | EXTI_Line12 );
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	//EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_Line =EXTI_Line0|EXTI_Line1|EXTI_Line3| EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9 | EXTI_Line11 | EXTI_Line12 ;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	
	EXTI_Init(&EXTI_InitStructure);
#if 1
	EXTI_ClearITPendingBit(EXTI_Line13);//
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_Line =EXTI_Line13;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	
	EXTI_Init(&EXTI_InitStructure);
#endif

#else
  EXTI_ClearITPendingBit(EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9 | EXTI_Line11 | EXTI_Line12 );
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9 | EXTI_Line11 | EXTI_Line12 ;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  /*****************gjt add 140409 *******************/
  
  EXTI_Init(&EXTI_InitStructure);
 #endif 
 
}

/**********************************************************************************************************
*                                           BSP_Key_Init()
*
* Description : Initialize the board's Key.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
 **********************************************************************************************************/
void BSP_Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);

	/* 配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
		
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIOINTInit();									//初始化按键中断传递的消息邮箱
	
}
/**********************************************************************************************************
*                                           BSP_GPIOPolling_Init()
*
* Description : Initialize the board's Key.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_GPIOPolling_Init().
*
* Note(s)     : none.
 **********************************************************************************************************/
void BSP_GPIOPolling_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	/* 配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
		
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIOPwrInit();
	
}

/**********************************************************************************************************
*                                             BSP_All_ControlLines_Init()
*
* Description : Initialize GPIO for All Control Lines.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_All_ControlLines_Init(void)
{
	GPIO_InitTypeDef               gpio_init;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOG, ENABLE);
	
/************************************* MBUS Channels Channel1,2,3,4,5,6 ***********************************/
	gpio_init.GPIO_Pin 		= GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOA, &gpio_init);
  	
  	gpio_init.GPIO_Pin 		= GPIO_Pin_4 | GPIO_Pin_5; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOC, &gpio_init);
  	
/************************************* LCM Control Lines **************************************************/
	gpio_init.GPIO_Pin 		= GPIO_Pin_7 | GPIO_Pin_8; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOG, &gpio_init);
  	
/************************************* RS485 Control Lines ************************************************/
#ifdef   _MBUS_DETACH_
	gpio_init.GPIO_Pin 		= GPIO_Pin_3; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOD, &gpio_init);

	gpio_init.GPIO_Pin 		= GPIO_Pin_6; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOD, &gpio_init);
#else
	gpio_init.GPIO_Pin 		= GPIO_Pin_8; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOA, &gpio_init);
#endif
/************************************* GPRS Control Lines ************************************************/
	gpio_init.GPIO_Pin 		= GPIO_Pin_2 | GPIO_Pin_3; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOE, &gpio_init);

/*************************************蜂鸣器 ***********************************/
	gpio_init.GPIO_Pin 		= GPIO_Pin_5; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   				//普通推挽输出
  	GPIO_Init(GPIOB, &gpio_init);
  	

  	
}

/**********************************************************************************************************
*                                             BSP_FSMC_IO_Init()
*
* Description : Initialize GPIO for FSMC.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_FSMC_IO_Init(void)
{
	GPIO_InitTypeDef               gpio_init;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF 
    						| RCC_APB2Periph_GPIOG, ENABLE);

	/*							SRAM						*/
	
    /* ---------------------- CFG GPIO -------------------- */
    /* SRAM data lines configuration.                        */
    gpio_init.GPIO_Pin   = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &gpio_init);

    gpio_init.GPIO_Pin   = GPIO_Pin_7  | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 |
                           GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &gpio_init);

    /* SRAM address lines configuration. A0  --- A17           */
    gpio_init.GPIO_Pin   = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_4 |
                           GPIO_Pin_5  | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOF, &gpio_init);

    gpio_init.GPIO_Pin   = GPIO_Pin_0  | GPIO_Pin_1  | GPIO_Pin_2  | GPIO_Pin_3  | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOG, &gpio_init);

    gpio_init.GPIO_Pin   = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOD, &gpio_init);

    /* NOE and NWE configuration.                           */
    gpio_init.GPIO_Pin   = GPIO_Pin_4  | GPIO_Pin_5;
    GPIO_Init(GPIOD, &gpio_init);

    /* NE3 configuration.                                   */
    gpio_init.GPIO_Pin   = GPIO_Pin_10;
    GPIO_Init(GPIOG, &gpio_init);
    
    /* NBL0, NBL1 configuration.                            */
    gpio_init.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOE, &gpio_init);
    
    /*							LCM							*/
    //8位数据线D0 -- D7, A0, NOE, NWE, 均与上面配置相同
    /* 					NE3 configuration.                  */
    gpio_init.GPIO_Pin   = GPIO_Pin_7;
    GPIO_Init(GPIOD, &gpio_init);
    
}

/*******************************************************************************
* Function Name   : NVIC_Configuration_USART
* Description        : Configures NVIC and Vector Table base location.
* Input                    : None
* Output                 : None
* Return                 : None
*******************************************************************************/
void NVIC_Configuration_USART(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;
  
   /*begin:yangfei modifed 2013-01-07 for change VectTab*/
	#ifdef VECT_TAB_SRAM
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else
	 /* Set the Vector Table base location at 0x08000000 */
   NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
	#endif
  
   /* Configure the NVIC Preemption Priority Bits */  
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
   /* Enable the UART4 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;        //通道设置为串口4中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化
   
   /* Enable the UART5 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;        //通道设置为串口5中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化
   
   /* Enable the UART3 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;        //通道设置为串口5中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;	   //中断响应优先级0
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化

   /* Enable the USART2 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;       //通道设置为串口2中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;	   //中断响应优先级1
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化
   
   /* Enable the USART1 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;       //通道设置为串口1中断
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;	   //中断响应优先级1
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		   //打开中断
   NVIC_Init(&NVIC_InitStructure); 						   //初始化
}

/**********************************************************************************************************
*                                             BSP_UART1_Init()
*
* Description : BSP_UART1_Init Configuration
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_USART1_Init(void) 
{
  	GPIO_InitTypeDef gpio_init;
	USART_InitTypeDef uart_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	gpio_init.GPIO_Pin 		= GPIO_Pin_9;
	gpio_init.GPIO_Mode 	= GPIO_Mode_AF_PP;
	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_10;
	gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio_init);

	/* 配置USART1参数
	    - BaudRate = DEBUG_BAUNDRATE baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - DEBUG_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = DEBUG_BAUNDRATE;
	uart_init.USART_WordLength = USART_WordLength_8b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = DEBUG_CHECK;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &uart_init);

    /* 若接收数据寄存器满，则产生中断 */
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	USART1_QueueCreate();
	
	/* 第5步：使能 USART1， 配置完毕 */
	USART_Cmd(USART1, ENABLE);
    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    //USART_ClearFlag(USART1, USART_FLAG_TC);     // 清标志
    
    
}

/**********************************************************************************************************
*                                             BSP_UART2_Init()
*
* Description : BSP_UART2_Init Configuration
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_USART2_Init(void) 
{
  	GPIO_InitTypeDef gpio_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);

	gpio_init.GPIO_Pin 		= GPIO_Pin_2;
	gpio_init.GPIO_Mode 	= GPIO_Mode_AF_PP;
	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_3;
	gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio_init);

	if(MBUS_DEFAULT_PARA == WAN_HUA)
		{
			METER_ComSet2();
		}
	else
		{
			METER_ComSet1();
		}

    /* 若接收数据寄存器满，则产生中断 */
    //USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	/* 第5步：使能 USART2， 配置完毕 */
	USART_Cmd(USART2, ENABLE);
	USART2_QueueCreate();

    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    USART_ClearFlag(USART2, USART_FLAG_TC);     // 清标志
}

void METER_ComSet1(void) 
{
	USART_InitTypeDef uart_init;

	/* 配置USART2参数
	    - BaudRate = 2400 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - MBUS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = 2400;
	uart_init.USART_WordLength = USART_WordLength_9b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = USART_Parity_Even;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &uart_init);
}

void METER_ComSet5(void) 
{
  	USART_InitTypeDef uart_init;
	/* 配置USART2参数
	    - BaudRate = 2400 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - MBUS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = 2400;
	uart_init.USART_WordLength = USART_WordLength_8b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = USART_Parity_No;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &uart_init);
}

void METER_ComSet2(void) 
{
  	USART_InitTypeDef uart_init;

	/* 配置USART2参数
	    - BaudRate = 1200 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - MBUS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = 1200;
	uart_init.USART_WordLength = USART_WordLength_8b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = USART_Parity_No;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &uart_init);
}

/* 
	    - BaudRate = 1200 baud  偶校验 
*/
void METER_ComSet4(void) 
{
  	USART_InitTypeDef uart_init;

	uart_init.USART_BaudRate = 1200;
	uart_init.USART_WordLength = USART_WordLength_9b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = USART_Parity_Even;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &uart_init);
}
/*begin:yangfei added 2013-03-25 for support 485 meter*/
void METER_ComSet3(void) 
{
  	USART_InitTypeDef uart_init;
	/* 配置USART2参数
	    - BaudRate = 2400 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - MBUS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = 2400;
	uart_init.USART_WordLength = USART_WordLength_9b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = USART_Parity_Even;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &uart_init);
}

void METER_ComSet6(void) 
{
  	USART_InitTypeDef uart_init;
	/* 配置USART2参数
	    - BaudRate = 2400 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - MBUS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = 2400;	///测试联强表使用1200，原为2400
	uart_init.USART_WordLength = USART_WordLength_9b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = USART_Parity_Even;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &uart_init);

}

void METER_ComSet7(void) 
{
  	USART_InitTypeDef uart_init;
	/* 配置USART2参数
	    - BaudRate = 2400 baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - MBUS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/

	uart_init.USART_BaudRate = 2400;
	uart_init.USART_WordLength = USART_WordLength_9b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = USART_Parity_Even;  //	
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &uart_init);

}

/*end:yangfei added 2013-03-25 for support 485 meter*/
/**********************************************************************************************************
*                                             BSP_UART3_Init()
*
* Description : BSP_UART3_Init Configuration
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_USART3_Init(void) 
{
  	GPIO_InitTypeDef gpio_init;
	USART_InitTypeDef uart_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	gpio_init.GPIO_Pin 		= GPIO_Pin_10;
	gpio_init.GPIO_Mode 	= GPIO_Mode_AF_PP;
	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_11;
	gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &gpio_init);

	/* 配置USART3参数
	    - BaudRate = GPRS_BAUNDRATE baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - GPRS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = GPRS_BAUNDRATE;
	uart_init.USART_WordLength = USART_WordLength_8b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = GPRS_CHECK;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &uart_init);

    /* 若接收数据寄存器满，则产生中断 */
    //USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	/* 第5步：使能 USART3， 配置完毕 */
	USART_Cmd(USART3, ENABLE);
	USART3_QueueCreate();

    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    USART_ClearFlag(USART3, USART_FLAG_TC);     // 清标志
}
/*
设置GPRS波特率
*/
void USART3_BAUNDRATE_Init(uint32  BAUNDRATE) 
{
  USART_InitTypeDef uart_init;
  /* 配置USART3参数
	    - BaudRate = GPRS_BAUNDRATE baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - GPRS_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = BAUNDRATE;
	uart_init.USART_WordLength = USART_WordLength_8b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = GPRS_CHECK;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &uart_init);
}
/**********************************************************************************************************
*                                             BSP_UART4_Init()
*
* Description : BSP_UART4_Init Configuration
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_UART4_Init(void) 
{
  	GPIO_InitTypeDef gpio_init;
	USART_InitTypeDef uart_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	gpio_init.GPIO_Pin 		= GPIO_Pin_10;
	gpio_init.GPIO_Mode 	= GPIO_Mode_AF_PP;
	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_11;
	gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &gpio_init);

	/* 配置USART4参数
	    - BaudRate = ZIGBEE_BAUNDRATE baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - ZIGBEE_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled*/
	USART_DeInit(UART4);
	uart_init.USART_BaudRate = ZIGBEE_BAUNDRATE;
	uart_init.USART_WordLength = USART_WordLength_8b;
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = ZIGBEE_CHECK;
	//uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &uart_init);

    /*若接收数据寄存器满，则产生中断*/ 
    //USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

	/* 第5步：使能 USART4， 配置完毕*/ 
	USART_Cmd(UART4, ENABLE);
	UART4_QueueCreate();

    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    //USART_ClearFlag(UART4, USART_FLAG_TC);     // 清标志
    //uart4s_init(36, 2400);   
}

/**********************************************************************************************************
*                                             BSP_UART5_Init()
*
* Description : BSP_UART5_Init Configuration  support:485
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_UART5_Init(void) 
{
  	GPIO_InitTypeDef gpio_init;
	USART_InitTypeDef uart_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	gpio_init.GPIO_Pin 		= GPIO_Pin_12;
	gpio_init.GPIO_Mode 	= GPIO_Mode_AF_PP;
	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio_init);

	gpio_init.GPIO_Pin = GPIO_Pin_2;
	gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &gpio_init);

	/* 配置USART5参数
	    - BaudRate = RS485_BAUNDRATE baud
	    - Word Length = 8 Bits
	    - One Stop Bit
	    - RS485_CHECK
	    - Hardware flow control disabled (RTS and CTS signals)
	    - Receive and transmit enabled
	*/
	uart_init.USART_BaudRate = RS485_BAUNDRATE;
	#ifdef   _MBUS_DETACH_
	uart_init.USART_WordLength = USART_WordLength_9b;
	#else
	uart_init.USART_WordLength = USART_WordLength_8b;
	#endif
	uart_init.USART_StopBits = USART_StopBits_1;
	uart_init.USART_Parity = RS485_CHECK;
	uart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &uart_init);

    /* 若接收数据寄存器满，则产生中断 */
    //USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

	/* 第5步：使能 USART5， 配置完毕 */
	USART_Cmd(UART5, ENABLE);
	UART5_QueueCreate();

    /* 如下语句解决第1个字节无法正确发送出去的问题 */
    //USART_ClearFlag(UART5, USART_FLAG_TC);     // 清标志
}


/**********************************************************************************************************
*                                             BSP_I2C_Init()
*
* Description : I2C Configuration
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none. 软件模拟
**********************************************************************************************************/
#ifdef   _MBUS_DETACH_
void BSP_I2C_Init(void) 
{
/* Configure I2C2 pins: PB6->SCL and PB7->SDA */
  /*GPIO_InitTypeDef  GPIO_InitStructure; 
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  		 //开漏输出
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  */
  GPIO_InitTypeDef GPIO_InitStructure;
  I2C_InitTypeDef i2cInitStructure;
  NVIC_InitTypeDef nvicInitStructure;
  #if 1
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	

	
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
 nvicInitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
 nvicInitStructure.NVIC_IRQChannelPreemptionPriority = 0;
 nvicInitStructure.NVIC_IRQChannelSubPriority = 3;
 nvicInitStructure.NVIC_IRQChannelCmd = ENABLE;
 NVIC_Init(&nvicInitStructure);


GPIO_PinRemapConfig(GPIO_Remap_I2C1,ENABLE);
 GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;  		 //开漏输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  I2C_Cmd(I2C1, ENABLE);
  
  i2cInitStructure.I2C_Mode = I2C_Mode_I2C;
  i2cInitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  i2cInitStructure.I2C_Ack = I2C_Ack_Enable;
  i2cInitStructure.I2C_OwnAddress1 = 0x32;
  i2cInitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  i2cInitStructure.I2C_ClockSpeed = ClockSpeed;
 
  I2C_Init(I2C1, &i2cInitStructure);
	
  I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, ENABLE);
#endif
}
#else
void BSP_I2C_Init(void) 
{
  	GPIO_InitTypeDef  GPIO_InitStructure; 

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //允许GPIOB时钟

  /* Configure I2C2 pins: PB6->SCL and PB7->SDA */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  		 //开漏输出
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
}
#endif
/**********************************************************************************************************
*                                             BSP_SPISpeed()
*
* Description : SD Card SPI Speed select
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_SPISpeed(uint8_t b_high)
{
  	SPI_InitTypeDef SPI_InitStructure;
  	
  	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  	SPI_InitStructure.SPI_CRCPolynomial = 7;
  	
  	/* Speed select */
  	if(b_high == 0)
  	{
		 SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  	}
  	else
  	{
		 SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
  	}
  	
  	SPI_Init(SPI2, &SPI_InitStructure);
}

/**********************************************************************************************************
*                                             BSP_SD_SPI_Init()
*
* Description : SD Card SPI Configuration
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

void BSP_SD_SPI_Init(void) 
{
  	
  	GPIO_InitTypeDef  gpio_init;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	/*begin:yangfei added 2012.11.29*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	 //使能DMA1时钟  	
	/*end:yangfei added 2012.11.29*/
  	gpio_init.GPIO_Pin 		= GPIO_Pin_12; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_Out_PP;   						//普通推挽输出
  	GPIO_Init(GPIOB, &gpio_init);
  	
  	gpio_init.GPIO_Pin 		= GPIO_Pin_13|GPIO_Pin_15; 
  	gpio_init.GPIO_Speed 	= GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode 	= GPIO_Mode_AF_PP;   						//复用推挽输出
  	GPIO_Init(GPIOB, &gpio_init); 
  	
  	gpio_init.GPIO_Pin = GPIO_Pin_14; 
  	gpio_init.GPIO_Speed = GPIO_Speed_50MHz; 
  	gpio_init.GPIO_Mode = GPIO_Mode_IPU;   								//上拉输入
  	GPIO_Init(GPIOB, &gpio_init);
  	
	
  	MSD_SPIHighSpeed(0);
  		
  	SPI_Cmd(SPI2,ENABLE);  
}

/**********************************************************************************************************
*                                             BSP_LCM_FSMC_Init()
*
* Description : Initialize FSMC for LCM.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_LCM_Init().
*
* Note(s)     : FSMC_Bank1_NORSRAM1    StartAddr 0x60000000
**********************************************************************************************************/
static FSMC_NORSRAMInitTypeDef  Lcm_init;

void  BSP_LCM_FSMC_Init (void)
{
  	FSMC_NORSRAMTimingInitTypeDef  p;
  	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
  	/*-- FSMC Configuration ------------------------------------------------------*/
  	/*----------------------- SRAM Bank 1 ----------------------------------------*/
  	/* FSMC_Bank1_NORSRAM1 configuration */
  	p.FSMC_AddressSetupTime = 1;
  	p.FSMC_AddressHoldTime = 0;
  	p.FSMC_DataSetupTime = 2;
  	p.FSMC_BusTurnAroundDuration = 0;
  	p.FSMC_CLKDivision = 0;
  	p.FSMC_DataLatency = 0;
  	p.FSMC_AccessMode = FSMC_AccessMode_B;					//???这是个啥意思
  	
  	/* Color LCD configuration ------------------------------------
  	   LCD configured as follow:
  	      - Data/Address MUX = Disable
  	      - Memory Type = SRAM
  	      - Data Width = 8bit
  	      - Write Operation = Enable
  	      - Extended Mode = Enable
  	      - Asynchronous Wait = Disable */
  	Lcm_init.FSMC_Bank = FSMC_Bank1_NORSRAM1;
  	Lcm_init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  	Lcm_init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  	Lcm_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;
  	Lcm_init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  	Lcm_init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  	Lcm_init.FSMC_WrapMode = FSMC_WrapMode_Disable;
  	Lcm_init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  	Lcm_init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  	Lcm_init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  	Lcm_init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  	Lcm_init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  	Lcm_init.FSMC_ReadWriteTimingStruct = &p;
  	Lcm_init.FSMC_WriteTimingStruct = &p;
  	
  	FSMC_NORSRAMInit(&Lcm_init);
  	
  	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

/**********************************************************************************************************
*                                             BSP_LCM_Init()
*
* Description : Initialize FSMC for LCM.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : FSMC_Bank1_NORSRAM3    StartAddr 0x68000000
**********************************************************************************************************/

void  BSP_LCM_Init (void)
{
    BSP_LCM_FSMC_Init();
    
}

/**********************************************************************************************************
*                                             BSP_SRAM_Init()
*
* Description : Initialize FSMC for Sram.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : FSMC_Bank1_NORSRAM3    StartAddr 0x68000000
**********************************************************************************************************/
static FSMC_NORSRAMInitTypeDef        sram_init;

void  BSP_SRAM_Init (void)
{
    FSMC_NORSRAMTimingInitTypeDef  p;
    
    //RCC->AHBENR = 0x00000114;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
    
    /* ---------------------- CFG FSMC --------------------*/ 
    p.FSMC_AddressSetupTime 		= 3;
  	p.FSMC_AddressHoldTime 			= 3;
  	p.FSMC_DataSetupTime 			= 3;
  	p.FSMC_BusTurnAroundDuration 	= 0;
  	p.FSMC_CLKDivision 				= 1;
  	p.FSMC_DataLatency 				= 0;
  	p.FSMC_AccessMode 				= FSMC_AccessMode_A;	
  	
  	sram_init.FSMC_Bank = FSMC_Bank1_NORSRAM3;
  	sram_init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  	sram_init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
  	sram_init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  	sram_init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  	sram_init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  	sram_init.FSMC_WrapMode = FSMC_WrapMode_Disable;
  	sram_init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  	sram_init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  	sram_init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  	sram_init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
  	sram_init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  	sram_init.FSMC_ReadWriteTimingStruct = &p;
  	sram_init.FSMC_WriteTimingStruct = &p;
  	
  	FSMC_NORSRAMInit(&sram_init);  
  	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);
}

/**********************************************************************************************************
*                                             BSP_Gprs_CtrInit_Xmz()
*
* Description : Initialize Power On for GPRS.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : 
**********************************************************************************************************/
void BSP_Gprs_CtrInit_Xmz(void)
{
//	GprsRestart_xmz();
	GPRS_POW_OFF();
	delay(0x0000ffff);

	GPRS_POW_ON();
	delay(0x0000ffff);
	GPRS_RESET_DISABLE();

	delay(0x002fffff);
    GPRS_RESET_ENABLE();
    delay(0x002fffff);
    GPRS_RESET_DISABLE();
    delay(0x00000fff);
}














/**********************************************************************************************************
*********************************************************************************************************
*                                           OS PROBE FUNCTIONS
*********************************************************************************************************
**********************************************************************************************************/

/**********************************************************************************************************
*                                           OSProbe_TmrInit()
*
* Description : Select & initialize a timer for use with the uC/Probe Plug-In for uC/OS-II.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : OSProbe_Init().
*
* Note(s)     : none.
**********************************************************************************************************/

#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == 1))
void  OSProbe_TmrInit (void)
{
    TIM_TimeBaseInitTypeDef  tim_init;


    tim_init.TIM_Period        = 0xFFFF;
    tim_init.TIM_Prescaler     = 256;
    tim_init.TIM_ClockDivision = TIM_CKD_DIV4;
    tim_init.TIM_CounterMode   = TIM_CounterMode_Up;

#if (OS_PROBE_TIMER_SEL == 2)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseInit(TIM2, &tim_init);
    TIM_SetCounter(TIM2, 0);
    TIM_PrescalerConfig(TIM2, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM2, ENABLE);
#elif (OS_PROBE_TIMER_SEL == 3)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInit(TIM3, &tim_init);
    TIM_SetCounter(TIM3, 0);
    TIM_PrescalerConfig(TIM3, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM3, ENABLE);
#elif (OS_PROBE_TIMER_SEL == 4)
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInit(TIM4, &tim_init);
    TIM_SetCounter(TIM4, 0);
    TIM_PrescalerConfig(TIM4, 256, TIM_PSCReloadMode_Immediate);
    TIM_Cmd(TIM4, ENABLE);
#endif
}
#endif

/**********************************************************************************************************
*                                            OSProbe_TmrRd()
*
* Description : Read the current counts of a 16-bit free running timer.
*
* Argument(s) : none.
*
* Return(s)   : The 16-bit counts of the timer in a 32-bit variable.
*
* Caller(s)   : OSProbe_TimeGetCycles().
*
* Note(s)     : none.
**********************************************************************************************************/

#if ((APP_OS_PROBE_EN   == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN == 1))
CPU_INT32U  OSProbe_TmrRd (void)
{
#if (OS_PROBE_TIMER_SEL == 2)

    return ((CPU_INT32U)TIM_GetCounter(TIM2));

#elif (OS_PROBE_TIMER_SEL == 3)

    return ((CPU_INT32U)TIM_GetCounter(TIM3));

#elif (OS_PROBE_TIMER_SEL == 4)

    return ((CPU_INT32U)TIM_GetCounter(TIM4));

#endif
}
#endif
