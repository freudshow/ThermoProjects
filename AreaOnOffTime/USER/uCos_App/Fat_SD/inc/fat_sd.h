/****************************************Copyright (c)****************************************************
**                                      
**                                 山东华宇空间技术公司(西安分部) 
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               fat_sd.h
** Descriptions:            includes.h for ucos
**
**--------------------------------------------------------------------------------------------------------

** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/


#ifndef  __FAT_SD_H__
#define  __FAT_SD_H__

#ifdef __cplusplus
	extern "C" {
	#endif

/* Includes ------------------------------------------------------------------*/
#include  <stdio.h>
#include  <string.h>
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <math.h>


#define DEC_TO_ASCII(x)  ((x)+0x30)

/* -----------------------Global variable-------------------------------------*/

/* -----------------------------------------------------------------------*/
extern void mountSd(void);
extern uint8 SDSaveData(const char* path, const void* buffer, uint16 NumByteToWrite,uint32 off);
extern uint8 SDSaveUpdateData(const char* path, const void* buffer, uint16 NumByteToWrite,uint32 off);
extern uint8 SDReadData(const char* path, void* buffer, uint16 NumByteToRead,uint32 off);
extern uint8 SDSaveLog(const char* path, const void* buffer, uint16 NumByteToWrite,uint32 off);
extern void GetFilePath(char* path,uint8* time,int ARRAY);
extern void GetTimeNodeFilePath(char* path,uint8 * time,uint16* TimeNodes);
extern void GetTimeNodeFilePath_Forward(char* path,uint8 * time,DataStoreParaType *datastorePara);
extern void GetTimeNode(uint8 * time,uint16* TimeNodes);
extern void BcdTimeToHexTime(uint8* BcdTime, uint8* HexTime);

extern void MakeDirInit(uint8 Month);
extern void MakeDir(char* dir);

/*用户命令函数声明。*/
void mountSd(void);
void UartCmdHelp(u8 argc,void **argv);
void UartCmdCls(u8 argc,void **argv);
void UartCmdls(u8 argc,void **argv);
void UartCmdmv(u8 argc,void **argv);
void UartCmdcd(u8 argc,void **argv);
void UartCmdmkdir(u8 argc,void **argv);
void UartCmdpwd(u8 argc,void **argv);
void UartCmdcat(u8 argc,void **argv);
void UartCmdrm(u8 argc,void **argv);
void UartCmdnew(u8 argc,void **argv);
void UartCmdSDWrite(u8 argc,void **argv);
void UartCmdtime(u8 argc,void **argv);
extern void Uart_ClearScreen(void);
void UartCmdSetDebugLevel (u8 argc,void **argv);

void UartCmdReset(u8 argc,void **argv);
void UartCmdSetBKP(u8 argc,void **argv);
void UartCmdCp(u8 argc,void **argv);
void UartCmdVersion(u8 argc,void **argv);
void UartSetChannel(u8 argc,void **argv);
void UartCmdTest(u8 argc,void **argv);
uint8 MakeFile(const char* path);
#ifdef __cplusplus
	}
	#endif

#endif

/*********************************************************************************************************
      										END FILE
*********************************************************************************************************/
