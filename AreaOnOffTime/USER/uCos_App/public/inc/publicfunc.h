#ifndef				_PUBLICFUNC_H
#define				_PUBLICFUNC_H	

#include "Bsp.h"

#define MERGETWOBYTE(byt_h,byt_l)  (((uint16)(byt_h)<<8)+byt_l)
#define LOWBYTEOF(dbyte)   ((uint8)(dbyte))
#define HIGHBYTEOF(dbyte)   ((uint8)((dbyte)>>8))


#define	HEX_TO_ASCII(x)				((x<=0x09)?(x+0x30):(x+0x37))
#define UINT8_TO_ASCII_H(x)			HEX_TO_ASCII((x&0x0F))							
#define UINT8_TO_ASCII_L(x)			HEX_TO_ASCII((x&0x0F))
									

#define  NO_ERR 0 
#define  ERR_1  1 
#define  ERR_2  2 
#define  ERR_3  3 
#define  ERR_4  4 
#define  ERR_5  5 


//===============================================
extern void CopyNHword(uint16* p1,uint16* p2,uint32 n);
extern uint8  CmpNByte(uint8* No1,uint8* No2,uint32 n);
//===============================================
extern void  SetMeterNo(uint8* No1,uint8 data);
extern void  CopyMeterNo(uint8* No1,uint8* No2);
extern uint8  CmpMeterNo(uint8* No1,uint8* No2);
extern int  CmpMeterNoHL(uint8* No1,uint8* No2);
extern void  CopyEle(uint8* Ele1,uint8* Ele2);
extern uint8  CmpEle(uint8* Ele1,uint8* Ele2);
//===============================================
extern void  CopyMeterNoFrm(uint8* No1,uint8* No2);
extern uint8  CmpMeterNoFrm(uint8* No1,uint8* No2);
extern uint8  CmpEleFrm(uint8* Ele1,uint8* Ele2);
//===============================================
extern uint8 HexToBcd(uint8 hexData);
extern uint8 BcdToHex(uint8 bcdData);
extern uint16 BcdToHex_16bit(uint16 hexData);//ZXL 08.06.09添加
//===============================================
extern uint8 BcdCheck(uint8 Data);
extern uint8 BcdCheck_n(uint8 *pTemp,uint8 n);
extern uint8 MeterNoBcdCheck(uint8* No1);
//===============================================
extern uint8 IsLeap(uint8 year);
extern void CountDate (uint8* TheDate,uint8 AddDay);
extern void CountMinute(uint8 *TheDate,uint8 AddMinute);  //将分钟进行相加    注，所加分钟不能超过1小时
extern void CountMonth(uint8* TheDate);//月 年 的月加1  //zxl 08.06.08
extern void CountHour(uint8* TheDate); //时 日 月 年 中的时+1//zxl 08.06.25
extern void SubDate (uint8* TheDate); //将 日 月 年 中的日 减 1 
extern void SubMonth (uint8* TheDate); //将  月 年 中的月 减 1 
extern uint8 GetLastDayofMonth(uint8 MonthBcd,uint8 YearBcd);
extern void SubDate1(uint8* StartDate,uint8* LastDate,uint8* ReDate);//两个日期相减，得到时间差
//================================================zxl 08.04.20添加
extern uint8 AsciiToHex(uint8 Asciicode);
extern uint8 HexToAscii(uint8 Hexcode);
extern uint8 Hex_BcdToAscii(uint16 Hex_bcd_code,uint8* AscBuf);//16位的HEX或者BCD码转化为ASCII码，然后反方向存放在AscBuf中，长度为num
extern uint8 Hex_BcdToAscii_8bit(uint8 Hex_bcd_code,uint8* AscBuf);//8位的HEX或者BCD码转化为ASCII码，然后正方向存放在AscBuf中，长度为2   yb add 101101

extern uint32 AsciiToBcd_16bit(uint8* pBuf);//限制为5个字节的ASC码数组，转换为一个16位（2字节）的BCD数字，比如{30，38，30，30，31}，转换为8001
extern int8 Reversalmemcmp(uint8* p1,uint8* p2,uint8 len);//反转比较大小
extern void Reversalmemcpy(uint8 *destin, uint8 *source, uint16 n);//从源地址拷贝N个字节反向存到目的地址 
extern uint8 TimeCheck(uint8* pTime);//日期和时间检查函数，顺序为  秒-分-时-日-月-年，正确返回0，日期错误返回1，时间错误返回2
extern uint8 ASCIPToHex(uint8* pStr,uint16 *buf); //三个字节的ASC码的IP转化为一个字节的HEX
extern uint8 ConverHexToASCIP(uint8* pTemp1,uint8* pTemp2);//16进制转换为ASCII的IP，例如C0 A8 01 08 转换为192.168.1.8,返回值为转化后IP的长度
extern uint8 ConverASCIPToHex(uint8* pTemp1,uint8* pTemp2);//将ASCII码的IP地址转化为4字节16进制的数字
extern uint8 Bcd645DataToAscii(uint8* pAscii,uint8* pBcdData,uint8 n,uint8 m);//BCD数字转换为ASCII字符串，数字按照645协议的标准排列,n:m是数据形式，表示整数部分是n,小数部分
extern uint8 Bcd645DataToAsciiTest(uint8* pAscii,uint8* pBcdData,uint8 len);
extern uint16 DecToHex_16bit(uint16 Dec);//2字节的10进制转化为16进制,输入范围：0~65535
//=================================================yb 09.01.14 add
extern uint8 CmpData(uint8 *Target,uint8 *Current);                  //比较年月日的大小      如果大于目标日期则返回2，小于则返回1，等于则返回0
extern uint8 CmpDate_H(uint8 *Target,uint8 *Current);                  //比较时日月年的大小      如果大于目标日期则返回2，小于则返回1，等于则返回0   2000-2099
extern uint16 abs_value(uint16 a,uint16 b);
extern uint16 x_n_value(uint16 x,uint8 n);
extern void Remove_R4(uint8 *a,uint8 mm);
extern void Remove_L4(uint8 *a,uint8 mm);
extern uint8 JudgeAvail(uint8*pdata,uint8 bit);                       //判断位是否有效(bin format)
extern void gpHex16ToStr(uint16 x,char* str);
extern uint32 Uint32HexToBCD(uint32 mm);
extern uint32 Uint32BCDToHex(uint32 mm);
extern uint8 AddBcdByte(uint8 byte1,uint8 byte2,uint8* pCarry);//其中pCarry为指向进位的指针
extern uint8 AddBcdStr(uint8* str1,uint8* str2,uint8 len);//两个BCD串相加，结果放在第一个串中
//两个Bcd字节减法，byte2-byte1,其中pCarry为指向借位的指针
extern uint8 SubBcdByte(uint8 byte1,uint8 byte2,uint8* pCarry);
extern uint8 SubBcdStr(uint8* str1,uint8* str2,uint8 len);//两个BCD串相减，结果放在第一个串中
//两个BCD串相加，结果放在第一个串中,其中最高位为符号位
extern uint8 AddBcdStrWithSign(uint8* str1,uint8* str2,uint8 len);
//两个BCD串相减，str2-str1 结果放在第一个串中,其中最高位为符号位
extern uint8 SubBcdStrWithSign(uint8* str1,uint8* str2,uint8 len);
extern uint8 CmpStr(uint8 *str1, uint8 *str2, uint8 len);

extern int8 CmpStrAbsolute(uint8 *str1, uint8 *str2, uint8 len);

extern uint8 CmpStr1(uint8 *str1, uint8 *str2, uint8 len);
/////////////////////////////////////////////////////////////////////////////////
//数据格式调整
extern uint8 CountFormatA2(uint8 pData[4],uint8 decNum); 
void CountV_645_97(uint8* pData);//调整97表电压为A.7
void CountI_645_97(uint8* pI1,uint8* pI2);//将97回来的数据pI1调整为376。1数据格式25 pI2

//==================================================================================================================================
//===================================================针对终端越限部分做的函数=======================================================
extern int8 CmpMeVolata(uint8 *data,int32 Vol);//  得到电压的比较值
extern int8 CmpMeElcurrent(uint8 *data,int32 Elc);// 得到电流的比较值

extern uint8 CheckDT(uint8* DTBcd);
extern uint8 Zeller(uint8 year,uint8 month,uint8 day);
extern void Zeller_DTBcd(uint8* DTBcd);
extern uint8 CmpDateTime(uint8* pTarget, uint8* pCurrent, uint8 CmpFlag);
extern uint8 GetChannelType(void);//得到当前通道类型
extern uint8 GetChannelTypeFromFlash(void);//从flash中读取当前通道类型，因为在全局变量初始化之前，无法使用函数GetChannelType
extern    int CmpCurTime(uint8* pTime); 
extern     uint8 JudgeSameDayMonthAndYear(uint8 *CurrentDay,uint8* AskDate);   //判断日是不是同一天
extern     uint8 JudgeSameMonth(uint8 *CurrentMonth,uint8* AskDate);
extern     uint8 JudgeSameMonthAndYear(uint8 *CurrentMonth,uint8* AskDate);   //判断月是不是同一天
extern     uint8 JudgeSameMonthAndYear_1(uint8 *CurrentValue, uint8 *AskValue);  
extern uint8 JudgeSameDayMonthAndYear2(uint8 *CurrentDay,uint8* AskDate);
extern uint8 CmpData_M(uint8 *Target,uint8 *Current); 
extern  void GotoNextTime(uint8 time[5],uint8 FreezeInt);
extern void AdjectStartTime(uint8 time[5],uint8 FreezeInt);
extern uint8 JudgeMeterZero(uint8 *pdata,uint8 flag);
extern uint8 PUBLIC_CountCS(uint8* _data, uint16 _len);
extern char* PUBLIC_HexStreamToString(uint8 *pIn, uint16 InLen, char *pOut);
extern char* PUBLIC_MeterAddrToString(uint8 *pMeterAddrIn, char *pMeterOut,int length);

extern uint16 HexToBcd2bit(uint8 hexData);
//extern uint16 HexToBcd1(uint8 hexData);
extern uint32 HexToBcd4bit(uint16 hexData);

extern uint32 BcdToHex_16bit1(uint32 mm); //duan kou hao 
extern uint8 Uint16BCDToHex1(uint16 mm);

/*yangfei added */
uint32 HexToBcdUint32(uint32 hexData);
uint64  HexToBcdUint64(uint32 hexData); 

#endif
