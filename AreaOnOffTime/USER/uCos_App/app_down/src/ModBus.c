 /*
 文件名：ModBus.c
 创建人：林晓彬
 创建日期：2014、2、27
 版本号：01
 主要应用：用于485同PLC的通信
 修改记录：
 */
 //modbus

#include <includes.h>
#include "ModBus.h"


//MODBUS功能码

#define READ_COIL     01          //读继电器
#define READ_DI       02          //读输入状态 READ INPUT STATUS
#define READ_HLD_REG  03          //读保持器
#define READ_AI       04          //读输入寄存器 READ INPUT REGISTER 
								 
								 //05 WRITE SINGLE COIL
								 //06 WRITE SINGLE REGISTER
								 //15 WRITE MULTIPLE COIL
								 // 16 WRITE MULTIPLE REGISTER


#define PROTOCOL_EXCEPTION 0x81   //协议超出
#define PROTOCOL_ERR  1           //协议错误
#define FRM_ERR       2           //格式错误  

//PLC通信串口
//波特率9600,8位，无校验， 1个停止位

//临时的全局变量，485串口的发送缓冲 数组 com_out_aa
//unsigned char 	com_out_aa[8];	// 

		

//CRC高低字节校验码表   
const unsigned char auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
} ;
const unsigned  char auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
};


											 
//CRC校验子函数，入口：数据指针+数据段长度
/*
输入参数
*puchMsg   ：要发送数据的指针
usDataLen  ： 要发送数据的字节数
*/
unsigned short crc(unsigned char *puchMsg , unsigned short usDataLen)
   {
    unsigned char uchCRCHi = 0xFF ; /* 高CRC初始化*/
    unsigned char uchCRCLo = 0xFF ; /* 低CRC初始化*/
    unsigned uIndex ;  /* 用来查表 */
    
    while (usDataLen--) /* 整个数据缓冲区，不带数据的CRC两个字节 */
    {
     uIndex = uchCRCHi ^ *puchMsg++ ; /* calculate the CRC */
     uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
     uchCRCLo = auchCRCLo[uIndex] ;
    }
    return (uchCRCHi << 8 | uchCRCLo) ;
}

/*
//函数功能：计算要发送的有效数据的CRC值，并将数据由协议缓冲区移到485对应的发送队列缓冲区
//输入参数：	*src_buf 要发送数据的目源缓冲区---将要发送的MBUS指令打包好（CRC值是在这个函数即时计算的）
//输入参数：	*dst_buf 要发送数据的发送缓冲区----在这里是指串口的发送队列

*/
void construct_rtu_frm ( unsigned char *dst_buf,unsigned char *src_buf,unsigned char lenth)
{
    unsigned short  crc_tmp;
    crc_tmp = crc(src_buf, lenth);   //原地址加数据段做CRC计算  计算要发送的有效数据的CRC值（16bit的数据）
     
    *(src_buf+lenth) = crc_tmp >> 8 ;  //把CRC高位防置在随后的地址	由此要求：src_buf作为要发送数据的缓冲区，其长度要大于要发送数据的最大长度
    *(src_buf+lenth+1) = crc_tmp & 0xff;  //放CRC低位
    lenth++;                             //把数据指针指向最后
    lenth++;							 //增加了两个CRC字节 则这里必须增加数据长度
 
    while ( lenth--)                   //搬动协议缓冲区到串口发送缓冲区  
    {
       *dst_buf = *src_buf;
       dst_buf++;
       src_buf++;	
     }
}
//读继电器状态响应   或者 或者向PLC中写入一个16bit的数据，返回值的检验是上位机进行一次读操作
//读的格式为：主机地址    读功能码    读的高位地址   读的低位地址    字节长度高    字节长度低   CRC校验，共8个字节 ，地址是16位的
//响应格式为：从机地址      功能码      有效数据字节数   n个有效字节数     CRC校验码
/*
//输入参数：	board_adr PLC标识地址
				unsigned short start_address  读的起始地址
				unsigned short lenth:读取数据的字节数，读取的返回数据只有两种，32位浮点数，和16位无符号数
				这里改为传入结构体指针		ModBusMRData ,为从上位机接收的指令

这个函数的名字要修改一下，修改成，作为PLC 读写函数使用，先向里面写一个数
*/
void  rtu_read_coil_status ( ModBusMRData *PLCCMD,unsigned char n) 
   {
    unsigned char tmp[16], tmp_lenth;		 //tmp[100] 是发送数据的缓冲区
	unsigned char com_out_aa[16];//临时缓冲区
    //unsigned int temp_start;
    //unsigned char temp1,temp2;
    //ModBusMRData *PLCCMD
	//PLCCMD  上位机传下数据 0x01030000100002到pData->MeterAddr[6]={0x02，0x00，0x01, 0x00, 0x03, 0x01, 0x00};	位置需要调整
	/*			//	这个看上位机怎么操作
	tmp[0] = datain[5];	  //ID
	tmp[1] =datain[4];	   //功能码
	tmp[2] = datain[3];  //高位地址
	tmp[3] = datain[2];//低位地址
    tmp[4] = datain[1]; //地址数 高位
	tmp[5] = datain[0];	  //地址数 低位
	*/


	/*			//	这个看上位机怎么操作
	tmp[0] = datain[5];	  //ID
	tmp[1] =datain[4];	   //功能码
	tmp[2] = datain[3];  //高位地址
	tmp[3] = datain[2];//低位地址
    tmp[4] = datain[1]; //地址数 高位
	tmp[5] = datain[0];	  //地址数 低位
	*/

	
	tmp[0] = PLCCMD->SlaveAddr;
	tmp[1] = PLCCMD->FucCode;
	tmp[2] = PLCCMD->Adrr >> 8;  //高位地址
	tmp[3] = PLCCMD->Adrr;//低位地址
    tmp[4] = PLCCMD->DataLength >> 8; //地址数 高位
	tmp[5] = PLCCMD->DataLength;	  //地址数 低位
	//PLCCMD->CRCData;  不需要
	
	tmp_lenth = n;   
    

    
    construct_rtu_frm (com_out_aa,tmp, tmp_lenth);	 //com_out_aa  是串口的发送缓冲区  ，这里要替换

	memcpy((unsigned char*)PLCCMD,com_out_aa,8);
    //return (tmp_lenth+2);
	//将数据
    }


/*
ModBus 必须对返回的数据进行CRC校验，基本流程是取得所有的返回值，计算CRC值，并与返回的CRC值进行比较，相等则通信成功
//对返回的数据做RTU数据分析
//入口，接收缓冲区的大小，长度
//输入参数： unsigned char *source_p  接收数据缓冲区
             rtu_number_long：读回一次数据的数据长度（包含整个协议）
*/			 
int rtu_data_anlys(unsigned char *source_p,unsigned char rtu_number_long)
   {
    unsigned short crc_result; 
	unsigned short crc_tmp; //将缓冲区的数据的CRC校验码提取出来
    //unsigned char tmp1;
    //unsigned int tmp2,tmp3,tmp4;
    //unsigned char *source_p;
    //unsigned char temp;
    
    //source_p = uart0_bb;
    //temp = *source_p;
    //if(board_address!=temp) return;
    
    crc_tmp = *(source_p + rtu_number_long-2);                // 由接收缓冲区计算出 crc  第一字节  (这个CRC值还是大端结构) 
    crc_tmp = crc_tmp * 256 + *( source_p+rtu_number_long-1); // 得到实际  CRC 值	 （小端结构）
    crc_result = crc(source_p, rtu_number_long-2);            // 计算 CRC 值
    
   
    if ( crc_tmp != crc_result ) // 对比 CRC 校验正确
      {
       return -1;
       }
	/*整个先不添加*/
	/*
    //将modubus协议数据（大端结构）转换成符合上位机的小端结构  
    //source_p=uart1_aa;
    tmp1=*source_p;
    tmp3=(*(source_p+2)*256)+*(source_p+3);
    tmp4=(*(source_p+4)*256)+*(source_p+5);
    switch ( *(source_p+1) ) // 功能码   根据功能码，进行读写操作（解析上位机的指令）
        {
        case READ_COIL:                   //读取继电器状态 
            tmp2=rtu_read_coil_status (tmp1,tmp3,tmp4);
            rtu_out_active(tmp2);	
            break;

        case READ_DI: //读取开关量输入
            tmp2=rtu_read_input_status(tmp1,tmp3,tmp4);
            rtu_out_active(tmp2);	
            break;

        case READ_HLD_REG:  //读取保持寄存器
            tmp2=rtu_read_hldreg(tmp1,tmp3,tmp4);
            rtu_out_active(tmp2);	
            break ;

        case READ_AI:      //读取模拟量输入
            tmp2=rtu_read_anloginput (tmp1,tmp3,tmp4);
            rtu_out_active(tmp2);	
            break;

        case PROTOCOL_EXCEPTION:
        return -1*PROTOCOL_ERR;   
       
        default:
        return -1*PROTOCOL_ERR;
       }
	   */
    return 0;
}


/*
//读取数据输入状态状态
int rtu_read_input_status ( unsigned char board_adr,int start_address,int lenth) 
   {
    unsigned char tmp[100], tmp_lenth;
    unsigned int temp_start;
    unsigned char temp1,temp2;
    
    tmp_lenth = lenth;
    temp_start = start_address;
    
    if(tmp_lenth<=8)  
        {
         tmp[2]=1;
         }
       else 
         {
          tmp[2]=2;
          }
          
    alert_value_mid=alert_value_mid>>temp_start; 
         
    tmp[0] = board_adr;
    tmp[1] = READ_COIL;
    temp1= alert_value_mid/256;
    temp2= alert_value_mid;
    
    if(tmp[2]==1)
         {
         tmp[3] = temp2;         //给出实际的数据1个字节，可代表8个继电器     
         tmp_lenth = 4;
         } 
     else 
         {
         tmp[3] = temp1;         //给出实际的数据1个字节，可代表8个继电器     
         tmp[4] = temp2;
         tmp_lenth = 5;
         }
    
    construct_rtu_frm (com_out_aa, tmp, tmp_lenth);
    return (tmp_lenth+2);
    }
//读取采样保持器
//读格式：本机地址+功能码+读高位地址+读地位地址+数据段长度高位+数据段长度低位+CRC校验码
//返回格式：本机地址+功能码+有效数据长度 +数据1+数据2+++++++
int rtu_read_hldreg ( unsigned char board_adr,int start_address,int lenth) 
{
    unsigned char tmp[100], tmp_lenth;
    unsigned char i,j,k;
    unsigned int temp;
    
    tmp_lenth=lenth;
    
    tmp[0] = board_adr;
    tmp[1] = READ_HLD_REG;
    tmp[2] = tmp_lenth;
    
    j=start_address;
    k=2;
    for(i=j;i<tmp_lenth;i++)
      {
       temp=uart1_value[i];
       k++;
       tmp[k] = temp >> 8 ;
       k++;
       tmp[k] = uart1_value[i];
       }
    tmp_lenth = k+1;
    construct_rtu_frm (com_out_aa, tmp, tmp_lenth);
    return (tmp_lenth+2);
}
//读取采样保持器
//读格式：本机地址+功能码+读高位地址+读地位地址+数据段长度高位+数据段长度低位+CRC校验码
//返回格式：本机地址+功能码++有效数据长度+数据1+数据2+++++++
int rtu_read_anloginput( unsigned char board_adr,int start_address,int lenth) 
{
    unsigned char tmp[100], tmp_lenth;
    unsigned char i,j,k;
    unsigned int temp;
    
    tmp_lenth=lenth;
    
    tmp[0] = board_adr;
    tmp[1] = READ_HLD_REG;
    tmp[2] = tmp_lenth;
    
    j=start_address;
    k=2;
    for(i=j;i<tmp_lenth;i++)
      {
       temp=uart1_value[i];
       k++;
       tmp[k] = temp >> 8 ;
       k++;
       tmp[k] = uart1_value[i];
       }
    tmp_lenth = k+1;
    construct_rtu_frm (com_out_aa, tmp, tmp_lenth);
    return (tmp_lenth+2);
}
*/

