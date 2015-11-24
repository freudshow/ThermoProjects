//文件名：ModBus.h

#ifndef        _ModBus_H
#define		   _ModBus_H


//发送数据实例（读取浮点型数据，实际上PLC只有两种数据32位浮点型以及16位无符号型，这个可以在上位机限定死，只读00 02，以及00 01）	 
// 设备ID	读功能	 起始地址413   读取两个地址的数据（4个字节）										  CRC校验 （crc 16-2）
// 01       03       01 9B          00 02 （均是16进制，读取）  对应 40413（十进制）地址（413是有效地址） B4 18 
//返回数据
// 	设备ID	 读功能	 读取字节数（或者说返回的字节数）  返回字节数 （前面读了2个地址则返回4个字节）	  CRC校验
//  01        03     04                                3F 19 99 9A                                     CD DB 
//发送数据格式	   ModBus是大端结构，高位在低地址位，先发送高地址位
//Modbus读数据结构
#pragma pack(1) 
typedef struct{
        //3个字节的时间缓冲	，与串口波特率相关
		uint8 SlaveAddr;					//这里是PLC地址（地址域），0是广播-----物理内存最低地址  1
		uint8 FucCode;					//功能代码（功能域）	2
		unsigned short 	Adrr;			 //地址
		//uint8 HAdrr;					//读的高地址位	 （）	 3
		//uint8 LAdrr;				//读的低地址位
		unsigned short 	DataLength;
		//uint8 HDataLength;					//地址数量高位	00	(0x0022)
		//uint8 LDataLength;					//地址数量低位	02
		//uint8 HData;							//数据域高位
		//uint8 LData;							//数据域低位
		unsigned short CRCData;
		//uint8 HCRCData;									//crc校验高位
		//uint8 LCRCData;									//crc校验低位	----- 物理内存最高地址
	     //3个字节的时间缓冲 ，与串口波特率相关
	}ModBusMRData;	//写数据

//Modbus读从设备返回数据
typedef struct{
        //3个字节的时间缓冲	，与串口波特率相关
		uint8 SlaveAddr;					//这里是PLC地址（地址域），0是广播-----物理内存最低地址  1
		uint8 FucCode;					//功能代码（功能域）	2
		uint8 ByteNUM;					//返回有效数据的字节数	 （）	 3
		uint8 Data[8];					//地址数量高位	00	(0x0022),最多4个自己，这里的缓冲区大一些

		unsigned short CRCData;
		//uint8 HCRCata;									//crc校验高位
		//uint8 LCRCata;									//crc校验低位	----- 物理内存最高地址
	     //3个字节的时间缓冲 ，与串口波特率相关
	}ModBusSRData;  //读数据

//OLC的上位机下发的数据域结构体
typedef struct
{
	//uint8 ForAllOrOne;
	//uint8 Device_type;
	uint8 Channel;  //通道号   上位机
	uint8 ProtocolVer;		//协议版本号   上位机
	uint8 ControlCode[6];	  //上位机下发的PLC配置信息
	uint8 Lenth;		  //上位机下发的数据域长度
}FrameMsg_31;

#pragma pack()

extern unsigned short crc(unsigned char *puchMsg , unsigned short usDataLen);
extern void construct_rtu_frm ( unsigned char *dst_buf,unsigned char *src_buf,unsigned char lenth);
//void construct_rtu_frm ( unsigned char *dst_buf,unsigned char *src_buf,unsigned char lenth);
//extern int rtu_data_anlys(unsigned char *source_p,unsigned char rtu_number_long);
extern  void  rtu_read_coil_status ( ModBusMRData *PLCCMD,unsigned char n); 
#endif
