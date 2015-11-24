
/*****************************************************************
*
*
*
******************************************************************/
/* Includes ------------------------------------------------------------------*/
#include <includes.h>
#ifdef   _MBUS_DETACH_
#define I2C_M_RD		(1 << 0)	/* read data, from slave to master */
#define I2C_M_SIM           (1 << 1)
#define I2C_M_GSTOP       (1 << 2)

OS_EVENT *I2cMsgRetSem;
OS_EVENT *I2cMsgSem;
 struct i2c_msg_t
{
   uint16 addr;
   uint16 subAddr;
   uint16 flag;
   uint16 len;
   uint16 msg_ptr;
   uint8  buf[255]; 
   uint16 result;
 };

struct i2c_msg_t i2c_msg;

void I2C_delay(void)		  //软件延时（非精确）
{	
   uint16_t i=20000;     
   while(i) 
   { 
     i--; 
   } 
}
/*******************************************************************************
* Function Name  : I2C_WriteByte
* Description    : 写一字节数据
* Input          : - SendByte: 待写入数据
*           	   - WriteAddress: 待写入地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功写入,=0失败
*******************************************************************************/           
uint8 I2C_Write(uint8 *pSendByte, uint8 len, uint8 DeviceAddress)
{	
 uint8 err;
  OSSemPend(I2cMsgSem, 0, &err);
  i2c_msg.addr = (DeviceAddress & 0xFE);
  i2c_msg.flag = I2C_M_GSTOP;
  i2c_msg.msg_ptr = 0;
  i2c_msg.len = len;
  i2c_msg.result = 0;
  memcpy(i2c_msg.buf,pSendByte,len);
  I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
  I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
  I2C_AcknowledgeConfig(I2C1, ENABLE);
   while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
  I2C_GenerateSTART(I2C1, ENABLE);
  OSSemPend(I2cMsgRetSem, 0, &err);
  OSSemPost(I2cMsgSem);
   I2C_delay();			 
   return i2c_msg.result;
}									 

/*******************************************************************************
* Function Name  : I2C_ReadByte
* Description    : 读取一串数据
* Input          : - pBuffer: 存放读出数据
*           	   - length: 待读出长度
*                  - ReadAddress: 待读出地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功读入,=0失败
*******************************************************************************/          
uint8 I2C_ReadByte(uint8* pBuffer,   uint8 length,   uint8 ReadAddress,  uint8 DeviceAddress)
{		

 uint8 err;
  OSSemPend(I2cMsgSem, 0, &err);
  i2c_msg.addr = (DeviceAddress & 0xFE);
  i2c_msg.flag = I2C_M_SIM;
  i2c_msg.msg_ptr = 0;
  i2c_msg.len = length;
   i2c_msg.buf[0] = ReadAddress & 0xF0;//yaotong 20131105
   	I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
	 I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
   I2C_AcknowledgeConfig(I2C1, ENABLE);	
   while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
   I2C_GenerateSTART(I2C1, ENABLE);  
   OSSemPend(I2cMsgRetSem, 0, &err);	 
   OSSemPost(I2cMsgSem);
   memcpy(pBuffer,i2c_msg.buf+1,length - 1);
   
   
   return i2c_msg.result;
}

/**
  * @brief  This function handles I2C1 Event interrupt request.
  * @param  None
  * @retval : None
  */
void I2C1_EV_IRQHandler(void)
{
  u32 u32Event;
  u32Event = I2C_GetLastEvent(I2C1);
  switch (u32Event)
  {
    case I2C_EVENT_MASTER_MODE_SELECT:                 /* EV5 */

	if(!(i2c_msg.flag & I2C_M_RD))
      {
        /* Master Transmitter ----------------------------------------------*/
        I2C_Send7bitAddress(I2C1, i2c_msg.addr, I2C_Direction_Transmitter);
      }
      else
      {
        /* Master Receiver -------------------------------------------------*/  
        I2C_Send7bitAddress(I2C1, i2c_msg.addr, I2C_Direction_Receiver); 
      }
      break;
        
    /* Master Transmitter --------------------------------------------------*/
    case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:  
      
 	   I2C_SendData(I2C1, i2c_msg.buf[i2c_msg.msg_ptr++]); 
	   if(i2c_msg.flag & I2C_M_SIM)
	   	I2C_ITConfig(I2C1, I2C_IT_BUF, DISABLE);
	   if(i2c_msg.len == 1)
	   	I2C_ITConfig(I2C1, I2C_IT_BUF, DISABLE);
      break;

    /* Test on I2C1 EV8 and clear it */
  case I2C_EVENT_MASTER_BYTE_TRANSMITTING:  /* Without BTF, EV8 */     
      if(i2c_msg.msg_ptr < i2c_msg.len)
      {
        /* Transmit I2C1 data */
        I2C_SendData(I2C1, i2c_msg.buf[i2c_msg.msg_ptr++]);
      
      }
      else
      {
        I2C_ITConfig(I2C1, I2C_IT_BUF, DISABLE);

      }        
      break;

    case I2C_EVENT_MASTER_BYTE_TRANSMITTED: /* With BTF EV8-2 */
	
	 if(i2c_msg.flag & I2C_M_GSTOP)
	 {
	   
        I2C_GenerateSTOP(I2C1, ENABLE);	
	    i2c_msg.result = 1;
	    OSSemPost(I2cMsgRetSem);
	 }
	 else
	 {
	    i2c_msg.flag = I2C_M_RD;	   
	    I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
	    I2C_GenerateSTART(I2C1, ENABLE);
	 }	
      break;

    /* Master Receiver -------------------------------------------------------*/
    case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
 	 if((i2c_msg.msg_ptr + 1) == (i2c_msg.len ))
     {
       /* Disable I2C1 acknowledgement */
       I2C_AcknowledgeConfig(I2C1, DISABLE);
	    I2C_GenerateSTOP(I2C1, ENABLE); //从模式
     }
      break;

   /* Test on I2C1 EV7 and clear it */
   case I2C_EVENT_MASTER_BYTE_RECEIVED: 	
     i2c_msg.buf[i2c_msg.msg_ptr++] = I2C_ReceiveData (I2C1); 	   
	  if((i2c_msg.msg_ptr + 1) == (i2c_msg.len ))
     {
       /* Disable I2C1 acknowledgement */
       I2C_AcknowledgeConfig(I2C1, DISABLE);
	    I2C_GenerateSTOP(I2C1, ENABLE); //从模式
     }
      if((i2c_msg.msg_ptr ) == (i2c_msg.len ))
     {
     
	   i2c_msg.result = 1;
	   OSSemPost(I2cMsgRetSem);
     }
     /* Disable ACK and send I2C1 STOP condition before receiving the last data */	  
	 
     break;
   case 0x40:
   	  i2c_msg.buf[i2c_msg.msg_ptr] = I2C_ReceiveData (I2C1); 
     if((i2c_msg.msg_ptr +1) >= (i2c_msg.len ))
     {
       /* Disable I2C1 acknowledgement */
		 i2c_msg.result = 1;
	   OSSemPost(I2cMsgRetSem);
     } 
	 break;
    default:
	I2C_ReadRegister(I2C1, I2C_Register_SR1);
	I2C_ReadRegister(I2C1, I2C_Register_SR2);
      break;
  }
}
#else
/* Private define ------------------------------------------------------------*/
#define SCL_H         GPIO_SetBits(GPIOB , GPIO_Pin_6)
#define SCL_L         GPIO_ResetBits(GPIOB , GPIO_Pin_6)
   
#define SDA_H         GPIO_SetBits(GPIOB , GPIO_Pin_7)
#define SDA_L         GPIO_ResetBits(GPIOB , GPIO_Pin_7)

#define SDA_read      GPIO_ReadInputDataBit(GPIOB , GPIO_Pin_7)

//使用SysTick的普通计数模式对延迟进行管理
//包括delay_us,delay_ms 
//static uint8  fac_us = 0;  //us延时倍乘数
//static uint16 fac_ms = 0;  //ms延时倍乘数

/*
//初始化延迟函数
void delay_init(uint8 SYSCLK)
{
 SysTick->CTRL &= 0xfffffffb; //选择内部时钟 HCLK/8
 fac_us = SYSCLK / 8;     
 fac_ms = (uint16)fac_us * 1000;
} 
          
//延时Nms
//注意Nms的范围
//Nms<=0xffffff*8/SYSCLK
//对72M条件下,Nms<=1864
void delay_ms(uint16 nms)
{   
  SysTick->LOAD = (u32)nms * fac_ms;   //时间加载 
  SysTick->CTRL |= 0x01;               //开始倒数   
  while(!(SysTick->CTRL & (1<<16)));   //等待时间到达
  SysTick->CTRL &= 0XFFFFFFFE;         //关闭计数器
  SysTick->VAL = 0X00000000;           //清空计数器    
} 
 
//延时us          
void delay_us(u32 Nus)
{ 
  SysTick->LOAD = Nus*fac_us;          //时间加载     
  SysTick->CTRL |= 0x01;               //开始倒数   
  while(!(SysTick->CTRL & (1<<16)));   //等待时间到达
  SysTick->CTRL = 0X00000000;          //关闭计数器
  SysTick->VAL = 0X00000000;           //清空计数器    
}*/  

void I2C_delay(void)		  //软件延时（非精确）
{	
   uint8_t i=200;     
   while(i) 
   { 
     i--; 
   } 
}

uint8 I2C_Start(void)	 //I2C开始位
{
	SDA_H;
	SCL_H;
	I2C_delay();
	if(!SDA_read)
	  return 0;	                    //SDA线为低电平则总线忙,退出
	SDA_L;
	I2C_delay();
	if(SDA_read) 
	  return 0;	                    //SDA线为高电平则总线出错,退出
	SDA_L;							//SCL为高电平时，SDA的下降沿表示停止位
	I2C_delay();
	return 1;
}

void I2C_Stop(void)			   //I2C停止位
{
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SDA_H;					   //SCL为高电平时，SDA的上升沿表示停止位
	I2C_delay();
}

static void I2C_Ack(void)		//I2C响应位
{	
	SCL_L;
	I2C_delay();
	SDA_L;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

static void I2C_NoAck(void)		//I2C非响应位
{	
	SCL_L;
	I2C_delay();
	SDA_H;
	I2C_delay();
	SCL_H;
	I2C_delay();
	SCL_L;
	I2C_delay();
}

uint8 I2C_WaitAck(void) 	  //I2C等待应答位
{
	SCL_L;
	I2C_delay();
	SDA_H;			
	I2C_delay();
	SCL_H;
	I2C_delay();
	if(SDA_read)
	{
      SCL_L;
      return 0;
	}
	SCL_L;
	return 1;
}

 /*******************************************************************************
* Function Name  : I2C_SendByte
* Description    : 数据从高位到低位
* Input          : - SendByte: 发送的数据
* Output         : None
* Return         : None
*******************************************************************************/
void I2C_SendByte(uint8 SendByte) 
{
    uint8 i;

    for(i = 0;i < 8; i++) 
    {
      SCL_L;
      I2C_delay();
      if(SendByte & 0x80)
        SDA_H;  			  //在SCL为低电平时，允许SDA数据改变
      else 
        SDA_L;   
      SendByte <<= 1;
      I2C_delay();
      SCL_H;
      I2C_delay();
    }
    SCL_L;
}

/*******************************************************************************
* Function Name  : I2C_ReceiveByte
* Description    : 数据从高位到低位
* Input          : None
* Output         : None
* Return         : I2C总线返回的数据
*******************************************************************************/
uint8 I2C_ReceiveByte(void)  
{ 
    uint8 i,ReceiveByte = 0;

    SDA_H;				
    for(i = 0;i < 8; i++) 
    {
      ReceiveByte <<= 1;      
      SCL_L;
      I2C_delay();
	  SCL_H;
      I2C_delay();	
      if(SDA_read)				   //在SCL为高电平时，SDA上的数据保持不变，可以读回来
      {
        ReceiveByte |= 0x01;
      }
    }
    SCL_L;
    return ReceiveByte;
}

/*******************************************************************************
* Function Name  : I2C_WriteByte
* Description    : 写一字节数据
* Input          : - SendByte: 待写入数据
*           	   - WriteAddress: 待写入地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功写入,=0失败
*******************************************************************************/           
uint8 I2C_Write(uint8 *pSendByte, uint8 len, uint8 DeviceAddress)
{	
	
    if(!I2C_Start())
	  return 0;

    I2C_SendByte(DeviceAddress & 0xFE); //器件地址 + W

    if(!I2C_WaitAck())
	{
	  I2C_Stop(); 
	  return 0;
	}
	
	while(len--)
	{
		I2C_SendByte(*pSendByte++);
    	I2C_WaitAck();
	}
    
	   
    I2C_Stop(); 

    return 1;
}									 

/*******************************************************************************
* Function Name  : I2C_ReadByte
* Description    : 读取一串数据
* Input          : - pBuffer: 存放读出数据
*           	   - length: 待读出长度
*                  - ReadAddress: 待读出地址
*                  - DeviceAddress: 器件类型
* Output         : None
* Return         : 返回为:=1成功读入,=0失败
*******************************************************************************/          
uint8 I2C_ReadByte(uint8* pBuffer,   uint8 length,   uint8 ReadAddress,  uint8 DeviceAddress)
{		
	//I2C_WriteByte(ReadAddress, DeviceAddress);
	if(!I2C_Start())
	  return 0;

    I2C_SendByte(DeviceAddress & 0xFE); //器件地址 + W
	I2C_WaitAck();

	I2C_SendByte(ReadAddress | 0x04); 	//快捷读取
	I2C_WaitAck();
	length--;

    while(length)
    {
      *pBuffer = I2C_ReceiveByte();
      if(length == 1)
	    I2C_NoAck();
      else 
	    I2C_Ack(); 
      pBuffer++;
      length--;
    }

    I2C_Stop();

    return 1;
}
#endif	
