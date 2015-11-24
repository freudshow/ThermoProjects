

#include <includes.h>
#include "DateTime.h"

#define HEX_TO_BCD(x) ((x/0x0A)*0x10+(x%0x0A))
#define BCD_TO_HEX(x) ((x/0x10)*0x0A+(x%0x10))

uint8 TIME_AddSec(uint8* pDate,uint8 sec) //秒增量
{
    uint8 temp,temp1,temp2;
    temp = BcdToHex(pDate[0])+sec;
    temp1 = temp % 60;
    temp2 = temp / 60;
    pDate[0] = HexToBcd(temp1);
    if (temp2!=0)  TIME_AddMin(pDate,temp2);
    return 0;

}
uint8 TIME_AddMin(uint8* pDate,uint8 min) //分钟增量
{
    uint8 temp,temp1,temp2;
    temp = BcdToHex(pDate[1])+min;
    temp1 = temp % 60;
    temp2 = temp / 60;
    pDate[1] = HexToBcd(temp1);
    if (temp2!=0)  TIME_AddHour(pDate,temp2);
    return 0;

}
uint8 TIME_AddHour(uint8* pDate,uint8 hour) //小时增量
{
    uint8 temp,temp1,temp2;
    temp = BcdToHex(pDate[2])+hour;
    temp1 = temp % 24;
    temp2 = temp / 24;
    pDate[2] = HexToBcd(temp1);
    if (temp2!=0)  TIME_AddDate(pDate,temp2);
    return 0;
}
uint8 TIME_AddDate(uint8* pDate,uint8 date) //日增量
{
	uint8 year1;
	uint8 month1;
	uint8 day1;
	uint8 days[13];
	
		
	year1 = BcdToHex(pDate[5]);
	month1 = BcdToHex(pDate[4]);
	day1 = BcdToHex(pDate[3]);
	
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
		if ((day1+date) <= days[month1] )
		{
			pDate[3]= HexToBcd(day1+date);
			pDate[4]=HexToBcd(month1);
			pDate[5] = HexToBcd(year1);
		}
		else
		{
			pDate[3] = HexToBcd(day1+date - days[month1]);
			if (month1==12)
			{
				pDate[5] = HexToBcd(year1+1);
				pDate[4] =1;
			}
			else
			{
				pDate[5] = HexToBcd(year1);
				pDate[4] =HexToBcd(month1+1);
			}
		}
    
	}
	else
	{
		if ((day1+date) < 29)
		{
			pDate[3]= HexToBcd(day1+date);
			pDate[4]=2;
			pDate[5] = HexToBcd(year1);
		}
		else
		{
			pDate[3] =HexToBcd(day1+date - 29);
			pDate[4] = 3;
			pDate[5] = HexToBcd(year1);
		}
	}
    return 0;
}
uint8 TIME_AddMon(uint8* pDate,uint8 mon) //月增量
{
     uint8 temp,temp1,temp2;
    temp = BcdToHex(pDate[1])+mon;
    temp1 = temp % 12;
    temp2 = temp / 12;
    pDate[4] = HexToBcd(temp1);
    if (temp2!=0)  pDate[5] = HexToBcd(BcdToHex(pDate[5])+ temp2);
    return 0;
}
uint8 TIME_SubMin(uint8* pDate,uint8 min) //分钟减法
{
    uint8 temp,temp1;
    temp = BcdToHex(pDate[1]);
    if (temp>=min) temp1 = temp-min;
    else 
    {
    	temp1 = temp+60-min;
    	TIME_SubHour(pDate,1);
    }
    pDate[1] = HexToBcd(temp1);
    return 0;

}
uint8 TIME_SubHour(uint8* pDate,uint8 hour) //小时减法
{
   uint8 temp,temp1;
    temp = BcdToHex(pDate[1]);
    if (temp>hour) temp1 = temp-hour;
    else 
    {
    	temp1 = temp+24-hour;
    	SubDate(pDate);
    }
    pDate[2] = HexToBcd(temp1);
    return 0;
}

uint8 TIME_AddTime(uint8* pDate,uint8 time,uint8 flag) //增量
{
	    switch(flag)
    {
    	case 0:
           TIME_AddMin(pDate,time); //分钟增量
    		break;
    	case 1:
    		TIME_AddHour(pDate,time);//小时增量

    	    break;
    	case 2:
    	    TIME_AddDate(pDate,time); //日增量
    	    break;
    	case 3:
    		TIME_AddMon(pDate,time); //月增量
    	    break;
    	default:
    	    break;
    }
    return 0;
}
int8 TIME_CmpTime(uint8* pTime2,uint8* pTime1)//精确到分钟 
{
	if (pTime2[5]>pTime1[5]) return 1;//比较年
	else if(pTime2[4]<pTime1[4]) return -1;//
	else
	{
		if(pTime2[4]>pTime1[4]) return 1;//月
		else if(pTime2[3]<pTime1[3]) return -1;//
		else
		{
			if(pTime2[3]>pTime1[3]) return 1;//日
			else if(pTime2[2]<pTime1[2]) return -1;
			else
			{
				if(pTime2[2]>pTime1[2]) return 1;//时
				else if(pTime2[1]<pTime1[1]) return -1;//
				else
				{
					if(pTime2[1]>pTime1[1]) return 1;//分
					else if(pTime2[0]<pTime1[0]) return -1;//
					else return 0;
				}
			}
		}
	}
}

void TIME_BspSystemTimeInit(void)
{
	uint8 err = 0;
	uint8 tempRead[17] = {0x00};
	uint8 temptime[6]  = {0x00};
	CPU_SR cpu_sr;
	
	Rd8025(tempRead);

	if((tempRead[15] & 0x20) == 0){  //如果检测到RX8025出现过震动停止。
		OSSemPend(I2cReqSem, 0, &err);
		RX8025_ClearXST();
		OSSemPost(I2cReqSem);
	}
	
	temptime[ARRAY_YEAR]   = BCD_TO_HEX(tempRead[ARRAY_YEAR+1]);
	temptime[ARRAY_MONTH]  = BCD_TO_HEX(tempRead[ARRAY_MONTH+1]);
	temptime[ARRAY_DAY]    = BCD_TO_HEX(tempRead[ARRAY_DAY+1]);
	temptime[ARRAY_HOUR]   = BCD_TO_HEX(tempRead[ARRAY_HOUR]);
	temptime[ARRAY_MINUTE] = BCD_TO_HEX(tempRead[ARRAY_MINUTE]);
	temptime[ARRAY_SECOND] = BCD_TO_HEX(tempRead[ARRAY_SECOND]);
	
	OS_ENTER_CRITICAL();
	memcpy(gSystemTime, temptime, 6);
	OS_EXIT_CRITICAL();
}

void TIME_UpdateSystemTime(void)
{
	uint8 temptime[13]={0};
	CPU_SR cpu_sr;

	/*yangfei added temp*/
	UserReadDateTime(temptime);
    temptime[4] =(temptime[4] & 0x1F);
	
	temptime[ARRAY_YEAR]   = BCD_TO_HEX(temptime[ARRAY_YEAR]);
	temptime[ARRAY_MONTH]  = BCD_TO_HEX(temptime[ARRAY_MONTH]);
	temptime[ARRAY_DAY]    = BCD_TO_HEX(temptime[ARRAY_DAY]);
	temptime[ARRAY_HOUR]   = BCD_TO_HEX(temptime[ARRAY_HOUR]);
	temptime[ARRAY_MINUTE] = BCD_TO_HEX(temptime[ARRAY_MINUTE]);
	temptime[ARRAY_SECOND] = BCD_TO_HEX(temptime[ARRAY_SECOND]);
	
	OS_ENTER_CRITICAL();
	memcpy(gSystemTime, temptime, 6);
	OS_EXIT_CRITICAL();
}

uint8 ReadDateTime(uint8* retbuf)
{
	uint8 temptime[6] = {0x00};
	CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();
	memcpy(temptime, gSystemTime, 6);
	OS_EXIT_CRITICAL();
	
	retbuf[ARRAY_SECOND] = HEX_TO_BCD(temptime[ARRAY_SECOND]);
	retbuf[ARRAY_MINUTE] = HEX_TO_BCD(temptime[ARRAY_MINUTE]);
	retbuf[ARRAY_HOUR] 	 = HEX_TO_BCD(temptime[ARRAY_HOUR]);
	retbuf[ARRAY_DAY] 	 = HEX_TO_BCD(temptime[ARRAY_DAY]);
	retbuf[ARRAY_MONTH]  = HEX_TO_BCD(temptime[ARRAY_MONTH]);
	retbuf[ARRAY_YEAR]   = HEX_TO_BCD(temptime[ARRAY_YEAR]);
    
    return 0;
}

