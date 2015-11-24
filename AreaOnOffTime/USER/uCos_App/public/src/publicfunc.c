

#include <includes.h>


//==============================================
void CopyNHword(uint16* p1,uint16* p2,uint32 n)
{
	uint32 i;
	for(i=0;i<n;i++){
		p1[i]=p2[i];
	}
}
//===============================================
void  SetMeterNo(uint8* No1,uint8 data)
{
	memset(No1,data,6);
}
void  CopyMeterNo(uint8* No1,uint8* No2)
{
	memcpy(No1,No2,6);
}
uint8  CmpMeterNo(uint8* Source,uint8* Dest)
{
	uint8 No1[6] = {0x00};
	uint8 No2[6] = {0x00};
	
	memcpy(No1, Source, 6);
	memcpy(No2, Dest, 6);

	if(memcmp(No1,No2,6)==0){
		return NO_ERR;
	}
	return 1;
}
int CmpMeterNoHL(uint8* No1,uint8* No2)
{
  if(memcmp(No1,No2,6)>0) return 1;  //No1>No2,返回1
  if(memcmp(No1,No2,6)==0) return 0;//No1=No2,返回0
   else  return -1; //No1<No2,返回-1
} 
uint8 JudgeMeterZero(uint8 *pdata,uint8 flag)
{
  uint8 i;
  for(i =0;i<6;i++)
    if(pdata[i] != flag)  return 1;
  return 0;  
}


void  CopyEle(uint8* Ele1,uint8* Ele2)
{
	memcpy(Ele1,Ele2,4);
}
uint8  CmpEle(uint8* Ele1,uint8* Ele2)
{
	if(memcmp(Ele1,Ele2,4)==0){
		return TRUE;
	}
	return FALSE;
}

void  CopyMeterNoFrm(uint8* No1,uint8* No2)
{
	uint8 i;
	for(i=0;i<6;i++){
		No1[i]=No2[i];
	}
}

uint8  CmpNByte(uint8* No1,uint8* No2,uint32 n)
{
	uint32 i;
	
	for(i=0;i<n;i++){
		if(No1[i]!=No2[i]){
			return FALSE;
		}
	}
	
	return TRUE;
}

uint8  CmpMeterNoFrm(uint8* No1,uint8* No2)
{
	return CmpNByte(No1,No2,6);
}
uint8  CmpEleFrm(uint8* Ele1,uint8* Ele2)
{
	return CmpNByte(Ele1,Ele2,4);
}
//===============================================
uint8 HexToBcd(uint8 hexData)
{
	uint8 tmpData;
	tmpData = (hexData/0x0A)*0x10+(hexData%0x0A);
	return tmpData;	
}

/************************************************
*文件名：HexToBcd2bit
*功能：	 2位16进制转换为BCD码
*入口参数：uint8 hexData--2位16进制数
*出口参数：tmpData-- BCD码
*************************************************/
uint16 HexToBcd2bit(uint8 hexData)  
{
	uint16 tmpData,h,f;
	uint16 ten,individual;
	ten        = hexData/0x0A;	
	individual = hexData%0x0A;
	h = HexToBcd( ten );
	f = HexToBcd( individual );
   	tmpData = h*0x10+f;
	return tmpData;	
}
/************************************************
*文件名：HexToBcd4bit
*功能：	 4位16进制转换为BCD码
*入口参数：uint16 hexData--4位16进制数
*出口参数：tmpData-- BCD码
*************************************************/
uint32 HexToBcd4bit(uint16 hexData) 
{
	uint16 myriad,kilobit,hundred,ten,individual;
	uint32 tmpData ;
	individual =  hexData % 0x0A;					   
	ten     =  ( hexData / 0x0A )%0x0A; 			   
	hundred = (( hexData / 0x0A )/0x0A)%0x0A ;	       
	kilobit = ((( hexData / 0x0A )/0x0A)/0x0A)%0x0A;    
	myriad  =  ((( hexData / 0x0A )/0x0A)/0x0A)/0x0A;  
	tmpData = myriad*0x10000 + kilobit*0x1000 + hundred*0x100 + ten*0x10 + individual*0x1 ;	 
	return 	tmpData;
}

/************************************************
*文件名：HexToBcd4bit
*功能：	 4位16进制转换为BCD码
*入口参数：uint16 hexData--4位16进制数
*出口参数：tmpData-- BCD码
yangfei 
*************************************************/
uint32 HexToBcdUint32(uint32 hexData) 
{
	uint32 tmpData =0;
    tmpData = HexToBcd(hexData%100);
    tmpData += HexToBcd(hexData%10000/100)<<8;
    tmpData += HexToBcd(hexData%1000000/10000)<<16;
    tmpData += HexToBcd(hexData/1000000)<<24;
	return 	tmpData;
}
/************************************************
*文件名：HexToBcd4bit
*功能：	 4位16进制转换为BCD码
*入口参数：uint16 hexData--4位16进制数
*出口参数：tmpData-- BCD码
yangfei 
*************************************************/
uint64  HexToBcdUint64(uint32 hexData) 
{
	uint64 tmpData =0;
    tmpData = HexToBcd(hexData%100);
    tmpData += HexToBcd(hexData%10000/100)<<8;
    tmpData += HexToBcd(hexData%1000000/10000)<<16;
    tmpData += HexToBcd(hexData%100000000/1000000)<<24;
    //tmpData += HexToBcd(hexData/100000000);
    tmpData += HexToBcd(hexData/100000000)*0x100000000;
	return 	tmpData;
}

uint8 BcdToHex(uint8 bcdData)
{
	uint8 tmpData;
	tmpData = (bcdData/0x10)*0x0A+(bcdData%0x10);
	return tmpData;	
}

uint32 BcdToHex_16bit1(uint32 mm) //duan kou hao 
{
	 uint32 tmpData =0;
	 uint32 temp =0;  
	 tmpData = ((uint8)(mm)/0x10)*0x0A + (uint8)(mm)%0x10;
	 temp += tmpData;
	 tmpData = ((uint8)(mm>>8)/0x10)*0x0A + (uint8)(mm>>8)%0x10;
	 temp += tmpData*100;
	 tmpData = ((uint8)(mm>>16)/0x10)*0x0A + (uint8)(mm>>16)%0x10;
	 temp += tmpData*10000;
	 tmpData = ((uint8)(mm>>24)/0x10)*0x0A + (uint8)(mm>>24)%0x10;
	 temp += tmpData*1000000;
	 
	 return temp;
}

uint8 Uint16BCDToHex1(uint16 mm)  // ip diao yong 
{
 uint32 tmpData =0;
 uint32 temp =0;  
 tmpData = ((uint8)(mm)/0x10)*0x0A + (uint8)(mm)%0x10;
 temp += tmpData;
 tmpData = ((uint8)(mm>>8)/0x10)*0x0A + (uint8)(mm>>8)%0x10;
 temp += tmpData*100;
 tmpData = ((uint8)(mm>>16)/0x10)*0x0A + (uint8)(mm>>16)%0x10;
 temp += tmpData*10000;
 tmpData = ((uint8)(mm>>24)/0x10)*0x0A + (uint8)(mm>>24)%0x10;
 temp += tmpData*1000000;
 
 return temp;
}

//===============================================
uint8 BcdCheck(uint8 Data)
{
	if((Data&0x0f)>0x09){//注意 ！(Data&0x0f) 必须有括号！！！
		return FALSE;
	}
	if((Data&0xf0)>0x90){//注意 ！(Data&0xf0) 必须有括号！！！
		return FALSE;
	}
	return TRUE;
}
uint8 MeterNoBcdCheck(uint8* No1)
{
	uint8 i;
	if(No1[0]==0x00 && No1[1]==0x00 && No1[2]==0x00 && No1[3]==0x00 && No1[4]==0x00 && No1[5]==0x00 && No1[6]==0x00) return FALSE;
	for(i=0;i<7;i++){
		if(No1[i]==0xff){
			return (FALSE);
		}
	}
	return (TRUE);//将原来的BCD码判断改为0xff判断
}

//===============================================
uint8 IsLeap(uint8 year)
{
	return ((year % 4 == 0 && year%100 != 0) || year % 400 == 0);
}
void CountDate (uint8* TheDate,uint8 AddDay)
{
	uint8 year1;
	uint8 month1;
	uint8 day1;
	uint8 days[13];
	
	year1 = TheDate[2];
	month1 = TheDate[1];
	day1 = TheDate[0];
	
	year1 = BcdToHex(year1);
	month1 = BcdToHex(month1);
	day1 = BcdToHex(day1);
	
	days[0] = 0;
	days[1] = 31;
	days[2] = 28;
	days[3] = 31;
	days[4] = 30;
	days[5] = 31;
	days[6] = 30;
	days[7] = 31;
	days[8] = 31;
	days[9] = 30;
	days[10] = 31;
	days[11] = 30;
	days[12] = 31;
		
	if( (!IsLeap(year1) || month1!= 2))
	{
		if ((day1+AddDay) <= days[month1] )
		{
			TheDate[0]= HexToBcd(day1+AddDay);
			TheDate[1]=HexToBcd(month1);
			TheDate[2] = HexToBcd(year1);
		}
		else
		{
			TheDate[0] = HexToBcd(day1+AddDay - days[month1]);
			if (month1==12)
			{
				TheDate[2] = HexToBcd(year1+1);
				TheDate[1] =1;
			}
			else
			{
				TheDate[2] = HexToBcd(year1);
				TheDate[1] =HexToBcd(month1+1);
			}
		}
    
	}
	else
	{
		if ((day1+AddDay) < 29)
		{
			TheDate[0]= HexToBcd(day1+AddDay);
			TheDate[1]=2;
			TheDate[2] = HexToBcd(year1);
		}
		else
		{
			TheDate[0] =HexToBcd(day1+AddDay - 29);
			TheDate[1] = 3;
			TheDate[2] = HexToBcd(year1);
		}
	}
}
void CountMonth(uint8* TheDate)//月 年 的月加1 //ZXL 08.06.08添加
{
	uint8 year1;
	uint8 month1;
	
	year1 = TheDate[1];
	month1 = TheDate[0];

	year1 = BcdToHex(year1);
	month1 = BcdToHex(month1);

	if(month1!=12){
		TheDate[0]=HexToBcd(month1+1);
		
	}
	else{
		TheDate[0]=HexToBcd(1);
		TheDate[1]=HexToBcd(year1+1); //这里不处理00年
	}
}

void CountHour(uint8* TheDate) //时 日 月 年 中的时+1
{
	uint8 year1;
	uint8 month1;
	uint8 day1;
	uint8 hour1;
	//uint8 days[13];
	
	year1 = TheDate[3];
	month1 = TheDate[2];
	day1 = TheDate[1];
	hour1 = TheDate[0];
	
	year1 = BcdToHex(year1);
	month1 = BcdToHex(month1);
	day1 = BcdToHex(day1);
	hour1 = BcdToHex(hour1);
	
	if(hour1 != 23)
	{
		TheDate[0]=HexToBcd(hour1+1);
	}
	else
	{
		CountDate(&TheDate[1],1);
		TheDate[0]=0x00;
	}
}

void SubDate (uint8* TheDate) //将 日 月 年 中的日 减 1 
{
	uint8 year1;
	uint8 month1;
	uint8 day1;
	uint8 days[13];
	
	year1 = TheDate[2];
	month1 = TheDate[1];
	day1 = TheDate[0];
	
	year1 = BcdToHex(year1);
	month1 = BcdToHex(month1);
	day1 = BcdToHex(day1);
	
	days[0] = 0;
	days[1] = 31;
	days[2] = 28;
	days[3] = 31;
	days[4] = 30;
	days[5] = 31;
	days[6] = 30;
	days[7] = 31;
	days[8] = 31;
	days[9] = 30;
	days[10] = 31;
	days[11] = 30;
	days[12] = 31;
		
	if( (!IsLeap(year1) || month1!= 3))
	{
		if (day1>1)
		{
			TheDate[0]= HexToBcd(day1-1);
			TheDate[1]=HexToBcd(month1);
			TheDate[2] = HexToBcd(year1);
		}
		else
		{
			
			if (month1==1)
			{
				TheDate[2] = HexToBcd(year1-1); //这里不处理00年问题
				month1=12;
				TheDate[1] =HexToBcd(month1);
			}
			else
			{
				TheDate[2] = HexToBcd(year1);
				month1=month1-1;
				TheDate[1] =HexToBcd(month1);
			}
			TheDate[0] = HexToBcd(days[month1]);
			
		}
    
	}
	else
	{
		if (day1>1)
		{
			TheDate[0]= HexToBcd(day1-1);
			TheDate[1]=3;//HexToBcd
			TheDate[2] = HexToBcd(year1);
		}
		else
		{
			TheDate[0] =HexToBcd(29);
			TheDate[1] = 2;//HexToBcd
			TheDate[2] = HexToBcd(year1);
		}
	}

}
void SubMonth (uint8* TheDate) //将  月 年 中的月 减 1 
{
	uint8 year1;
	uint8 month1;
	
	year1 = TheDate[1];
	month1 = TheDate[0];

	year1 = BcdToHex(year1);
	month1 = BcdToHex(month1);

	if(month1!=1){
		TheDate[0]=HexToBcd(month1-1);
		
	}
	else{
		TheDate[0]=HexToBcd(12);
		TheDate[1]=HexToBcd(year1-1); //这里不处理00年
	}

}
uint8 GetLastDayofMonth(uint8 MonthBcd,uint8 YearBcd) 
{
	uint8 year1,month1;	
	uint8 days[13];
	year1 = BcdToHex(YearBcd);
	month1 = BcdToHex(MonthBcd);
    days[0] = 0;
	days[1] = 31;
	days[2] = 28;
	days[3] = 31;
	days[4] = 30;
	days[5] = 31;
	days[6] = 30;
	days[7] = 31;
	days[8] = 31;
	days[9] = 30;
	days[10] = 31;
	days[11] = 30;
	days[12] = 31;
    if( (!IsLeap(year1) || month1!= 2)){
    	return (HexToBcd(days[month1]));
    
    }
    else{
    	return (0x29); 
    }
}

void CountMinute(uint8 *TheDate,uint8 AddMinute)
{
    uint8 year1;
	uint8 month1;
	uint8 day1;
	uint8 hour1;
	uint8 minute1,minute;
	year1 = TheDate[4];
	month1 = TheDate[3];
	day1 = TheDate[2];
	hour1 = TheDate[1];
	minute1 = TheDate[0];
	
	year1 = BcdToHex(year1);
	month1 = BcdToHex(month1);
	day1 = BcdToHex(day1);
	hour1 = BcdToHex(hour1);
	minute1 = BcdToHex(minute1);
	
	minute =minute1 +AddMinute;
	if(minute < 60)
	{
		TheDate[0]=HexToBcd(minute);
	}
	else
	{
        CountHour(&TheDate[1]);
        TheDate[0] =0;
	}
}

/************************************************
*文件名：
*功能：两个日期相减，得到时间差
*入口参数：StartDate：开始时间，格式为：秒分时月年
           LastDate：结束时间，格式为：秒分时月年
*出口参数：ReDate：返回时间差，格式为：秒分时月年
*************************************************/
void SubDate1(uint8* StartDate,uint8* LastDate,uint8* ReDate)
{
	uint8 i,result;
	uint8 TempLastDate[6];
	uint8 TempStartDate[6];
	result = FALSE;
	//先判断StartDate和LastDate的大小，若LastDate小于StartDate，则返回时差ReDate全为0
	for(i=6;i>0;i--)
	{
		if(LastDate[i-1]>StartDate[i-1])
		{
			result = TRUE;	
			break;
		}
	}
	if(result == FALSE) 
	{
		for(i=0;i<6;i++)
		{
			ReDate[i] = 0;
		}
		return;
	}
	
	memcpy(TempStartDate,StartDate,6);
	memcpy(TempLastDate,LastDate,6);
	for(i=0;i<6;i++)
	{
		TempStartDate[i] = BcdToHex(TempStartDate[i]);
		TempLastDate[i] = BcdToHex(TempLastDate[i]);
	}
  //秒减	
	if(TempLastDate[0] < TempStartDate[0])
	{
		TempLastDate[1] = TempLastDate[1] - 1;
		TempLastDate[0] = TempLastDate[0] + 60;
	}
	ReDate[0] = HexToBcd(TempLastDate[0] -TempStartDate[0]);
 //	分减
	if(TempLastDate[1] < TempStartDate[1])
	{
		TempLastDate[2] = TempLastDate[2] - 1;
		TempLastDate[1] = TempLastDate[1] + 60;
	}
	ReDate[1] = HexToBcd(TempLastDate[1] -TempStartDate[1]);
//时减	
	if(TempLastDate[2] < TempStartDate[2])
	{
		SubDate(&TempLastDate[3]); //将日减1
		TempLastDate[2] = TempLastDate[2] + 24;
	}
	ReDate[2] = HexToBcd(TempLastDate[2] -TempStartDate[2]);
	
 //日减
	if(TempLastDate[3] < TempStartDate[3])
	{
		SubMonth(&TempLastDate[4]);
		TempLastDate[3] = TempLastDate[3] + BcdToHex(GetLastDayofMonth(TempLastDate[4],TempLastDate[5]));
	}
	ReDate[3] = HexToBcd(TempLastDate[3] -TempStartDate[3]);
	
 //月减
	if(TempLastDate[4] < TempStartDate[4])
	{
		TempLastDate[5] = TempLastDate[5] - 1;
		TempLastDate[4] = TempLastDate[4] + 12;
	}
	ReDate[4] = HexToBcd(TempLastDate[4] -TempStartDate[4]);
//年减
	ReDate[5] = HexToBcd(TempLastDate[5] -TempStartDate[5]);

	return;
}

//ZXL 08.04.20 添加 ASCII和HEX 相互转换
 uint8 AsciiToHex(uint8 Asciicode)
{
     uint8 result = 0;
 	 if((Asciicode<'9'||Asciicode=='9')&&(Asciicode>'0'||Asciicode=='0'))
 	     result = Asciicode - 0x30;
 	 // return  (Asciicode - 0x30);
 	else if((Asciicode<'F'||Asciicode=='F')&&(Asciicode>'A'||Asciicode=='A'))
 	     result = Asciicode - 0x37;
 	 //return (Asciicode - 0x37);
 	else if((Asciicode<'f'||Asciicode=='f')&&(Asciicode>'a'||Asciicode=='a'))
	     result = Asciicode - 0x57;
	 //return (Asciicode - 0x57);
   return result;
}
 uint8  HexToAscii(uint8 Hexcode)
{
 	uint8 result = 0;
 	if ((Hexcode<9||Hexcode == 9)&&(Hexcode>0||Hexcode == 0))
 	   result = Hexcode + 0x30; 
 	//return (Hexcode + 0x30);
 	if((Hexcode<0x0F||Hexcode == 0x0F)&&(Hexcode>0x0A||Hexcode == 0x0A))
 	   result = Hexcode + 0x37;
 	//return (Hexcode + 37);
  return result;
}

uint8	Hex_BcdToAscii(uint16 Hex_bcd_code,uint8* AscBuf)//16位的HEX或者BCD码转化为ASCII码，然后反方向存放在AscBuf中，长度为num
{
	uint8  num = 0;
	//uint8  TempBuf;
	uint16 b1,b2;
	b2 = Hex_bcd_code;
	if(b2==0)//Hex_bcd_code为0的情况
	{
		AscBuf[num] = 0x30;
		num = 1;
	}
	while(b2)
	{
		b1 = b2%10;
		AscBuf[num] = HexToAscii(b1);
		b2 = b2/10;
		num++;
	}
	return num;
}

uint8	Hex_BcdToAscii_8bit(uint8 Hex_bcd_code,uint8* AscBuf)//8位的HEX或者BCD码转化为ASCII码，然后正方向存放在AscBuf中，长度为2
{
	uint8  num = 2;
	uint8 b1,b2;
	
	b2 = Hex_bcd_code;
	while(num>0)
	{
	    num--;
		b1 = b2%16;
		AscBuf[num] = HexToAscii(b1);
		b2 = b2/16;
		
	}
	return 2;
}

//ZXL 08.09.28添加
uint32 AsciiToBcd_16bit(uint8* pBuf)//限制为5个字节的ASC码数组，转换为一个16位（2字节）的BCD数字，比如{30，38，30，30，31}，转换为8001
{//可以用做端口号的转换，或者BCD方式输入的终端地址的转换
	uint8 i;
	uint32 result = 0;
	uint8  tempbuf[5];
	for(i=0;i<5;i++)
	{
		tempbuf[i] = AsciiToHex(pBuf[i]);
	}
	result += tempbuf[4];
	result += tempbuf[3]*10; 
	result += tempbuf[2]*100;
	result += tempbuf[1]*1000;
	result += tempbuf[0]*10000;
	return result;
}

int8 Reversalmemcmp(uint8* p1,uint8* p2,uint8 len)//反转比较大小
{   
    uint8 i;
    int8 flag = 0;
   
    for(i=len;i>0;i--)
    {
    	if(p1[i-1]>p2[i-1])  
    	{
    	 flag = 1;
    	 break;
    	}
    	if(p1[i-1]<p2[i-1])
    	{
    	 flag = -1;
    	 break;
    	}
    }	
	return flag;
}

void Reversalmemcpy(uint8 *destin, uint8 *source, uint16 n)
{
	uint16 i;
	for(i = 0;i < n;i++)
	{
	    destin[i] = source[n-i-1];
	}
}

uint8 TimeCheck(uint8* pTime)//日期和时间检查函数，顺序为  秒-分-时-日-月-年，正确返回0，日期错误返回1，时间错误返回2
{
	uint8 year,month,day;
	
	year = BcdToHex(pTime[5]);
	month = BcdToHex(pTime[4]&0x1f);
	day = BcdToHex(pTime[3]);
	
	if(pTime[0]>0x59||pTime[1]>0x59||pTime[2]>0x23)//秒分时  时间的判断
	{
		return 2;
	}
	//日期的判断
	else if(month>12)//月份的判断
		 {
		 	return 1;
		 }
 	
	
 	if(month==2)//日的判断
 	{   
 		if(IsLeap(year))
 		{
 			if(day>29||day==0) return 1;
 		}
 		else
 		{
 			if(day>28||day==0) return 1;
 		}
 	}
 	else if(month == 4||month == 6||month == 9||month == 11)
	     {
			if(day>30||day==0)	return 1;
		 }
		 else
		 {
		 	if(day>31||day==0)	return 1;
		 }
    return 0;
}


/****************************************************
  入口参数：pStr：要转换的区域首指针
            buf:  要转换的长度，范围为1到3
  出口参数：buf:  转后的HEX数据存放
*****************************************************/            
uint8 ASCIPToHex(uint8* pStr,uint16 *pbuf)
{  // uint8 i,j;
	uint16 buf;
	buf = *pbuf;
	switch(buf)
		   {
		   	   case 1:
		   	     buf = AsciiToHex(pStr[0]);
		   	     break;
		   	   case 2:
		   	     buf = AsciiToHex(pStr[1]) ;
		   	     buf = buf + (AsciiToHex(pStr[0])*10);
		   	     break;
		   	   case 3:
		   	     buf = AsciiToHex(pStr[2]);
		   	     buf = buf + (AsciiToHex(pStr[1])*10);
		   	     buf = buf + (AsciiToHex(pStr[0])*100);
		         break;
		       case 0x0F:
		         return TRUE;
		       default:
		         return FALSE;
		   }
	 *pbuf = buf;
	 return TRUE;
}
uint8 ASCIPToHex1(uint16 *ss)
{  
	*ss += 100;
	return TRUE;
}

uint8  ConverASCIPToHex(uint8* pTemp1,uint8* pTemp2)//从数组第一个开始，正序
{
	uint16 b1 = 0;
	//uint8 b2;
	uint8 i,j,result;
	i = 0;
	j = 0;
	
	do
	{
		if((pTemp1[i]>'9'&&(pTemp1[i]!='.'&&pTemp1[i]!=0x00))||(pTemp1[i]<'0'&&(pTemp1[i]!='.'&&pTemp1[i]!=0x00)))  return FALSE;
		
		if((pTemp1[i] == '.') || (pTemp1[i] == 0x00))//0X2E = '.'//ZXL 08.06.18改0x0D为0x00;
		{ 
		   b1 = i - j;
		   result = ASCIPToHex(&pTemp1[j],&b1);
		   if(result == FALSE)   return  FALSE;
		   
		   if(b1>255)  return FALSE;
		   *pTemp2++ = (uint8)b1;
		   j = i + 1;
		}
		i++;
	}while(pTemp1[i-1] != 0x00);//ZXL 08.06.18改0x0D为0x00;
	
    return TRUE;
}
uint8 ConverHexToASCIP(uint8* pTemp1,uint8* pTemp2)
{
	uint8 a,i,num,len;
	uint8 TempBuf[3];
	len =0;
	for(i=0;i<=3;i++)
	{
	    a = pTemp1[3-i];
	    num = Hex_BcdToAscii((uint16)a,TempBuf);
	    Reversalmemcpy(pTemp2,TempBuf,num);
	    pTemp2 += num;
	    len += num;
	    if(i==3)  *pTemp2++ = 0x00;
	    else  *pTemp2++ = '.';//0x2E
	}
	
	return (len + 3);
}

//BCD数字转换为ASCII字符串，数字按照645协议的标准排列,n:m是数据形式，表示整数部分是n,小数部分是m,返回值为ASCII字符串的长度
uint8 Bcd645DataToAscii(uint8* pAscii,uint8* pBcdData,uint8 n,uint8 m)
{
	uint8 len;
	uint8 data;
	uint8 tempBuff[32];
	uint8 i = 0,j = 0;
	
	len = (n+m)/2;
	while(len>0)
	{
		data = pBcdData[len-1];
		tempBuff[j++] = (data>>4)&0x0F;
		tempBuff[j++] = data&0x0F;
		len--;
	}
	j = 0;
	while(i<n && tempBuff[i]==0)
	{
		i++;
	}
	
	if(i==n)
	{
		pAscii[j++] = 0x30;
	}
	
			
	for(;i<n;i++)
	{
		pAscii[j++] = tempBuff[i] + 0x30;
	}
	if(m!=0)
	{
		pAscii[j++] = 0x2e;
		for(;i<n+m;i++)
		{
			pAscii[j++] = tempBuff[i] + 0x30;
		}

	}
		return j;
}
uint8 Bcd645DataToAsciiTest(uint8* pAscii,uint8* pBcdData,uint8 length)
{
	uint8 len;
	uint8 data;
	uint8 tempBuff[32];
	uint8 i = 0,j = 0;
	len = length;
	
	while(len>0)
	{
		data = pBcdData[len-1];
		tempBuff[j++] = (data>>4)&0x0F;
		tempBuff[j++] = data&0x0F;
		len--;
	}
	j = 0;
				
	for(;i<length*2;i++)
	{
		pAscii[j++] = tempBuff[i] + 0x30;
	}
	return j;
}
uint8 DecToHex(uint8 Dec)//一字节十进制转换成16进制,输入范围：0~15
{
	uint8 hexdata;
	Dec = Dec % 16;
	switch(Dec)
	{
		case 10:
			hexdata = 0x0a;
			break;
		case 11:
			hexdata = 0x0b;
			break;
		case 12:
			hexdata = 0x0c;
			break;
		case 13:
			hexdata = 0x0d;
			break;
		case 14:
			hexdata = 0x0e;
			break;
		case 15:
			hexdata = 0x0f;
			break;
		default:
			hexdata = Dec;
			break;										
	}
	return hexdata;
}

uint16 DecToHex_16bit(uint16 Dec)//2字节的10进制转化为16进制,输入范围：0~65535
{
	uint16 hexdata;
	uint8 i;
	uint8 hexBuff[4] = {0,0,0,0};
	Dec = Dec % 0xffff;//防止大于65535
	for(i=4;i>0;i--)
	{
		hexBuff[i-1] = DecToHex(Dec%0x10);
		Dec = Dec / 0x10;
	}
	hexdata = (hexBuff[0]*0x10+hexBuff[1])*0x100 + (hexBuff[2]*0x10+hexBuff[3]);
	return hexdata;
}
//********************************************************************************************
uint8 CmpData(uint8 *Target,uint8 *Current)                  //比较日月年的大小      如果大于目标日期则返回2，小于则返回1，等于则返回0   2000-2099
{
   if(Current[2] > Target[2]) return 2;
   else if(Current[2] == Target[2])
   {
     if(Current[1] > Target[1]) return 2;
     else if(Current[1] == Target[1])
     {
        if(Current[0] > Target[0]) return 2;
        else if(Current[0] == Target[0])
             return  0;
        else return 1; 
     }
     else return 1; 
   }
   else return 1;
   
}

//********************************************************************************************
uint8 CmpDate_H(uint8 *Target,uint8 *Current)                  //比较时日月年的大小      如果大于目标日期则返回2，小于则返回1，等于则返回0   2000-2099
{
   if(Current[3] > Target[3]) return 2;
   else if(Current[3] == Target[3])
   {
     if(Current[2] > Target[2]) return 2;
     else if(Current[2] == Target[2])
     {
        if(Current[1] > Target[1]) return 2;
        else if(Current[1] == Target[1])
         {      
            if(Current[0] > Target[0]) return 2;
            else if(Current[0] == Target[0])  
                 return  0;
            else return 1;               
         }      
        else return 1; 
     }
     else return 1; 
   }
   else return 1;
   
}
//***************************取两数相减的绝对值************************************************
uint16 abs_value(uint16 a,uint16 b)
{
  if((a-b)>0)  return (a-b);
  else return (b-a);
}  
//***************************取两数相减的绝对值************************************************

//***************************求取X的n次方******************************************************
uint16 x_n_value(uint16 x,uint8 n)
{
uint8 i,num;
num = 1;
for(i=0;i<n;i++)
 {
   num *= x;
 }
 return num;
}
//**************************求取X的n次方*******************************************************
//**************************mm字节的数向右移动移动4位********************************************
void Remove_R4(uint8 *pTemp,uint8 mm)
{
  uint8 a,b,c,i;
  a =pTemp[mm-1];
  for(i =(mm-1);i>0;i--)
  {
   a = a<<4;
   b =pTemp[i-1];
   c =b&0xF0;
   pTemp[i] = a|(c>>4);
   a = b;
  }  
  pTemp[i] = b<<4;
}
//**************************mm字节的数向右移动4位********************************************

//**************************mm字节的数向左移动4位********************************************
void Remove_L4(uint8 *pTemp,uint8 mm)
{
  uint8 a,b,c,i;
  a =pTemp[0];
  for(i =1;i<mm;i++)
  {
   a = a>>4;
   b =pTemp[i];
   c =b&0x0F;
   pTemp[i-1] = a|(c<<4);
   a = b;
  }  
  pTemp[i-1] = b>>4;
}
//**************************mm字节的数向左移动4位********************************************
//===================

////判断位是否有效(bin format)
uint8 JudgeAvail(uint8 *pdata,uint8 bit)
{
  uint8 result;
  uint8 i,j;
  i = (bit-1)/8;
  j = (bit-1)%8;
  j = (1<<j);
  result = pdata[i] & j;
  return result;
}

uint8 BcdCheck_n(uint8 *pTemp,uint8 n)
{
    uint8 i;
	for(i=0;i<n;i++)
	{
		if(BcdCheck(pTemp[i])==FALSE)
		{
			return (FALSE);
		}
	}
	return (TRUE);//表号六个字节全为合法BCD码
}

uint8 gpHexToAscii(uint8 x)
{
	if(x<0x0a){
		return(x+'0');
	}
	else{
		return(x-0x0a+'A');
	}
}
void gpHex16ToStr(uint16 x,char* str)
{
	uint8 i,n;
	uint8 buf[10];
	n=0;
	while(x){
		buf[n++]=gpHexToAscii(x%10);
		x=x/10;
	}
	
	//反序
	for(i=0;i<n;i++){
		str[i]=buf[n-1-i];
	}
	str[i]='\0';
}

uint32 Uint32HexToBCD(uint32 mm)
{
   uint32 i;
   uint32 a,Temp;
   a = mm;
   Temp =10000000;
   mm =0;
   for(i=7;i>0;i--)
   {
    mm |= ((a/Temp)<<4*i);
    a -= (a/Temp)*Temp;
    Temp /=10;
   }
   mm |= a/Temp; 
  return mm;   
}
uint32 Uint32BCDToHex(uint32 mm)
{
 uint32 tmpData =0;
 uint32 temp =0;  
 tmpData = ((uint8)(mm)/0x10)*0x0A + (uint8)(mm)%0x10;
 temp += tmpData;
 tmpData = ((uint8)(mm>>8)/0x10)*0x0A + (uint8)(mm>>8)%0x10;
 temp += tmpData*100;
 tmpData = ((uint8)(mm>>16)/0x10)*0x0A + (uint8)(mm>>16)%0x10;
 temp += tmpData*10000;
 tmpData = ((uint8)(mm>>24)/0x10)*0x0A + (uint8)(mm>>24)%0x10;
 temp += tmpData*1000000;
 
 return temp;
}
//=================
//==================================================================================================================================
//===================================================针对终端越限部分做的函数=======================================================
/*
int8 CmpMeVolata(uint8 *data,uint16 Vol)  //  得到电压
{ 
  uint16 tempVol =0;
  tempVol = (data[1]<<8); 
  tempVol += data[0];
  if(tempVol> Vol)  return 1;
  else if(tempVol == Vol) return 0;
  else return (-1);
}*/
int8 CmpMeElcurrent(uint8 *data,int32 Elc)
{
   int32 ElcValue;
   if(Elc<0) Elc= -Elc;
    data[2] = data[2] & 0x7f;
   ElcValue = data[0] + (data[1]<<8) + data[2]<<16;
   if(ElcValue>Elc)  return 1;
   else if(ElcValue == Elc) return 0;
   else return (-1);
}
uint8 CountFormatA2(uint8 pData[4],uint8 decNum)//计算数据格式A2
{
    uint8 Temp[8];
    uint8* pTemp = Temp;
    uint8 sign,b,G,n,i,len; 
    for(i=0;i<4;i++)
    {
    	pTemp[2*i] = (pData[3-i]>>4) & 0x0F;
    	pTemp[2*i+1] = pData[3-i] & 0x0F;
    }
  	sign = pData[3] & 0x80;
	b=*pTemp++;
	i=0;
	len = 8-decNum;
	while(!b & (i<len))
    {
    	b=*pTemp++;
    	i++;
    }
    n = 8-decNum-i;//指数
    if(n>5) return 1;
	G = 4-n+3;//4-n为最高为的位置，其中3等于结果数据长度
	G<<=5;
	if(sign) G |= 0x10;
	pData[1] = (G | Temp[i]) | sign;
	pData[0] = (Temp[i+1]<<4) | Temp[i+2];
	return 0;
}
uint8 AddBcdByte(uint8 byte1,uint8 byte2,uint8* pCarry)//其中pCarry为指向进位的指针
{
	uint8 b1,b2,b3,b4,b,temp,result;
	b = *pCarry;
	b1=byte1 & 0x0F;
	b2 = (byte1>>4) & 0x0F;
	b3=byte2 & 0x0F;
	b4 = (byte2>>4) & 0x0F;	
	temp = b1+b3+b;
	b = temp/10;
	temp = temp % 10;
	result = temp;
	temp = b2+b4+b;
	b = temp/10;
	temp = temp % 10;	
	result = (temp<<4) + result;
	*pCarry = b;
	return result;
}
uint8 AddBcdStr(uint8* str1,uint8* str2,uint8 len)//两个BCD串相加，结果放在第一个串中
{
	uint8 i,temp;
	uint8 b=0;
	uint8* pCarry;
	pCarry = &b;
	for(i=0;i<len;i++)
	{
		temp = AddBcdByte(*str1,*str2,pCarry);
		*str1 = temp;
		str1++;
		str2++;	
	}
	return 0;	
}
//两个Bcd字节减法，byte1-byte2,其中pCarry为指向借位的指针
uint8 SubBcdByte(uint8 byte1,uint8 byte2,uint8* pCarry)
{
	uint8 b1,b2,b3,b4,b,temp,result;
	b = *pCarry;
	b1=byte1 & 0x0F;
	b2 = (byte1>>4) & 0x0F;
	b3=byte2 & 0x0F;
	b4 = (byte2>>4) & 0x0F;
	if(b1>=(b3+b)) 
	{
		temp = b1-b3-b;
		b = 0;
	}
	else 
	{
		temp = b1+10-b3-b;
		b=1;
	}
	result = temp;
	if(b2>=(b4+b)) 
	{
		temp = b2-b4-b;
		b = 0;
	}
	else 
	{
		temp = 10+b2-b4-b;
		b=1;
	}
	result = (temp<<4) + result;
	*pCarry = b;
	return result;
}
uint8 SubBcdStr(uint8* str1,uint8* str2,uint8 len)//两个BCD串相减，结果放在第一个串中
{
	uint8 i,temp;
	uint8 b=0;
	uint8* pCarry;
	pCarry = &b;
	for(i=0;i<len;i++)
	{
		temp = SubBcdByte(*str1,*str2,pCarry);
		*str1 = temp;
		str1++;
		str2++;	
	}
	return b;	
}
uint8 CmpStr1(uint8 *str1, uint8 *str2, uint8 len)
{
	int8 i=0;
	for(i=len-1; i>=0; i--)
	{
	    if(str1[i]==str2[i]) continue;
		else if(str1[i]>str2[i])return 1;
	    else return 2;//这样可以收敛快些
	}
	return 0;
}

uint8 CmpStr(uint8 *str1, uint8 *str2, uint8 len)
{
	int8 i=0;
	for(i=len-1; i>=0; i--)
	{
	    if(str1[i]==str2[i]) continue;
		else if(str1[i]>str2[i])return 0;
	    else return 1;//这样可以收敛快些
	}
	return 0;
}

//两个BCD串相加，结果放在第一个串中,其中最高位为符号位
uint8 AddBcdStrWithSign(uint8* str1,uint8* str2,uint8 len)
{
	uint8 sign1,sign2,b;
	sign1=str1[len-1] & 0x80;
	sign2 = str2[len-1] & 0x80;
	str1[len-1] = str1[len-1] & 0x7F;
	str2[len-1] = str2[len-1] & 0x7F;		
	if(sign1==sign2) 
	{
		AddBcdStr(str1,str2,len);
		str1[len-1] |= sign1;
	}
	else
	{
		b = CmpStr(str1,str2,len);//比较两个字符串大小
		if(b) 
		{	
		    SubBcdStr(str2,str1,len);
		    memcpy(str1, str2, len);
		    str1[len-1] |= sign2;
		}
		else
		{
			SubBcdStr(str1,str2,len);
		    str1[len-1] |= sign1;
		}
	}
	return 0;
}
//两个BCD串相减，str2-str1 结果放在第一个串中,其中最高位为符号位
uint8 SubBcdStrWithSign(uint8* str1,uint8* str2,uint8 len)
{
	uint8 sign1,sign2,b;
	sign1=str1[len-1] & 0x80;
	sign2 = str2[len-1] & 0x80;
	str1[len-1] = str1[len-1] & 0x7F;
	str2[len-1] = str2[len-1] & 0x7F;		
	if(sign1!=sign2) 
	{
		AddBcdStr(str1,str2,len);
		str1[len-1] |= sign1;
	}
	else
	{
		b = CmpStr(str1,str2,len);//比较两个字符串大小
		if(b) 
		{	
		    SubBcdStr(str2,str1,len);
		    str1[len-1] |= sign2;
		}
		else
		{
			SubBcdStr(str1,str2,len);
		    str1[len-1] |= sign1;
		}
		
	}
	return 0;
}
void CountV_645_97(uint8* pData)
{
   	uint8 b1,b2;
 	b1 =pData[0]<<4;
  	b2 =(pData[1]<<4) | (pData[0]>>4);
  	pData[0] = b1;
  	pData[1] = b2;
}
void CountI_645_97(uint8* pI1,uint8* pI2)//将97回来的数据pI1调整为376。1数据格式25 pI2
{
   uint8 sign = pI1[1] & 0x80;	
	pI2[0] = pI1[0]<<4;
	pI2[1] = pI1[0]>>4 | pI1[1]<<4;
	pI2[2] = pI1[1]>>4 | sign;
	
}

int8 CmpStrAbsolute(uint8 *str1, uint8 *str2, uint8 len)
{
    uint8 b1,b2;
    
	int8 i=0;   
    b1 = str1[len-1] & 0x7F;
    b2 = str2[len-1] & 0x7F; 
    if(b1>b2) return 1;
    else if(b1<b2) return -1;   
	for(i=len-2; i>=0; i--)
	{
	    if(str1[i]==str2[i]) continue;
		else if(str1[i]>str2[i]) return 1;
		else return -1;
	}
	return 0;
}
/*********************************************************************************************************
** 函数名称: CheckDT
** 功能描述: 输入DTBcd：顺序为秒 分 时 日 月 年  ，月的高三位为星期. 均为BCD格式（由于月的高三位为星期.故对于这个字节不时BCD格式） 
** 对输入的日期时间进行合法性验证，正确返回0 ； 不正确返回非0
** 特别说明：对于星期，由于可能的范围有 0-6 ，或 1-7 ，所以本函数没有进行合法性判断
********************************************************************************************************/
uint8 CheckDT(uint8* DTBcd)
{
	uint8 year1,month1,day1;
	uint8 days[13];
	uint8 i,tmpbcd;
	//===
	//将来可能需要在这里对星期进行判断
	//===
	
	//==进性BCD玛的合法性判断
	for(i=0;i<6;i++){
		tmpbcd = DTBcd[i];
		if(i==4){
			tmpbcd &= 0x1F;//去掉月的星期信息
		}
		
		if(BcdCheck(tmpbcd)==FALSE){
			return (1);
		}
	}
	//==进性BCD的合法性判断
	
	//==下面进行取值范围的合法性判断
	
	if(BcdToHex(DTBcd[0])>60){ // 秒不能大于60
		return 2;
	}
	if(BcdToHex(DTBcd[1])>60){ // 分不能大于60
		return 3;
	}
	if(BcdToHex(DTBcd[2])>23){ // 时不能大于23
		return 4;
	}
	
	year1 = BcdToHex(DTBcd[5]);
	month1 = BcdToHex(DTBcd[4]&0x1F);
	day1 = BcdToHex(DTBcd[3]);
	
	if( month1 == 0 || month1>12 ){ //月应为1-12
		return 5;
	}
	
	//===================得到正确的12个月的天数
	days[0] = 0; //days[0]不使用
	days[1] = 31;
	days[2] = 28;
	days[3] = 31;
	days[4] = 30;
	days[5] = 31;
	days[6] = 30;
	days[7] = 31;
	days[8] = 31;
	days[9] = 30;
	days[10] = 31;
	days[11] = 30;
	days[12] = 31;

	if(IsLeap(year1)){ //如果为闰年，二月为29天
		days[2] = 29;
	}
	//===================得到正确的12个月的天数
	
	if(day1==0 || day1 > days[month1]){ //日期 不能为0，也不能超过28，29，30，或 31，（由月确定，前面已保证月为1-12）
		return 6;
	}
	
	return (0);//日期六个字节全为合法BCD码，并在合法取值范围内
}
/*********************************************************************************************************
** 函数名称: Zeller
** 功能描述: 蔡勒（Zeller）公式,给一个日期，就能用这个公式推算出是星期几
** 输入: 年月日,bin格式,注意不是BCD格式
** 输出：星期，0-6 ，0代表星期日
** 特别说明: 本函数只计算2000年之后的星期
********************************************************************************************************/
uint8 Zeller(uint8 year,uint8 month,uint8 day)
{
	//蔡勒（Zeller）公式: w=y+[y/4]+[c/4]-2c+[26(m+1)/10]+d-1 
	
	int32 w; //星期
	uint8 c; //世纪-1
	c = 20;
	if(month==1||month==2){ //如果是1月和2月,应修正为上一年的13月和14月
		if(year==0){
			year=99;
			c=19; //20世纪
		}
		else{
			year-=1;
		}
		month+=12;
	}
	
	w=(int32)year+year/4+c/4-2*c;
	w=w+(26*(month+1))/10;
	w=w+day-1;
	
	//负数取模应按数论中的余数的定义,所以下面将负数加7的整数倍调整为正数
	while(w<0){
		w+=7;
	}
	return (w%7);
}
/*********************************************************************************************************
** 函数名称: Zeller_DTBcd
** 功能描述: 使用蔡勒（Zeller）公式获得正确的星期
** 输入: 输入DTBcd：顺序为秒 分 时 日 月 年  , 均为BCD格式,月的高三位可以非0,函数内部会处理掉
** 输出：星期，将DTBcd中月的高三位写为用蔡勒公式算出的星期,其他字节不变
** 特别说明: 本函数只计算2000年之后的星期
********************************************************************************************************/
void Zeller_DTBcd(uint8* DTBcd)
{
	uint8 week,year,month,day;
	year = BcdToHex(DTBcd[5]);
	month = BcdToHex(DTBcd[4]&0x1f);
	day = BcdToHex(DTBcd[3]);
	
	week = Zeller(year,month,day);
	
	DTBcd[4]=(DTBcd[4]&0x1f)|(week<<5);
}

//比较月年的大小    如果大于目标日期则返回2，小于则返回1，等于则返回0
uint8 CmpData_M(uint8 *Target,uint8 *Current)
{
	 if(Current[1] > Target[1]) return 2;
     else if(Current[1] == Target[1])
     {
        if(Current[0] > Target[0]) return 2;
        else if(Current[0] == Target[0])
             return  0;
        else return 1; 
     }
     else return 1;
}


uint8 JudgeSameDayMonthAndYear(uint8 *CurrentDay,uint8* AskDate)
{  
    if(CurrentDay[0] !=AskDate[0])   return FALSE;
    if( (CurrentDay[1]&0x1F) != (AskDate[1]&0x1F) )   return FALSE;
    if(CurrentDay[2] !=AskDate[2])   return FALSE;
    return TRUE;
}
uint8 JudgeSameDayMonthAndYear2(uint8 *CurrentDay,uint8* AskDate)//零时10分前可视为同一天
{ 
    if((CurrentDay[0] ==AskDate[3]) && ((CurrentDay[1]&0x1F) == (AskDate[4]&0x1F) )  && (CurrentDay[2] ==AskDate[5]) )
     return TRUE;
    CountDate(CurrentDay,1);	
    if((CurrentDay[0] ==AskDate[3]) && ((CurrentDay[1]&0x1F) == (AskDate[4]&0x1F) )  && (CurrentDay[2] ==AskDate[5]) && (10>AskDate[1]))
    return TRUE;
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
//判断月是不是同一月
uint8 JudgeSameMonth(uint8 *CurrentMonth,uint8* AskDate)   //判断月是不是同一天
{  
    if( (CurrentMonth[0]&0x1F) != (AskDate[0]&0x1F) )   return FALSE;
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
//判断月是不是同一年的同一月

uint8 JudgeSameMonthAndYear(uint8 *CurrentMonth,uint8* AskDate)   //判断月是不是同一天
{  
    if(((CurrentMonth[0]&0x1F) == (AskDate[4]&0x1F)) && (CurrentMonth[1] ==AskDate[5])) return TRUE;
    
    return FALSE;
     
}

uint8 JudgeSameMonthAndYear_1(uint8 *CurrentValue, uint8 *AskValue) //每月的最后一天抄表时间延时到1日的零时10分
{

    if(((CurrentValue[0]&0x1F) == (AskValue[4]&0x1F)) && (CurrentValue[1] ==AskValue[5])) return TRUE;
     CountDate(CurrentValue,1);	
    if(((CurrentValue[0]&0x1F) == (AskValue[4]&0x1F)) && (CurrentValue[1] ==AskValue[5]) && (AskValue[3]==1) && (AskValue[2] ==0) && (AskValue[1] <10)) return TRUE;
     return FALSE;
}
void GotoNextTime(uint8 time[5],uint8 FreezeInt)
{
	switch(FreezeInt)
	{
		case 1://冻结密度15分钟
		    CountMinute(time,15);
			break;
		case 2://冻结密度30分钟
			CountMinute(time,30);	
			break;
		case 3://冻结密度60分钟
			CountMinute(time,60);		
			break;
		default:
			CountMinute(time,15);	
			break;
	}		
}

void AdjectStartTime(uint8 time[5],uint8 FreezeInt)
{
    uint8 temp1,temp2;  
    switch(FreezeInt)
	{
		case 1://冻结密度15分钟
		    temp1 = BcdToHex(time[0]);
		    temp2 = temp1 % 15;
		    if (temp2) 
		       CountMinute(time,15-temp2);
			break;
		case 2://冻结密度30分钟
			 if (BcdToHex(time[0])%30) CountMinute(time,(30-BcdToHex(time[0])%30));		
			break;
		case 3://冻结密度60分钟
			if (BcdToHex(time[0])%60) CountMinute(time,(60-BcdToHex(time[0])%60));			
			break;
		default:
			break;
	}	

}

uint8 PUBLIC_CountCS(uint8* _data, uint16 _len)
{
    uint8 cs = 0;
    uint16 i;
    
    for(i=0;i<_len;i++)
    {
       cs += *_data++;
    }
    
    return cs;    
}
/**
  * @brief  This function handles HexStreamToString.
  * @param  uint8 *pIn, uint16 InLen
  * @retval  char *pOut
  */
char* PUBLIC_HexStreamToString(uint8 *pIn, uint16 InLen, char *pOut)
{
	uint16 i;
	char *pTemp;
  /*begin:yangfei 2013-02-20 for pOut 缓存为1024，
  防止数组越界,防止打印太多，打印缓存为256*/
	if(InLen>100)
	  {
	    InLen = 100;
      }
  /*end:yangfei 2013-02-20*/
	pTemp = pOut;
	for(i=0; i<InLen; i++)
	{
		*pOut++ = UINT8_TO_ASCII_H( ((*pIn)>>4)&0x0F );
		*pOut++ = UINT8_TO_ASCII_L( (*pIn)&0x0F );
		pIn++;
	}
	*pOut	= '\0';
	
	return pTemp;
}

char* PUBLIC_MeterAddrToString(uint8 *pMeterAddrIn, char *pMeterOut,int length)
{
	uint16 i;
	char *pTemp;
	
	pTemp = pMeterOut;
	for(i=0; i < length; i++)
	{
		*pMeterOut++ = UINT8_TO_ASCII_H( (pMeterAddrIn[6-i]>>4)&0x0F );
		*pMeterOut++ = UINT8_TO_ASCII_L( pMeterAddrIn[6-i]&0x0F );
	}
	*pMeterOut		= '\0';
	
	return pTemp;
}
