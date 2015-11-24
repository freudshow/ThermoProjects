/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               SPI_MSD_Driver.c
** Descriptions:            The SPI SD Card application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              Gou Jiangtao
** Created date:            2012-08-24
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <includes.h>
#include "SPI_MSD_Driver.h"
#include <stm32f10x_dma.h>


/* Data response error */
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF
/* MSD reponses and error flags */
#define MSD_RESPONSE_NO_ERROR      0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF


/*begin:yangfei added*/
#define MSD_CS_LOW()  _card_enable()
#define MSD_CS_HIGH()  _card_disable()
#define MSD_START_DATA_SINGLE_BLOCK_READ 0xFE  /* Data token start byte, Start Single Block Read */
#define MSD_START_DATA_MULTIPLE_BLOCK_READ  0xFE  /* Data token start byte, Start Multiple Block Read */
#define MSD_START_DATA_SINGLE_BLOCK_WRITE 0xFE  /* Data token start byte, Start Single Block Write */
#define MSD_START_DATA_MULTIPLE_BLOCK_WRITE 0xFD  /* Data token start byte, Start Multiple Block Write */
#define MSD_STOP_DATA_MULTIPLE_BLOCK_WRITE 0xFD  /* Data toke stop byte, Stop Multiple Block Write */
#define MSD_ReadByte()  _spi_read_write(DUMMY_BYTE)
#define MSD_WriteByte  _spi_read_write
/*end:yangfei added*/

/* Private variables ---------------------------------------------------------*/
MSD_CARDINFO CardInfo;
uint8 gSdRWBuf[MSD_BLOCKSIZE];
OS_EVENT *FlashMutex;
extern OS_FLAG_GRP *Sem_SD_DMA; //声明了一个事件
uint16 gSD_Err = 0x00;

uint8 gu8SDStatus = 0;  //指示SD卡异常状态。

void MSD_SendCmd(u8 Cmd, u32 Arg, u8 Crc);

/*******************************************************************************
* Function Name  : MSD_GetResponse
* Description    : Returns the MSD response.
* Input          : None
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_GetResponse(u8 Response)
{
  u32 Count = 0xFFF;

  /* Check if response is got or a timeout is happen */
  while ((_spi_read_write(DUMMY_BYTE) != Response) && Count)
  {
    Count--;
  }

  if (Count == 0)
  {
    /* After time out */
    return MSD_RESPONSE_FAILURE;
  }
  else
  {
    /* Right response got */
    return MSD_RESPONSE_NO_ERROR;
  }
}
/*******************************************************************************
* Function Name  : _spi_read_write
* Description    : None
* Input          : - data:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
__inline int _spi_read_write(uint8_t data)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI2 peripheral */
  SPI_I2S_SendData(SPI2, data);

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI2 bus */
  return SPI_I2S_ReceiveData(SPI2);
}
														
/*******************************************************************************
* Function Name  : MSD_SPIHighSpeed
* Description    : SD Card Speed Set
* Input          : - b_high: 1 = 18MHz, 0 = 281.25Hz
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void MSD_SPIHighSpeed(uint8_t b_high)
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
	 SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
  }
  /*begin:201303141504 yangfei added for SD high speed*/
  else if(b_high == 2)
   {
      SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
   }
  /*end:201303141504 yangfei added for SD high speed*/
  else
  {
	 SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
  }

  SPI_Init(SPI2, &SPI_InitStructure);
}

/*******************************************************************************
* Function Name  : MSD_Init
* Description    : SD Card initializtion
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_Init(void)
{
	uint8_t r1;	
	uint8_t buff[6] = {0};
	uint16_t retry; 
	uint16_t retry_time = 5; 
//	uint32 lu32SDStatus = 0;
//	uint8 err;


	gu8SDStatus = 0;
	
	/* Check , if no card insert */
    //if( _card_insert() )
	//{ 
	  /* FATFS error flag */
      //return -1;
	//}

	/* Power on and delay some times */	
	for(retry=0; retry<0x100; retry++)
	{
		_card_power_on();
	}	

	/* Satrt send 74 clocks at least */
	for(retry=0; retry<10; retry++)
	{
		_spi_read_write(DUMMY_BYTE);
	}	
	
	/* Start send CMD0 till return 0x01 means in IDLE state */
	/*begin:yangfei modified 2013-09-28 for SD卡重试次数太多，等待时间太长，SD卡正常的时候只需1次就能成功*/
       #if 0
	for(retry=0; retry<0xFFF; retry++)
	#else
	for(retry=0; retry<retry_time; retry++)
	#endif
	{
		r1 = _send_command(CMD0, 0, 0x95);
		if(r1 == 0x01)
		{
			retry = 0;
			break;
		}
	}
	/* Timeout return */
	#if 0
	if(retry == 0xFFF)
	#else
	if(retry == retry_time)
	#endif
	{
		gu8SDStatus = 1;
		return 1;
	}
	/*end:yangfei modified 2013-09-28 for SD卡重试次数太多，等待时间太长，SD卡正常的时候只需1次就能成功*/
	
	/* Get the card type, version */
	r1 = _send_command_hold(CMD8, 0x1AA, 0x87);
	/* r1=0x05 -> V1.0 */
	if(r1 == 0x05)
	{
	  CardInfo.CardType = CARDTYPE_SDV1;

	  /* End of CMD8, chip disable and dummy byte */
	  _card_disable();
	  _spi_read_write(DUMMY_BYTE);
		
	  /* SD1.0/MMC start initialize */
	  /* Send CMD55+ACMD41, No-response is a MMC card, otherwise is a SD1.0 card */
	  for(retry=0; retry<0xFFF; retry++)
	  {
	     r1 = _send_command(CMD55, 0, 0);			/* should be return 0x01 */
		 if(r1 != 0x01)
		 {
			return r1;
		 }

		 r1 = _send_command(ACMD41, 0, 0);			/* should be return 0x00 */
		 if(r1 == 0x00)
		 {
			retry = 0;
			break;
		 }
	  }

	  /* MMC card initialize start */
	  if(retry == 0xFFF)
	  {
		 for(retry=0; retry<0xFFF; retry++)
	     {
			 r1 = _send_command(CMD1, 0, 0);		/* should be return 0x00 */
			 if(r1 == 0x00)
			 {
				retry = 0;
				break;
			 }
		 }

		 /* Timeout return */
		 if(retry == 0xFFF)
		 {
			gu8SDStatus = 2;
			return 2;
		 }	
			
		CardInfo.CardType = CARDTYPE_MMC;					
	  }		
		
	  /* Set spi speed high */
	  MSD_SPIHighSpeed(1);		
		
	  /* CRC disable */
	  r1 = _send_command(CMD59, 0, 0x01);
	  if(r1 != 0x00)
	  {
		  return r1;		/* response error, return r1 */
	  }
		  
	  /* Set the block size */
	  r1 = _send_command(CMD16, MSD_BLOCKSIZE, 0xFF);
	  if(r1 != 0x00)
	  {
		  return r1;		/* response error, return r1 */
	  }
   }	
	
   /* r1=0x01 -> V2.x, read OCR register, check version */
   else if(r1 == 0x01)
   {
	 /* 4Bytes returned after CMD8 sent	*/
	 buff[0] = _spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[1] = _spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[2] = _spi_read_write(DUMMY_BYTE);				/* should be 0x01 */
	 buff[3] = _spi_read_write(DUMMY_BYTE);				/* should be 0xAA */
		
	 /* End of CMD8, chip disable and dummy byte */ 
	 _card_disable();
	 _spi_read_write(DUMMY_BYTE);
		
	 /* Check voltage range be 2.7-3.6V	*/
	 if(buff[2]==0x01 && buff[3]==0xAA)
	 {
		for(retry=0; retry<0xFFF; retry++)
		{
			r1 = _send_command(CMD55, 0, 0);			/* should be return 0x01 */
			if(r1!=0x01)
			{
				return r1;
			}				

			r1 = _send_command(ACMD41, 0x40000000, 0);	/* should be return 0x00 */
			if(r1 == 0x00)
			{
				retry = 0;
				break;
			}
		}
 		 	
		/* Timeout return */
		if(retry == 0xFFF)
		{
			gu8SDStatus = 3;
			return 3;
		}

		/* Read OCR by CMD58 */
	    r1 = _send_command_hold(CMD58, 0, 0);
	    if(r1!=0x00)
	    {
            return r1;		/* response error, return r1 */
	    }

	    buff[0] = _spi_read_write(DUMMY_BYTE);					
		buff[1] = _spi_read_write(DUMMY_BYTE);					
		buff[2] = _spi_read_write(DUMMY_BYTE);					
		buff[3] = _spi_read_write(DUMMY_BYTE);					

		/* End of CMD58, chip disable and dummy byte */
		_card_disable();
		_spi_read_write(DUMMY_BYTE);
	
	    /* OCR -> CCS(bit30)  1: SDV2HC	 0: SDV2 */
	    if(buff[0] & 0x40)
	    {
           CardInfo.CardType = CARDTYPE_SDV2HC;
	    }
	    else
	    {
           CardInfo.CardType = CARDTYPE_SDV2;
	    }

		/* Set spi speed high */
		MSD_SPIHighSpeed(1);
		}	
   }


    
   return 0;
}


/*******************************************************************************
* Function Name  : SDCardTest
* Description    : 向SD卡写入一个数，然后读出，判断是否一致，
				不一致则提示异常。
* Input          : None
* Output         : None
* Return         : 
* Attention		 : None
*******************************************************************************/
void SDCardTest(void)
{
	uint8 err = 0;
	uint8 lu8retry = 0;
	uint8 lu8SDTest = 0xaa; //用于测试SD卡写入读出。
	uint8 lu8tmp = 0;
	
	OSTimeDlyHMSM(0,0,2,0); //系统刚初始化完等待2S，保证准确。

	if(gu8SDStatus == 0){
		OSMutexPend (FlashMutex, 0, &err);
		for(lu8retry=0; lu8retry<5; lu8retry++){
			lu8SDTest = 0xaa;
			lu8tmp = 0;
			err = SDSaveData("/SD_Test", (uint8 *)(&lu8SDTest),1,0);
			if(err == NO_ERR){
				OSTimeDlyHMSM(0,0,0,100);
				err = SDReadData("/SD_Test",(uint8 *)(&lu8tmp),1,0);
				if(err == NO_ERR){
					if(lu8tmp == lu8SDTest){
						  gu8SDStatus = 0;
					    break;
				}
				else
					gu8SDStatus = 4; //指示SD卡写入读出不一致异常。
				}
				else
					gu8SDStatus = 4; //指示SD卡写入读出不一致异常。
			}
			else
				gu8SDStatus = 4; //指示SD卡写入读出不一致异常。
		}
	
		OSMutexPost (FlashMutex);
	}

}


/*******************************************************************************
* Function Name  : MSD_GetCardInfo
* Description    : Get SD Card Information
* Input          : None
* Output         : None
* Return         : 0：NO_ERR; TRUE: Error
* Attention		 : None
*******************************************************************************/
int MSD_GetCardInfo(PMSD_CARDINFO cardinfo)
{
  uint8_t r1;
  uint8_t CSD_Tab[16];
  uint8_t CID_Tab[16];

  /* Send CMD9, Read CSD */
  r1 = _send_command(CMD9, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(_read_buffer(CSD_Tab, 16, RELEASE))
  {
	return 1;
  }

  /* Send CMD10, Read CID */
  r1 = _send_command(CMD10, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(_read_buffer(CID_Tab, 16, RELEASE))
  {
	return 2;
  }  

  /* Byte 0 */
  cardinfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  cardinfo->CSD.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  cardinfo->CSD.Reserved1 = CSD_Tab[0] & 0x03;
  /* Byte 1 */
  cardinfo->CSD.TAAC = CSD_Tab[1] ;
  /* Byte 2 */
  cardinfo->CSD.NSAC = CSD_Tab[2];
  /* Byte 3 */
  cardinfo->CSD.MaxBusClkFrec = CSD_Tab[3];
  /* Byte 4 */
  cardinfo->CSD.CardComdClasses = CSD_Tab[4] << 4;
  /* Byte 5 */
  cardinfo->CSD.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  cardinfo->CSD.RdBlockLen = CSD_Tab[5] & 0x0F;
  /* Byte 6 */
  cardinfo->CSD.PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  cardinfo->CSD.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  cardinfo->CSD.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  cardinfo->CSD.DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  cardinfo->CSD.Reserved2 = 0; /* Reserved */
  cardinfo->CSD.DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  /* Byte 7 */
  cardinfo->CSD.DeviceSize |= (CSD_Tab[7]) << 2;
  /* Byte 8 */
  cardinfo->CSD.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  cardinfo->CSD.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  cardinfo->CSD.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
  /* Byte 9 */
  cardinfo->CSD.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  cardinfo->CSD.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  cardinfo->CSD.DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /* Byte 10 */
  cardinfo->CSD.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
  cardinfo->CSD.EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
  cardinfo->CSD.EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
  /* Byte 11 */
  cardinfo->CSD.EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
  cardinfo->CSD.WrProtectGrSize = (CSD_Tab[11] & 0x1F);
  /* Byte 12 */
  cardinfo->CSD.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  cardinfo->CSD.ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  cardinfo->CSD.WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  cardinfo->CSD.MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
  /* Byte 13 */
  cardinfo->CSD.MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
  cardinfo->CSD.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  cardinfo->CSD.Reserved3 = 0;
  cardinfo->CSD.ContentProtectAppli = (CSD_Tab[13] & 0x01);
  /* Byte 14 */
  cardinfo->CSD.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  cardinfo->CSD.CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  cardinfo->CSD.PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  cardinfo->CSD.TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  cardinfo->CSD.FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  cardinfo->CSD.ECC = (CSD_Tab[14] & 0x03);
  /* Byte 15 */
  cardinfo->CSD.CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  cardinfo->CSD.Reserved4 = 1;

  if(cardinfo->CardType == CARDTYPE_SDV2HC)
  {
	 /* Byte 7 */
	 cardinfo->CSD.DeviceSize = (u16)(CSD_Tab[8]) *256;
	 /* Byte 8 */
	 cardinfo->CSD.DeviceSize += CSD_Tab[9] ;
  }

  cardinfo->Capacity = cardinfo->CSD.DeviceSize * MSD_BLOCKSIZE * 1024;
  cardinfo->BlockSize = MSD_BLOCKSIZE;

  /* Byte 0 */
  cardinfo->CID.ManufacturerID = CID_Tab[0];
  /* Byte 1 */
  cardinfo->CID.OEM_AppliID = CID_Tab[1] << 8;
  /* Byte 2 */
  cardinfo->CID.OEM_AppliID |= CID_Tab[2];
  /* Byte 3 */
  cardinfo->CID.ProdName1 = CID_Tab[3] << 24;
  /* Byte 4 */
  cardinfo->CID.ProdName1 |= CID_Tab[4] << 16;
  /* Byte 5 */
  cardinfo->CID.ProdName1 |= CID_Tab[5] << 8;
  /* Byte 6 */
  cardinfo->CID.ProdName1 |= CID_Tab[6];
  /* Byte 7 */
  cardinfo->CID.ProdName2 = CID_Tab[7];
  /* Byte 8 */
  cardinfo->CID.ProdRev = CID_Tab[8];
  /* Byte 9 */
  cardinfo->CID.ProdSN = CID_Tab[9] << 24;
  /* Byte 10 */
  cardinfo->CID.ProdSN |= CID_Tab[10] << 16;
  /* Byte 11 */
  cardinfo->CID.ProdSN |= CID_Tab[11] << 8;
  /* Byte 12 */
  cardinfo->CID.ProdSN |= CID_Tab[12];
  /* Byte 13 */
  cardinfo->CID.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  /* Byte 14 */
  cardinfo->CID.ManufactDate = (CID_Tab[13] & 0x0F) << 8;
  /* Byte 15 */
  cardinfo->CID.ManufactDate |= CID_Tab[14];
  /* Byte 16 */
  cardinfo->CID.CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
  cardinfo->CID.Reserved2 = 1;

  return 0;  
}

/*******************************************************************************
* Function Name  : _read_buffer
* Description    : None
* Input          : - *buff:
*				   - len:
*				   - release:
* Output         : None
* Return         : 0：NO_ERR; TRUE: Error
* Attention		 : None
*******************************************************************************/
int _read_buffer(uint8_t *buff, uint16_t len, uint8_t release)
{
  uint8_t r1;
  uint16_t retry;

  /* Card enable, Prepare to read	*/
  _card_enable();

  /* Wait start-token 0xFE */
  for(retry=0; retry<2000; retry++)
  {
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if(r1 == 0xFE)
	 {
		 retry = 0;
		 break;
	 }
  }

  /* Timeout return	*/
  if(retry == 2000)
  {
	 _card_disable();
	 return 1;
  }

  /* Start reading */
  for(retry=0; retry<len; retry++)
  {
     *(buff+retry) = _spi_read_write(DUMMY_BYTE);
  }

  /* 2bytes dummy CRC */
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);

  /* chip disable and dummy byte */ 
  if(release)
  {
	 _card_disable();
	 _spi_read_write(DUMMY_BYTE);
  }

  return 0;
}
/*******************************************************************************
* Function Name  : MSD_ReadSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_ReadSingleBlock_DMA(uint32_t sector, uint8_t *buffer)
{
    DMA_InitTypeDef  DMA_InitStructure;
    u8 rvalue = MSD_RESPONSE_FAILURE;
    uint8_t r1;
    uint16_t retry;
    char DuumyClock=DUMMY_BYTE;

	if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }
    /*initial dma channel 2*/
    DMA_DeInit(DMA1_Channel4);
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 200;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
    
    
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&DuumyClock;  //512字节的dummy
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);  
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);  
    //    DMA_ClearFlag(DMA_FLAG_TC2); 
    /* Send CMD17 : Read single block command */
   r1 = _send_command(CMD17, sector, 0);
	
   if(r1 != 0x00)
   {
	 return 1;
   }
  
  /* Card enable, Prepare to read	*/
  _card_enable();

  /* Wait start-token 0xFE */
  for(retry=0; retry<2000; retry++)
  {
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if(r1 == 0xFE)
	 {
		 retry = 0;
		 break;
	 }
  }

  /* Timeout return	*/
  if(retry == 2000)
  {
	 _card_disable();
	 return 1;
  }
  
            DMA_Cmd(DMA1_Channel5,ENABLE);   
            DMA_Cmd(DMA1_Channel4,ENABLE);

            while(!DMA_GetFlagStatus(DMA1_FLAG_TC5));
            
            while(!DMA_GetFlagStatus(DMA1_FLAG_TC4));
            
            DMA_ClearFlag(DMA1_FLAG_TC4); 

     /* 2bytes dummy CRC */
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);

  /* chip disable and dummy byte */ 
       _card_disable();
	_spi_read_write(DUMMY_BYTE);

    /* Set response value to success */
    rvalue = MSD_RESPONSE_NO_ERROR;
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);

	
   

    /* Returns the reponse */
    return rvalue;
}
/*******************************************************************************
* Function Name  : MSD_ReadSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_ReadSingleBlock(uint32_t sector, uint8_t *buffer)
{
  uint8_t r1;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }
	
  /* Send CMD17 : Read single block command */
  r1 = _send_command(CMD17, sector, 0);
	
  if(r1 != 0x00)
  {
	 return 1;
  }
	
  /* Start read and return the result */
  r1 = _read_buffer(buffer, MSD_BLOCKSIZE, RELEASE);

  /* Send stop data transmit command - CMD12 */
  _send_command(CMD12, 0, 0);

  return r1;
}


/*******************************************************************************
* Function Name  : MSD_ReadBlock
* Description    : Reads a block of data from the MSD.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the MSD.
*                  - ReadAddr : MSD's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the MSD.
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed 
*******************************************************************************/
u8 MSD_ReadBlock_DMA(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
  u8 rvalue = MSD_RESPONSE_FAILURE;
  DMA_InitTypeDef  DMA_InitStructure;
  char DuumyClock[512];
//  INT8U err;
  memset(DuumyClock,0xff,512);
//  debug("MSD_ReadBlock_DMA!\r\n");
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 ReadAddr = ReadAddr<<9;
  }  
  /* MSD chip select low */
    MSD_CS_LOW();
    DMA_DeInit(DMA1_Channel4);
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)pBuffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 512;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
    
    
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)DuumyClock;  //512字节的dummy
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    //DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    /* Enable DMA1 Channel4 Transfer Complete interrupt */
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
    /* Enable DMA1 Channel5 Transfer Complete interrupt */
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
		
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);  
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);  
  /* Send CMD17 (MSD_READ_SINGLE_BLOCK) to read one block */
  MSD_SendCmd(CMD17, ReadAddr, 0xFF);

  /* Check if the MSD acknowledged the read block command: R1 response (0x00: no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    /* Now look for the data token to signify the start of the data */
      if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
      {
     
	    DMA_Cmd(DMA1_Channel5,ENABLE);   
            DMA_Cmd(DMA1_Channel4,ENABLE);
           #ifdef DMA1_IRQ 
	    OSFlagPend(Sem_SD_DMA,(OS_FLAGS)3,OS_FLAG_WAIT_SET_ALL,0,&err);  //请求信号量集的第0和第1位且都置1。		
		//debug("MSD_ReadBlock_DMA OSFlagPend err=%d \r\n",err);
		DMA_ClearFlag(DMA1_FLAG_TC4); 
           #else
            while(!DMA_GetFlagStatus(DMA1_FLAG_TC5));
            
            while(!DMA_GetFlagStatus(DMA1_FLAG_TC4));
        
            DMA_ClearFlag(DMA1_FLAG_TC4); 
            #endif
	      /* Get CRC bytes (not really needed by us, but required by MSD) */
	      MSD_ReadByte();
	      MSD_ReadByte();
	      /* Set response value to success */
             rvalue = MSD_RESPONSE_NO_ERROR;
       }
      else
       {
	       //debug("\r\n erro:MSD_START_DATA_SINGLE_BLOCK_READ\r\n");
       }
  }
  else
  {
	 //debug("\r\n error:MSD_RESPONSE_NO_ERROR\r\n");
  }

    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);
  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY_BYTE);

  /* Send stop data transmit command - CMD12 */
  _send_command(CMD12, 0, 0);
  /* Returns the reponse */
  return rvalue;
}
/*******************************************************************************
* Function Name  : MSD_ReadMultipleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
                   - count:扇区数
* Output         : None
* Return         : count  返回读剩余扇区数	0:OK 其它：ERROR
* Attention		 : None
*******************************************************************************/
int MSD_ReadMultipleBlock(uint32_t sector, uint8_t *buffer,uint8_t count)
{
  uint8_t r1;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }
  /* Send CMD18 : Read multiple block command */
  r1 = _send_command(CMD18, sector, 0);
	
  if(r1 != 0x00)
  {
	 return 1;
  }

  do {
		if (_read_buffer(buffer, MSD_BLOCKSIZE, HOLD)) break;
		buffer += 512;
  } while (--count);
  	
  /* Send stop data transmit command - CMD12 */
  _send_command(CMD12, 0, 0);

  return count;
}
/*******************************************************************************
* Function Name  : MSD_ReadMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint8 AppSdReadBytesInBlk(uint32 BlkAddr,uint32 OffsetAddr,uint32 InBlkByteNum,uint8* DataBuf)
{
	uint32 sector = BlkAddr;
	if((OffsetAddr+InBlkByteNum)>MSD_BLOCKSIZE){ //读范围不能垮块
		while(1);
	}
	
	if(OffsetAddr==0&&InBlkByteNum==MSD_BLOCKSIZE){ //读一整块
		if(MSD_ReadSingleBlock(sector, DataBuf)!=0){
			return FALSE;
		}
	}
	else{ //读块的一部分
		if(MSD_ReadSingleBlock(sector, gSdRWBuf)!=0){
			return FALSE;
		}
		memcpy(DataBuf,&gSdRWBuf[OffsetAddr],InBlkByteNum);
	}
	
	return TRUE;
}

/**************************************************************************
 author:  yangfei
*******************************************************************************/
void SPI_DMA_Send_Init(const unsigned char *SendBuff,unsigned int size)
{	
    DMA_InitTypeDef DMA_InitStructure;						//定义DMA初始化结构体		
    DMA_DeInit(DMA1_Channel5);  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)SendBuff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);	
}
/**************************************************************************
 author:  yangfei
*******************************************************************************/
void SPI_DMA_SendEnable(void)
{
	/* Enable SPI2 DMA Tx request */
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);  		
	DMA_Cmd(DMA1_Channel5, ENABLE);  					
}
/*******************************************************************************
* Function Name  : MSD_GetDataResponse
* Description    : Get MSD card data response.
* Input          : None
* Output         : None
* Return         : The MSD status: Read data response xxx0<status>1
*                   - status 010: Data accecpted
*                   - status 101: Data rejected due to a crc error
*                   - status 110: Data rejected due to a Write error.
*                   - status 111: Data rejected due to other error.
*******************************************************************************/
u8 MSD_GetDataResponse(void)
{
  u32 i = 0;
  u8 response, rvalue;

  while (i <= 64)
  {
    /* Read resonse */
    //response = MSD_ReadByte();
    response = _spi_read_write(DUMMY_BYTE);
    /* Mask unused bits */
    response &= 0x1F;

    switch (response)
    {
      case MSD_DATA_OK:
      {
        rvalue = MSD_DATA_OK;
        break;
      }

      case MSD_DATA_CRC_ERROR:
        return MSD_DATA_CRC_ERROR;

      case MSD_DATA_WRITE_ERROR:
        return MSD_DATA_WRITE_ERROR;

      default:
      {
        rvalue = MSD_DATA_OTHER_ERROR;
        break;
      }
    }
    /* Exit loop in case of data ok */
    if (rvalue == MSD_DATA_OK)
      break;
    /* Increment loop counter */
    i++;
  }
  /* Wait null data */
  while (_spi_read_write(DUMMY_BYTE) == 0);
  /* Return response */
  return response;
}
/*******************************************************************************
* Function Name  : MSD_SendCmd
* Description    : Send 5 bytes command to the MSD card.
* Input          : - Cmd: the user expected command to send to MSD card
*                  - Arg: the command argument
*                  - Crc: the CRC
* Output         : None
* Return         : None
*******************************************************************************/
void MSD_SendCmd(u8 Cmd, u32 Arg, u8 Crc)
{
  u32 i = 0x00;
  u8 Frame[6];

  /* Construct byte1 */
  Frame[0] = (Cmd | 0x40);
  /* Construct byte2 */
  Frame[1] = (u8)(Arg >> 24);
  /* Construct byte3 */
  Frame[2] = (u8)(Arg >> 16);
  /* Construct byte4 */
  Frame[3] = (u8)(Arg >> 8);
  /* Construct byte5 */
  Frame[4] = (u8)(Arg);
  /* Construct CRC: byte6 */
  Frame[5] = (Crc);

  /* Send the Cmd bytes */
  for (i = 0; i < 6; i++)
  {
    //MSD_WriteByte(Frame[i]);
	 _spi_read_write(Frame[i]);
  }
}

/*******************************************************************************
* Function Name  : MSD_WriteSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_WriteSingleBlock_DMA(uint32_t sector, uc8 *buffer,u16 NumByteToWrite)
{
  //uint8_t r1;
  //uint16_t i=0;
  uint32_t retry;
  u8 value=0;
//  INT8U err;
  u8 rvalue = MSD_RESPONSE_FAILURE;
  DMA_InitTypeDef DMA_InitStructure;						//定义DMA初始化结构体	
  
  /*begin:yangfei added 2012.11.29*/ 
   //debug("MSD_WriteSingleBlock_DMA\r\n");
    _card_enable();	
    DMA_DeInit(DMA1_Channel5);  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize =NumByteToWrite;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    /* Enable DMA1 Channel5 Transfer Complete interrupt */
    #ifdef DMA1_IRQ
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    #endif
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);  
 /*end:yangfei added 2012.11.29*/	
  //SPI_DMA_Send_Init(buffer,MSD_BLOCKSIZE);
  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }	
  
  /* Card enable, Prepare to write */
  
  MSD_SendCmd(CMD24, sector, 0xff);
  //_spi_read_write(DUMMY_BYTE);
  //_spi_read_write(DUMMY_BYTE);
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
	  _spi_read_write(DUMMY_BYTE);
	  /* Start data write token: 0xFE */
	  _spi_read_write(0xFE);
		
	/*begin:yangfei added 2012.11.28*/
	DMA_Cmd(DMA1_Channel5, ENABLE);

     #ifdef DMA1_IRQ 
         OSFlagPend(Sem_SD_DMA,(OS_FLAGS)1,OS_FLAG_WAIT_SET_ALL,0,&err);  //请求信号量集的第0位置1
         DMA_ClearFlag(DMA1_FLAG_TC5);
      #else
	 
	 while(!DMA_GetFlagStatus(DMA1_FLAG_TC5))  ;
	 DMA_ClearFlag(DMA1_FLAG_TC5);
      #endif
	  /* 2Bytes dummy CRC */
	  _spi_read_write(DUMMY_BYTE);
	  _spi_read_write(DUMMY_BYTE);

	value=MSD_GetDataResponse();
		
	    if (value == MSD_DATA_OK)
	    {
	      rvalue = MSD_RESPONSE_NO_ERROR;
	    }
	    //debug("value=%x\r\n",value);
   }
  /*begin:yangfei added 2012.12.07 for wait programm finished else write error*/
	 /* Wait all the data programm finished */
	  retry = 0;
	  while(_spi_read_write(DUMMY_BYTE) == 0x00)
	  {	
		 /* Timeout return */
		 if(retry++ == 0x40000)
		 {
		    _card_disable();
		    return 3;
		 }
	  }
  /* chip disable and dummy byte */ 
  _card_disable();
  _spi_read_write(DUMMY_BYTE);
	
	/*yangfei added*/
	DMA_Cmd(DMA1_Channel5, DISABLE); 
		
  return rvalue;
}
/************************************************************************************
* Function Name  : MSD_WriteMultipleBlock_DMA(uint32_t sector, uc8 *buffer，u16 NumByteToWrite)
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
************************************************************************************/
int MSD_WriteMultipleBlock_DMA(uint32_t sector, uc8 *buffer, u8 NbrOfSector, u16 NumByteToWrite)
{
  uint8 r1;
//  uint16_t i=0;
  uint32_t retry;
  u8 value=0;
//  INT8U err;
  u8 rvalue = MSD_RESPONSE_FAILURE;
  DMA_InitTypeDef DMA_InitStructure;                                    //定义DMA初始化结构体	
  
    /*begin:yangfei added 2012.11.29*/ 
//    debug("MSD_WriteMultipleBlock_DMA\r\n");
    _card_enable();
    DMA_DeInit(DMA1_Channel5);  
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize =NumByteToWrite;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    /* Enable DMA1 Channel5 Transfer Complete interrupt */
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);  
 /*end:yangfei added 2012.11.29*/	
  //SPI_DMA_Send_Init(buffer,MSD_BLOCKSIZE);
  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
     sector = sector<<9;
  }
  if(CardInfo.CardType != CARDTYPE_MMC)
  {
        _send_command(ACMD23, NbrOfSector, 0);
   }
  /* Send CMD25 : Write multiple block command */
  MSD_SendCmd(CMD25, sector, 0xff);
  //_spi_read_write(DUMMY_BYTE);
  //_spi_read_write(DUMMY_BYTE);
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
	  _spi_read_write(DUMMY_BYTE);
	  /* Start data write token: 0xFE */
	  _spi_read_write(0xFC);

	/*begin:yangfei added 2012.11.28*/
	DMA_Cmd(DMA1_Channel5, ENABLE);

     #ifdef DMA1_IRQ 
         OSFlagPend(Sem_SD_DMA,(OS_FLAGS)1,OS_FLAG_WAIT_SET_ALL,0,&err);  //请求信号量集的第0位置1
         DMA_ClearFlag(DMA1_FLAG_TC5);
      #else
	 
	 while(!DMA_GetFlagStatus(DMA1_FLAG_TC5))  ;
	 DMA_ClearFlag(DMA1_FLAG_TC5);
      #endif
	  /* 2Bytes dummy CRC */
	  _spi_read_write(DUMMY_BYTE);
	  _spi_read_write(DUMMY_BYTE);

	value=MSD_GetDataResponse();

	    if (value == MSD_DATA_OK)
	    {
	      rvalue = MSD_RESPONSE_NO_ERROR;
	    }
//	   debug("value=%x\r\n",value);
   }
   /* Send end of transmit token: 0xFD */
    r1 = _spi_read_write(0xFD);
    if(r1 == 0x00)
    {
        return 4;
    } 
  /*begin:yangfei added 2012.12.07 for wait programm finished else write error*/
	 /* Wait all the data programm finished */
	  retry = 0;
	  while(_spi_read_write(DUMMY_BYTE) == 0x00)
	  {
		 /* Timeout return */
		 if(retry++ == 0x40000)
		 {
		    _card_disable();
		    return 3;
		 }
	  }
  /* chip disable and dummy byte */ 
  _card_disable();
  _spi_read_write(DUMMY_BYTE);
  
        /*yangfei added*/
        DMA_Cmd(DMA1_Channel5, DISABLE); 

  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_WriteSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_WriteSingleBlock(uint32_t sector, uc8 *buffer)
{
  uint8_t r1;
  uint16_t i;
  uint32_t retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }
	
  /* Send CMD24 : Write single block command */
  r1 = _send_command(CMD24, sector, 0);
	
  if(r1 != 0x00)
  {
	 return 1;
  }

  /* Card enable, Prepare to write */
  _card_enable();
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
  /* Start data write token: 0xFE */
  _spi_read_write(0xFE);
	
  /* Start single block write the data buffer */
  for(i=0; i<MSD_BLOCKSIZE; i++)
  {
    _spi_read_write(*buffer++);
  }

  /* 2Bytes dummy CRC */
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
	
  /* MSD card accept the data */
  r1 = _spi_read_write(DUMMY_BYTE);
  if((r1&0x1F) != 0x05)
  {
    _card_disable();
    return 2;
  }
	
  /* Wait all the data programm finished */
  retry = 0;
  while(_spi_read_write(DUMMY_BYTE) == 0x00)
  {	
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	    _card_disable();
	    return 3;
	 }
  }

  /* chip disable and dummy byte */ 
  _card_disable();
  _spi_read_write(DUMMY_BYTE);
	
  return 0;
}
/*******************************************************************************
* Function Name  : MSD_WriteMultipleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_WriteMultipleBlock(uint32_t sector, uc8 *buffer,uint8_t count)
{
  uint8_t r1;
  uint16_t i;
  uint32_t retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }
   if(CardInfo.CardType != CARDTYPE_MMC)
  {
        _send_command(ACMD23, count, 0);
   }
  /* Send CMD25 : Write multiple block command */
  r1 = _send_command(CMD25, sector, 0);
	
  if(r1 != 0x00)
  {
	 return 1;
  }

  /* Card enable, Prepare to write */
  _card_enable();
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
  /* Start data write token: 0xFE */
  //_spi_read_write(0xFE);

  do{
                _spi_read_write(0xFC);
		for(i=0; i<MSD_BLOCKSIZE; i++)
		{
		  _spi_read_write(*buffer++);
		}
		/* 2Bytes dummy CRC */
		  _spi_read_write(DUMMY_BYTE);
		  _spi_read_write(DUMMY_BYTE);
			
		  /* MSD card accept the data */
		  r1 = _spi_read_write(DUMMY_BYTE);
		  if((r1&0x1F) != 0x05)
		  {
		    _card_disable();
		    return 2;
		  }
		  
		  /* Wait all the data programm finished */
		  retry = 0;
		  while(_spi_read_write(DUMMY_BYTE) == 0x00)
		  {	
			 /* Timeout return */
			 if(retry++ == 0x40000)
			 {
			    _card_disable();
			    return 3;
			 }
		  }

  } while (--count);
 
   r1 = _spi_read_write(0xFD);
    if(r1 == 0x00)
  {
        return 4;
  }
    
  /* Wait all the data programm finished */
  retry = 0;
  while(_spi_read_write(DUMMY_BYTE) == 0x00)
  {	
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	    _card_disable();
	    return 5;
	 }
  }
  /* chip disable and dummy byte */ 
  _card_disable();
  _spi_read_write(DUMMY_BYTE);
	
  return count;
}
/*******************************************************************************
* Function Name  : MSD_WriteMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint8 AppSdWriteBytesInBlk(uint32 BlkAddr, uint32 OffsetAddr, uint32 InBlkByteNum, uint8* DataBuf)
{
	uint32 sector = BlkAddr;
	
  	if((OffsetAddr+InBlkByteNum)>MSD_BLOCKSIZE){ 					//写范围不能垮块
		while(1);
		}
	
	if(OffsetAddr==0&&InBlkByteNum==MSD_BLOCKSIZE)					//写一整块
		{ 
			if(MSD_WriteSingleBlock(sector, DataBuf)!=0)
				{
					return FALSE;
				}
		}
	else{ 															//写块的一部分
			if(MSD_ReadSingleBlock(sector,gSdRWBuf)!=0)
				{
					return FALSE;
				}
				
		memcpy(&gSdRWBuf[OffsetAddr],DataBuf,InBlkByteNum);
		
		if(MSD_WriteSingleBlock(sector,gSdRWBuf)!=0)
			{
				return FALSE;
			}
	}

  return TRUE;
}

/*******************************************************************************
* Function Name  : AppSdWrRdBytes
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/

uint8 AppSdWrRdBytes(uint32 SdAddr, uint32 ByteNum, uint8* DataBuf, uint8 RdOrWr)
{
	uint32 BlkAddr,OffsetAddr,InBlkByteNum;
	uint8 exit;
	
	if(ByteNum==0){
		return FALSE;
	}
	
	exit = FALSE;
	
	BlkAddr = SdAddr/MSD_BLOCKSIZE;
	while(1)
	{
		OffsetAddr = SdAddr-BlkAddr*MSD_BLOCKSIZE; 					//块内偏移地址
		
		//计算块内读或写得字节数
		if(ByteNum > (MSD_BLOCKSIZE-OffsetAddr))
			{
				InBlkByteNum = MSD_BLOCKSIZE-OffsetAddr;
			}
		else
			{
				InBlkByteNum = ByteNum;
				exit = TRUE;
			}
		//---------------单块范围内的读或写
		if(RdOrWr == SD_WR)
			{
				if(AppSdWriteBytesInBlk(BlkAddr,OffsetAddr,InBlkByteNum,DataBuf)==FALSE)
					{
						gSD_Err++;
						return FALSE;
					}
			}
		else{
				if(AppSdReadBytesInBlk(BlkAddr,OffsetAddr,InBlkByteNum,DataBuf)==FALSE)
					{
						gSD_Err++;
						return FALSE;
					}
			}
			
		//--------------
		if(exit==TRUE){
			break;
		}
		
		BlkAddr++;
		SdAddr += InBlkByteNum;
		ByteNum -= InBlkByteNum;
		DataBuf += InBlkByteNum;
	}
	return TRUE;
}

/*******************************************************************************
* Function Name  : _send_command
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int _send_command(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t r1;
  uint8_t retry;

  /* Dummy byte and chip enable */
  _spi_read_write(DUMMY_BYTE);
  _card_enable();

  /* Command, argument and crc */
  _spi_read_write(cmd | 0x40);
  _spi_read_write(arg >> 24);
  _spi_read_write(arg >> 16);
  _spi_read_write(arg >> 8);
  _spi_read_write(arg);
  _spi_read_write(crc);
  
  /* Wait response, quit till timeout */
  for(retry=0; retry<200; retry++)
  {
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if(r1 != 0xFF)
	 {
		 break;
	 }
  }

  /* Chip disable and dummy byte */ 
  _card_disable();
  _spi_read_write(DUMMY_BYTE);

  return r1;
}	

/*******************************************************************************
* Function Name  : _send_command_hold
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int _send_command_hold(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t r1;
  uint8_t retry;

  /* Dummy byte and chip enable */
  _spi_read_write(DUMMY_BYTE);
  _card_enable();

  /* Command, argument and crc */
  _spi_read_write(cmd | 0x40);
  _spi_read_write(arg >> 24);
  _spi_read_write(arg >> 16);
  _spi_read_write(arg >> 8);
  _spi_read_write(arg);
  _spi_read_write(crc);
  
  /* Wait response, quit till timeout */
  for(retry=0; retry<200; retry++)
  {
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if(r1 != 0xFF)
	 {
		 break;
	 }
  }

  return r1;
}

