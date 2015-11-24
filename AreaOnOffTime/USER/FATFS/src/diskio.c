/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2012        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: USB drive control */
//#include "atadrive.h"	/* Example: ATA drive control */
//#include "sdcard.h"		/* Example: MMC/SDC contorl */

/*yangfei added 2012-11-21*/
#include <includes.h>
#include "SPI_MSD_Driver.h"

/* Definitions of physical drive number for each media */
#define ATA		0
#define MMC		1
#define USB		2
/*--------------------Extern------------------------*/
extern MSD_CARDINFO CardInfo;
/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	return 0;
	
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
        //debug("\r\ndisk_read : sector=%d count=%d\r\n",sector,count);
	if (count == 1) {	
		if(MSD_ReadSingleBlock(sector, buff)==0)
		//if(MSD_ReadBlock_DMA(buff, sector, 512) == 0)
		{
			count = 0;
			//debug("ReadSingleBlock ok!\r\n");
		}
		else
		{
			//debug("ReadSingleBlock error!\r\n");
		}
	}	
	else {				
		if(MSD_ReadMultipleBlock(sector, buff,count)== 0)
		{
			count = 0;
			//debug("ReadMultipleBlock ok!\r\n");
		}
		else
		{	
		    //debug("ReadMultipleBlock error!\r\n");
		}
	}
	/*
	do
	{
         if(MSD_ReadSingleBlock(sector++, buff)) break;
	}while(--count);   */
	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
//	debug("\r\ndisk_write : sector=%d count=%d\r\n",sector,count);
	#if 1
	if (count == 1) {	
		if(MSD_WriteSingleBlock(sector, buff)==0)
		{
			count = 0;
			//debug("writeSingleBlock ok!\r\n");
		}
	}	
	else {				
		if(MSD_WriteMultipleBlock(sector, buff,count)== 0)
		{
			count = 0;
			//debug("writeMultipleBlock ok!\r\n");
		}
		else
		{
		    //debug("writeMultipleBlock error!\r\n");
		}
	}
	#endif
	#if 0
	do
	{
              err=MSD_WriteSingleBlock_DMA( sector++, buff,512);
		if(err) 
		{
		  //debug("err=%d\r\n",err);
		  break; 
		}

		buff += 512;
	}while(--count);   
	#endif
         #if 0
	if (count == 1) {
		if(MSD_WriteSingleBlock_DMA(sector, buff, 512)== 0)
		{
                        count = 0;
                        //debug("writeSingleBlock_DMA ok!\r\n");
		}
	}	
	else {
		if(MSD_WriteMultipleBlock_DMA(sector, buff , count , count*512)== 0)
		{
                        count = 0;
                        //debug("writeMultipleBlock_DMA ok!\r\n");
		}
		else
		{
		    //debug("writeMultipleBlock_DMA error!\r\n");
		}
	}
	#endif  
  	return count ? RES_ERROR : RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{

  if ( MSD_GetCardInfo(&CardInfo)) return RES_NOTRDY;
  switch ( ctrl ){
        case GET_SECTOR_COUNT:
                  *(u32*)buff = (u32) (CardInfo.Capacity/CardInfo.BlockSize);
                  //*(u32*)buff = 4*1024*1024*2;
                     break;
              case GET_SECTOR_SIZE:
                  //*(u16*)buff = (u16) SDCard.BlockLength;
                     break;
              case GET_BLOCK_SIZE:
                 *(u32*)buff = (u32) CardInfo.BlockSize;
                // *(u32*)buff = 512;
                     break;
              default:
                  break;
      }
  return RES_OK;
}


DWORD get_fattime (void)
{
	DWORD time;
	uint8 systime[6];
	CPU_SR			cpu_sr;	
	
	OS_ENTER_CRITICAL();
	memcpy(systime,gSystemTime,6);
	OS_EXIT_CRITICAL();
	
	time=((DWORD)(systime[ARRAY_YEAR]+20) << 25)	/* Year = 2012 */
			| ((DWORD)systime[ARRAY_MONTH] << 21)				/* Month = 1 */
			| ((DWORD)systime[ARRAY_DAY]  << 16)				/* Day_m = 1*/
			| ((DWORD)systime[ARRAY_HOUR]  << 11)				/* Hour = 0 */
			| ((DWORD)systime[ARRAY_MINUTE] << 5)				/* Min = 0 */
			| ((DWORD)0 >> 1);				/* Sec = 0 */		
	return	  time;
}


#endif
