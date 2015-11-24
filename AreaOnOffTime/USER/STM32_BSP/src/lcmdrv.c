#include <includes.h>
#define  IN_LCMDRV
#include "lcmdrv.h"
#include "ASCIICode.h"
#include "HZTable.h"
#include "PictureCode.h"
#include "app_down.h"
#include "PublicFunc.h"

#define LCMRST_ENABLE	GPIO_ResetBits(GPIOG , LCM_RESET)
#define LCMRST_DISABLE	GPIO_SetBits(GPIOG , LCM_RESET)

#define BCD_TO_HEX(x) ((x/0x10)*0x0A+(x%0x10))


BLT_CTR gBltCtr;

OS_EVENT *KeyMessageQ;
OS_EVENT *LcmSem;

extern uint8 KeyMessageFlag;
extern uint8 ShowType_Flag;

uint8 GetKeyStatus(void)
{
	uint8 err;
	uint32 Status;
	//Status=(uint32)OSQPend(KeyMessageQ, OS_TICKS_PER_SEC/50, &err);   //得到按键状态：上、下、左、右
	Status=(uint32)OSQPend(KeyMessageQ, 0, &err);   //得到按键状态：上、下、左、右
	KeyMessageFlag = 1;
	return (uint8)(Status);
}

uint8 GetKeyStatuschaobiao(void)
{
	uint8 err;
	uint32 Status;
	Status=(uint32)OSQPend(KeyMessageQ, 1500, &err);   //得到按键状态：返回
	KeyMessageFlag = 1;
	return (uint8)(Status);
}



void  delay(uint32  dly)
{  uint32  i;

   for(; dly>0; dly--) 
      for(i=0; i<2; i++);
}

uint8 readi(void)
{
    uint8 data;
    data=*((uint8*)(LDM_R_ADDR));
    return data;
    
}
uint8 readd(void)
{
    uint8 data;
    data=*((uint8*)(LDM_D_ADDR));
    delay(0);
    return data;
    
}


void writei(unsigned char ins)
{
   
   	*((uint8*)(LDM_R_ADDR))=ins;

    delay(0);
}

void writed(unsigned char dat)
{
     
   	*((uint8*)(LDM_D_ADDR))=dat;
   	//delay(0);
}

void lcd_initial(void)
{
//	uint8 Err 	= 0;

	delay(0x00000fff);
    LCMRST_ENABLE;
    delay(0x000fffff);
    LCMRST_DISABLE;
    delay(0x00000fff);
	
    writei(0xe2);  //set system reset
    delay(0x000fffff);   //delay 200ms

    writei(0x24);  //set Temp. command  TC1TC0 00=-0.00%
    writei(0x2b);  //internal pump

#ifdef UPDOWN_DIS    
	  writei(0xC2);  //set lcd mapping  MY=0,Mx=1,LC0=0
      writei(0x8D);//set AC[2-0]
#else
	  writei(0xC4);  //set lcd mapping  MY=1,Mx=0,LC0=0
#endif

      writei(0xA1);  //set line rate
      writei(0xc8);  //set n-line
      writei(0x1f);

      writei(0xD1);  //set color pattern RGB

      writei(0xD5);  //set 4K color mode

      writei(0xE9);  //set lcd bias ratio: 1/10bias

      writei(0x81);  //set Vbias potentiometer
      writei(0xc3);  //set contrast level:0-255
	
      writei(0xde);  //set com scan function
      
      LCM_LIGHT_ENABLE;
      gBltCtr.Sta = TRUE; //表示背光已打开
	  gBltCtr.DogCnt = 0;
	  gBltCtr.DogCntThrd = BLIGHT_DOG_CNT_THRD;
        
	  display_white();
	  //ShowPicture(28,9,104,104,&StartPic[0]);
	  
	  SetContrast(LCD_CON_VALUE);//设置对比值
	  writei(0xAd);  //display enable
	  
	  if(clock_state == FALSE)
	  {
		  ShowLineChar_HZ(4,10,&ClockErr[0],7);//集中器管理与维护
		  delay(0x00200000);
	  }  
      delay(0x00000500);
 }


void window_program(void)      //set window
{


    writei(0xF4);  //set start column
    writei(0x23);  //start seg106

    writei(0xF5);  //set start row
    writei(0x00);  //start com1

    writei(0xF6);  //set end column
   writei(0x6b);//应该设置为0x58，但为了使显示不完的字符不能从头显示出来，这里特别设置大一些
    
    writei(0xF7);  //set end row
    writei(0x9F);  //end com160
     
}

void display_black(void)
{
  int i;
  int j;
  window_program();
  for(i=0;i<160;i++)
     {

		for(j=0;j<54;j++)
		{
		   writed(0xff);
		   writed(0xff);
		   writed(0xff);
		}
		
		
	}
}

void display_white(void)
{
  int i;
  int j;
  window_program();
  for(i=0;i<160;i++)
     {

		for(j=0;j<54;j++)
		{
		   writed(0x00);
           writed(0x00);
		   writed(0x00);
		}
		}
}

void snow()
{
      int i;
      int j;
      window_program();
     for(i=0;i<160;i++)
     {
		for(j=0;j<54;j++)
		{
		 writed(0xf0);
                 writed(0xf0);
                 writed(0xf0);
		}
		for(j=0;j<54;j++)
		{
		 writed(0x0f);
                 writed(0x0f);
                 writed(0x0f);
		}

     }
}

void network()
{
	  int i;
      int j;
      window_program();
     for(i=0;i<160;i++)
     {
		for(j=0;j<54;j++)
		{
		 writed(0xf0);
                 writed(0xf0);
                 writed(0xf0);
		}
		for(j=0;j<54;j++)
		{
		         writed(0xff);
                 writed(0xff);
                 writed(0xff);
		}
     }
}


void horizontal()
{
  int i;
  int j;
  window_program();

  for(i=0;i<160;i++)
     {
		for(j=0;j<54;j++)
		{  writed(0xff);
           writed(0xff);
		   writed(0xff);
		}

		for(j=0;j<54;j++)
		{  writed(0xff);
           writed(0xff);
		   writed(0xff);
		}

	    for(j=0;j<54;j++)
		{
		   writed(0x00);
           writed(0x00);
		   writed(0x00);
		}
     }
}

void horizontal1()
{
  int i;
  int j;
  window_program();

  for(i=0;i<160;i++)
     {
		for(j=0;j<54;j++)
		{  writed(0xff);
           writed(0xff);
		   writed(0xff);
		}

		for(j=0;j<54;j++)
		{  writed(0xff);
           writed(0xff);
		   writed(0xff);
		}

	    for(j=0;j<54;j++)
		{
		   writed(0x00);
           writed(0x00);
		   writed(0x00);
		}
     }
}

void horizontal2()
{
  int i;
  int j;
  window_program();

  for(i=0;i<160;i++)
     {
	
		for(j=0;j<54;j++)
		{  writed(0xff);
           writed(0xff);
		   writed(0xff);
		}

	    for(j=0;j<54;j++)
		{
		   writed(0x00);
           writed(0x00);
		   writed(0x00);
		}
     }
}

void vertical()
{
      int i;
      int j;
      window_program();
     for(i=0;i<160;i++)
     {
		for(j=0;j<54;j++)
		{
		   writed(0xff);
           writed(0x0f);
		   writed(0xf0);
		}
     }
}

void vertical1()
{
      int i;
      int j;
      window_program();
     for(i=0;i<160;i++)
     {
		for(j=0;j<54;j++)
		{
		   writed(0x0f);
           writed(0xf0);
		   writed(0xff);
		}
     }
}

void frame()
{
      int i,j;
      window_program();
     for(i=0;i<160;i++)
	{
		if(i==0||i==159)
	        {
                 for(j=0;j<54;j++)
		    {writed(0xff);
             writed(0xff);
		     writed(0xff);}
		}

		else

		for(j=0;j<54;j++)
		{
		 if(j==44)
		   {writed(0xf0);
		    writed(0x00);
		    writed(0x00);}
                 else if(j==17)
		   {writed(0x00);
		    writed(0x0f);
		    writed(0x00);}
                 else
                       { writed(0x00);
                         writed(0x00);
		                 writed(0x00);
			}
		}
	}
}
/***********************************************************************************
*函数名：SetContrast()
*功能：设置液晶对比度
*入口参数：ContrastValue:对比度值 0--255 
*返回参数：无
************************************************************************************/
void SetContrast(uint8 ContrastValue)
{
      writei(0x81);  //set Vbias potentiometer
      writei(ContrastValue);  //set contrast level:0-255
}
/***********************************************************************************
*函数名：SetLine1()
*功能：设置行地址row    0<=line<=159 
*入口参数：chip:芯片选择
 		   line:设置的行（页）	
*返回参数：操作错误返回0，正确返回1
************************************************************************************/

void SetLine1(uint16 line) // 0<=line<=159    1 line = 1 row
{
	uint8 LineLSB;
	uint8 LineMSB;
	LineLSB = (line&0x0F)|0x60;
	LineMSB = (line >> 4)&0x0F;
	LineMSB = LineMSB | 0x70;
	
	writei(LineLSB);
	writei(LineMSB);
}

/***********************************************************************************
*函数名：SetLine()
*功能：设置行地址页  0<=line<=19 
*入口参数：
 		   line:设置的行（页）	
*返回参数：无
************************************************************************************/

void SetLine(uint16 line)  // 0<=line<=19    1 line = 8 row
{
	uint8 LineLSB;
	uint8 LineMSB;
	
	line = line % 20;
	line = line << 3;
	LineLSB = (line&0x0F)|0x60;
	LineMSB = (line >> 4)&0x0F;
	LineMSB = LineMSB | 0x70;
	
	writei(LineLSB);
	writei(LineMSB);
	
}

/***********************************************************************************
*函数名：SetColumn()
*功能：设置列地址  --Y  0--53
*入口参数：chip:芯片选择
 		   column:设置的列	
*返回参数：操作错误返回0，正确返回1
************************************************************************************/
void SetColumn(uint16 column)  //0<=column<=53   1 column = R G B = 3 列
{
	uint8 ColumnLSB;
	uint8 ColumnMSB;
	column = column % 54;
	column = column + COLUMN_OFFSET; // 液晶从106列开始
	ColumnLSB = (column & 0x0F)| 0x00;
	ColumnMSB = (column>>4) & 0x07;
	ColumnMSB = ColumnMSB | 0x10;
	writei(ColumnLSB);
	writei(ColumnMSB);
}

uint32 ConvertByte(uint8 Bytedata) //字节按位转换，0转换为0，1转换为F，转换后为32位
{
	uint8 i,flag;
	uint32 converdata = 0;
	for(i=0;i<8;i++)
	{
		flag = Bytedata&0x01;
		if(flag == 1)
		{
	//		converdata |= 0x0F; 
			converdata = converdata | (0x0F << (4*i));
		}
		
		Bytedata = Bytedata >> 1;
	}
	return converdata;
}

void Convert(uint8* Daddr, const uint8* Saddr,uint8 ByteNum) //
{
	uint8 i,j;
	uint8 temp1,temp2;
	uint8 flag;
	uint8 tempdata;
	
	for(i = 0;i < ByteNum;i++)
	{
		if(Saddr[i] == 0x00) //字节全为0的情况下
		{
			//memset(&Daddr[4*i],0,4);
			for(j =0;j<4;j++)
				Daddr[4*i+j] = 0x00;
			continue;
		}
		
		tempdata = Saddr[i];
		for(j = 0;j < 4;j++) //非全0的情况下
		{
			flag = tempdata&0x80;
			if(flag == 0x80)
			{
				temp1 = 0xF0;
				//Daddr[i*8+j] = 0x0F;
			}
			else
			{
				temp1 = 0x00;
			}
			
			//Daddr[i*8+j] = 0x00;
			tempdata = tempdata << 1;
			
			flag = tempdata&0x80;
			if(flag == 0x80)
			{
				temp2 = 0x0f;
			}
			else
			{
				temp2 = 0x00;
			}
				
			Daddr[i*4+j] = temp1|temp2;	
			
			tempdata = tempdata << 1;			
		}
	}
	
}

uint8 UpDown_Convert(uint8* Daddr,uint8* Saddr,uint8 ByteNum)
{
	uint8 i,j;
	uint8 temp;
	uint8 flag;
	uint8 tempdata, len;
	
	for(i = 0;i < ByteNum;i++)
	{
		if(Saddr[i] == 0x00) //字节全为0的情况下
		{
			//memset(&Daddr[4*i],0,4);
			for(j =0;j<8;j++)
			{
				Daddr[8*i+j] = 0x00;
			}	
			len = i*8+j;
			continue;
		}
		
		tempdata = Saddr[i];
		for(j = 0;j < 8;j++) //非全0的情况下
		{
			flag = tempdata&0x80;
			if(flag == 0x80)
			{
				Daddr[i*8+j] = 0xFF;
			}
			else
			{
				Daddr[i*8+j] = 0x00;
			}
			tempdata = tempdata << 1;
		}
		len = i*8+j;
	}
	
	for(i=0; (i+2)<len; i+=3)
	{
		tempdata = Daddr[i];
		Daddr[i] = Daddr[i+2];
		Daddr[i+2] = tempdata;
	}
	
	temp = len%3;
	if(temp == 1)
	{
		Daddr[i+2] = Daddr[i];
		Daddr[i+1] = 0x00;
		Daddr[i] = 0x00;	
		len = len + 2;
	}
	else if(temp == 2)
	{
		Daddr[i+3] = 0x00;
		Daddr[i+2] = Daddr[i+1];
		Daddr[i+1] = Daddr[i];
		Daddr[i] = 0x00;
		len = len + 2;
	}
	return len;
}

uint32 ConvertRevertShow(uint32 data)//反向存储的扫描转换数字
{
	uint32 tmpdata;
	uint8 bit1;
	uint8 bit3;
	uint8 bit4;
	uint8 bit6;
	uint8 bit7;
	uint8 bit8;
	bit1 = (data & 0x0000000F);
	bit3 = (data & 0x00000F00)>>8;
	bit4 = (data & 0x0000F000)>>12;
	bit6 = (data & 0x00F00000)>>20;
	bit7 = (data & 0x0F000000)>>24;
	bit8 = (data & 0xF0000000)>>28;
	
	tmpdata = data & 0x000F00F0;
	tmpdata = tmpdata | (bit1<<8);
	tmpdata = tmpdata | (bit3);
	tmpdata = tmpdata | (bit4<<20);
	tmpdata = tmpdata | (bit6<<12);
	tmpdata = tmpdata | (bit7<<28);
	tmpdata = tmpdata | (bit8<<24);
	
	return tmpdata;
}


/***********************************************************************************
*函数名：Show88()
*功能：显示8*8点阵
*入口参数：line:行(0-19)
 		   column: 列(0-54)
           address：字模区首地址
*返回参数：无
************************************************************************************/
void Show98(uint16 line,uint16 column,const uint8* addr)
{
	uint8 i,j,temp;
	//uint8 temp1,temp2;
	uint32 tempdata;
		
	SetLine(line);
	SetColumn(column);
	line = line << 3;
	for(i=1;i<=8;i++)
	{	
		if(addr[i-1]==0x00)
		{
			writed(0x00);
			writed(0x00);
			writed(0x00);
			writed(0x00);
			writed(0x00);
		}
		else
		{
			tempdata = ConvertByte(addr[i-1]);
	#ifdef	UPDOWN_DIS
			tempdata = ConvertRevertShow(tempdata);
			for(j=0;j<3;j++)
			{
				temp = tempdata&0x000000FF;
				temp = (temp << 4)|(temp >> 4);
				writed(temp);
				tempdata = tempdata >> 8;
			}	

			temp = tempdata&0x000000FF;		
			temp = (temp << 4)|(temp >> 4);			
			writed(0x00|(temp>>4));//0x00+第7位
			//writed(0x00);//写入空数据B，让上一周期的RG能够显示
			writed((temp&0x0F)<<4);
	#else
			for(j=0;j<4;j++)
			{
				temp = tempdata&0x000000FF;
				temp = (temp << 4)|(temp >> 4);
				writed(temp);

				tempdata = tempdata >> 8;
			}			
			writed(0x00);//写入空数据B，让上一周期的RG能够显示	
	#endif
		}

		SetLine1(line + i);
		SetColumn(column);
	}
}

/***********************************************************************************
*函数名：ShowChar()
*功能：显示8*16
*入口参数：line:行(0-9)
 		   column: 列(0-54)
           character:字符代码(标准ASCII码)
*返回参数：无
************************************************************************************/ 
void ShowChar(uint16 line,uint16 column,uint8 character)
{
	line = line << 1;
	Show98(line, column, &ASCIICode[character-0x20][0]);
	Show98(line + 1,column, &ASCIICode[character-0x20][8]);
}

void ShowChar1(uint16 line,uint16 column,uint16 character)
{
	line = line << 1;
	Show98(line, column, &ASCIICode[character-0x20][0]);
	Show98(line + 1,column, &ASCIICode[character-0x20][8]);
}
/***********************************************************************************
*函数名：ShowHZ()
*功能：显示一个16*16汉字（在屏幕上实际显示16*15或者16*18）
*入口参数：line:行(0-9)
 		   column: 列(0-53)
           hzcode:汉字编码，汉字的编码为：汉字在数组中的序号+0x90(144)
*返回参数：无
************************************************************************************/ 
void ShowHZ(uint8 line,uint8 column,uint16 hzcode)
{
	uint8 i,j;
	uint8 convertdata[162];
	//uint8 len;
	
	line = line % 10;
	line = line << 1;
	column = column % 54;
	
	//一般不能出现这种情况，在显示一行字符（汉字和字符混合）的时候会出现显示错误，但显示单个汉字时可以正确显示
	if(hzcode<0x90)
	{
		hzcode += 0x90;
	}
	
	SetLine(line);
	SetColumn(column);
	
	line = line << 3;
	for(i=0;i<16;i++)
	{
#ifdef UPDOWN_DIS
		len = UpDown_Convert(&convertdata[0],&HZTable[hzcode-0x90][i*2],2);
		for(j=0;j<len/2;j++)
		{
			writed((convertdata[2*j]<<4)|(convertdata[2*j+1]>>4));
		}
#else
		Convert(&convertdata[0], &HZTable[hzcode-0x90][i*2],2);	
		for(j=0;j<8;j++)
		{
			writed(convertdata[j]);
		}
		writed(0x00);//写入空数据，显示最后的点R,去掉屏蔽则显示18象素点
#endif		 
		SetLine1(line + i + 1);
		SetColumn(column);			
	}
	
}

/***********************************************************************************
*函数名：ShowLineHZ()
*功能：显示一行16*16汉字
*入口参数：line:行(0-9)
		   column:列（0--53）
		   number:显示的汉字个数
 		   linehzaddr:汉字代码地址
 		   len:汉字代码数组的长度
*返回参数：无
************************************************************************************/ 
void ShowLineHZ(uint8 line,uint8 column,uint16* linehzaddr,uint8 len)
{
	uint8 temp;
	uint8 i;
	//uint8 len;
	
	//line=line<<1; //lin*2
	line = line % 10;
	column = column%54; //防止负数出现
	//temp = column/5;
	temp = 54 - column;
	temp = temp / 5;
    if(len>temp)
    {
    	len = temp;    //防止要输出汉字个数大于本行剩余的列数空间，从而从头开始输入
    }   
     
    for(i=0;i<len;i++)
    {
     	ShowHZ(line,column,linehzaddr[i]);
     	column = column + 5;
    }
}
//显示一行ASCII码字符
void ShowLineChar(uint8 line,uint8 column,uint8* linecharaddr,uint8 len)
{
	uint8 temp;
	uint8 i;
	//uint8 len;
	
	//line=line<<1; //lin*2
	line = line % 10;
	column = column%54; //防止负数出现
	//temp = column/5;
	temp = 54 - column;
	temp = temp / 3;
    if(len>temp)
    {
    	len = temp;    //防止要输出字符个数大于本行剩余的列数空间，从而从头开始输入
    }   
     
    for(i=0;i<len;i++)
    {
     	ShowChar(line,column,linecharaddr[i]);
     	column = column + 3;
    }

}

void ShowLineChar1(uint8 line,uint8 column,const uint8* linecharaddr,uint8 len)
{
	uint8 temp;
	uint8 i;
	//uint8 len;
	
	//line=line<<1; //lin*2
	line = line % 10;
	column = column%54; //防止负数出现
	//temp = column/5;
	temp = 54 - column;
	temp = temp / 3;
    if(len>temp)
    {
    	len = temp;    //防止要输出字符个数大于本行剩余的列数空间，从而从头开始输入
    }   
     
    for(i=0;i<len;i++)
    {
     	ShowChar(line,column,linecharaddr[i]);
     	column = column + 3;
    }

}

void ShowLineChar2(uint8 line,uint8 column, uint16* linecharaddr,uint8 len)
{
	uint8 temp;
	uint8 i;
	//uint8 len;
	
	//line=line<<1; //lin*2
	line = line % 10;
	column = column%54; //防止负数出现
	//temp = column/5;
	temp = 54 - column;
	temp = temp / 3;
    if(len>temp)
    {
    	len = temp;    //防止要输出字符个数大于本行剩余的列数空间，从而从头开始输入
    }   
     
    for(i=0;i<len;i++)
    {
     	ShowChar(line,column,linecharaddr[i]);
     	column = column + 3;
    }

}

//显示任意长度的字符，可自动换行
void ShowAnyChar(uint8 line,uint8 column,uint8* linecharaddr,uint8 len)
{
	uint8 temp;
	uint8 i;
	
	line = line % 10;
	column = column%54; //防止负数出现

/*
	temp = 54 - column;
	temp = temp / 3;
    if(len <= temp)
    {
	    for(i=0;i<len;i++)
	    {
	     	ShowChar(line,column,linecharaddr[i]);
	     	column = column + 3;
	    }
	    return;
    }
*/    
	temp = column / 3;
	for(i=0;i<len;i++)
	{
		if(line < 10)
		{
			if(temp < 18)
			{
				ShowChar(line,3*temp,linecharaddr[i]);
				//temp++;
			}
			else
			{
				temp = 0;
				line++;
				ShowChar(line,3*temp,linecharaddr[i]);
				//temp++;	
			}
			temp++;
		}
	}
    return;

}

//显示时间数据 --- 月日时分
void ShowTimeChar(uint8 line,uint8 column,uint8* linecharaddr,uint8 len)
{
	uint8 temp;
	uint8 i;
	
	line = line % 10;
	column = column%54; //防止负数出现
	temp = 54 - column;
	temp = temp / 3;
	i=0;
	
    if(len>temp)
    {
    	len = temp;    //防止要输出字符个数大于本行剩余的列数空间，从而从头开始输入
    }   
    if(len < 8)
    {
    	ShowChar(line,column,0x30);					//月-- 十位
    	column += 3;
   	}
   	else
   	{
   		ShowChar(line,column,linecharaddr[i++]);	//月-- 十位
    	column += 3;
   	}
    ShowChar(line,column,linecharaddr[i++]);	//月-- 个位
    column += 3;
    ShowChar(line,column,0x2D);					//显示 " - "
    column += 3;
    ShowChar(line,column,linecharaddr[i++]);	//日-- 十位
    column += 3;
    ShowChar(line,column,linecharaddr[i++]);	//日-- 个位
    column += 3;   
    ShowChar(line,column,0x20);					//显示 "  "
    column += 3;
    ShowChar(line,column,linecharaddr[i++]);	//小时-- 十位
    column += 3;
    ShowChar(line,column,linecharaddr[i++]);	//小时-- 个位
    column += 3;   
    ShowChar(line,column,0x3A);					//显示 " : "
    column += 3;
    ShowChar(line,column,linecharaddr[i++]);	//分-- 十位
    column += 3;
    ShowChar(line,column,linecharaddr[i++]);	//分-- 个位
    column += 3;   

}

//显示一行ASCII码字符和汉字的混合
void ShowLineChar_HZ(uint8 line,uint8 column,const uint16* linecharaddr,uint8 len)
{
	uint8 temp;
	uint8 i;
	uint8 temp1 = 0;
	
	line = line % 10;        // 总共10行，换行功能	  
	column = column % 54;	 //总共54列，换列功能

	temp = 54 - column;		//

//begin:查看指定行能否显示完指定的len个字符，如果不能，则改变len.
	for( i = 0; i<len; i++ )	//len指每一行显示的汉字个数
	{
		if( linecharaddr[i]<0x90 )	temp1 = temp1 + 3; 	 //如果是ASCII字符
		else	temp1 = temp1 + 5; //如果是汉字
		if( temp <= temp1 )
		{
			len = i+1;
		}
	}
//end:查看指定行能否显示完指定的len个字符，如果不能，则改变len.

//循环单个显示字符		
	for( i = 0; i < len; i++ )
	{
		if( linecharaddr[i] >= 0x20 && linecharaddr[i] < 0x90 )	//显示ASCII字符
		{
			ShowChar( line, column, linecharaddr[i] );
			column = column + 3;
		}
		else if( linecharaddr[i] >= 0x90 )	  	//显示汉字
		{
			ShowHZ( line, column, linecharaddr[i] );
			column = column + 5;
		}
	}
	return;
}

/**********************************************************************************
*函数名：ReverseShow31
*功能：反显示一个3*1字块单元,前提是在COLOR MODE为64K的模式下，即LC[7：6]=10B，RRRRR-GGGGGG-BBBBB
*入口参数：line:反显示的行(0--160)
		   column:反显示的列(0--54)
*返回参数：无
*说明：COLOR MODE的设置和恢复可以放在外层调用函数中，但为了使外层函数的调用不再考虑这个设置，所以
	   放在本函数中，但缺点是设置与恢复反复被调用多次，浪费读写周期
***********************************************************************************/
void ReverseShow31(uint8 line, uint8 column)
{
	uint8 temp[2];
	uint32 i;
//	uint8 temp1;
	
	line = line % 160;
	column = column % 54;
	
	writei(0xd6);//设置COLOR MODE为64K的模式，即LC[7：6]=10B，RRRRR-GGGGGG-BBBBB
	
	SetLine1(line);
	SetColumn(column);
	temp[0] = readd();
	for(i=0;i<2;i++)
	{
		temp[i] = ~readd();
	}
	
	SetLine1(line);
	SetColumn(column);
	for(i=0;i<2;i++)
	{
		writed(temp[i]);
	}
	writei(0xd5);//恢复设置COLOR MODE为4K的模式，即LC[7：6]=01B，RRRR-GGGG-BBBB
	
}


void ReverseShow38(uint8 line,uint8 column)
{
	uint8 i;
	
	line = line << 3;
	for(i=0;i<8;i++)
	{
		ReverseShow31(line+i,column);
	}
	
}
/**********************************************************************************
*函数名：ReverseShow98
*功能：反显示一个9*8字块单元(9列8行)
*入口参数：line:反显示的行(0--19)
		   column:反显示的列(0--54)
*返回参数：无
***********************************************************************************/
void ReverseShow98(uint8 line, uint8 column)
{
	uint8 i;
	
	line = line % 20;
	column = column % 54;
	line = line << 3;
	
	for(i = 0;i < 8;i++)
	{
		ReverseShow31(line+i,column);
		ReverseShow31(line+i,column+1);
		ReverseShow31(line+i,column+2);
	}
}

/**********************************************************************************
*函数名：ReverseShowChar
*功能：反显示一个9*16字块单元(9列16行)
*入口参数：line:反显示的行(0--10)
		   column:反显示的列(0--54)
*返回参数：无
***********************************************************************************/
void ReverseShowChar(uint8 line, uint8 column)
{
//	uint8 i;
	
	line = line % 10;
	column = column % 54;
	line = line << 1;
	
//	writei(0xd6);//设置COLOR MODE为64K的模式，即LC[7：6]=10B，RRRRR-GGGGGG-BBBBB

	ReverseShow98(line,column);
	ReverseShow98(line+1,column);
	
//	writei(0xd5);//恢复设置COLOR MODE为4K的模式，即LC[7：6]=01B，RRRR-GGGG-BBBB
}

/**********************************************************************************
*函数名：ReverseShowHZ
*功能：反显示一个15*16字块单元(15列16行)
*入口参数：line:反显示的行(0--10)
		   column:反显示的列(0--54)
*返回参数：无
***********************************************************************************/
void ReverseShowHZ(uint8 line, uint8 column)
{
	uint8 i,j;
	
	line = line % 10;
	column = column % 54;
	line = line << 4;
	
//	writei(0xd6);//设置COLOR MODE为64K的模式，即LC[7：6]=10B，RRRRR-GGGGGG-BBBBB
	
	for(i=0;i<16;i++)
	{
		for(j=0;j<5;j++)
		{
			ReverseShow31(line+i,column+j);
		}
	}
		
//	writei(0xd5);//恢复设置COLOR MODE为4K的模式，即LC[7：6]=10B，RRRR-GGGG-BBBB
}

/***********************************************************************************
*函数名：ReverseShowLine
*功能：反显一行汉字
*入口函数：line:反显的行（0--9）
		  
*出口函数：无
************************************************************************************/
void ReverseShowLine(uint8 line)
{
	uint8 i;

	for(i=0;i<18;i++)
	  ReverseShowChar(line,i*3);

}

/***********************************************************************************
*函数名：ReverseShowLine_len
*功能：反显一行指定列数
*入口函数：startline:反显的行（0--9）
		   startcolumn:反显开始列（0--53）
		   columnnum:反显的列数（0--53）
*出口函数：无
************************************************************************************/
void ReverseShowLine_len(uint8 startline,uint8 startcolumn,uint8 columnnum)
{
	uint8 i,j;
	
	startline = startline % 10;
	startcolumn = startcolumn % 54;
	startline = startline << 4;
	
//	writei(0xd6);//设置COLOR MODE为64K的模式，即LC[7：6]=10B，RRRRR-GGGGGG-BBBBB
	
	for(i=0;i<16;i++)
	{
		for(j=0;j<columnnum;j++)
		{
			ReverseShow31(startline+i,startcolumn+j);
		}
	}
	
	//ReverseShowChar(startline,(startcolumn+i*3));	
}

/***********************************************************************************
*函数名：ShowPicture
*功能：显示图片--以3*1为单位
*入口函数：startline:图片开始行（0--160）
		   startcolumn：图片开始列（0--53）
		   pictureline：图片行点数 （1--160）
		   picturecolumn：图片列点数 （1--160）
		   pictureaddr：图片地址
*出口函数：无
************************************************************************************/
void ShowPicture(uint8 startline,uint8 startcolumn,uint8 pictureline,uint8 picturecolumn,uint8* pictureaddr)
{
	uint8 i,j;
	uint8 lastbits;//columnbyte
	uint8 convertdata[162];
	
	startline = startline % 160;
	startcolumn = startcolumn % 54;
	//pictureline = pictureline/8;
	//pictureline = pictureline % 160;
	//picturecolumn = picturecolumn % 160;
	
	lastbits = picturecolumn % 8;
	picturecolumn = picturecolumn >> 3;//列数为8的的倍数，即有多少个字节
	
	if(lastbits != 0) //横向扫描不会出现这种情况
	{
		picturecolumn += 1;
	}
	

	for(i = 0;i<pictureline;i++)
	{
		SetLine1(startline + i);
		SetColumn(startcolumn);
#ifdef UPDOWN_DIS
		len = UpDown_Convert(&convertdata[0],&pictureaddr[i*picturecolumn],picturecolumn);
		for(j=0;j<len/2;j++)
		{
			writed((convertdata[2*j]<<4)|(convertdata[2*j+1]>>4));
		}
#else			
		Convert(&convertdata[0],&pictureaddr[i*picturecolumn],picturecolumn);
		for(j=0;j<(picturecolumn*4);j++)
		{
		   //Show98(startline+i,startcolumn+j,pictureaddr+(i*picturecolumn+j)*8);
		   writed(convertdata[j]);  
		}
		writed(0x00);//写入空数据
#endif		
	//	SetLine1(startline + i + 1);
	//	SetColumn(startcolumn);					
	}	
}

/**********************************************************************************
*函数名：ShowRectangle
*功能：显示一个指定开始行、列和矩形长、宽的矩形
*入口函数：startline:开始行（0--160）
		   startcolumn：开始列（0--53）
		   rectangleline：矩形行点数（长度） （1--160）
		   rectanglecolumn：矩形列点数（宽度） （1--160）最好为3的倍数
*返回参数：无
***********************************************************************************/
void ShowRectangle(uint8 startline,uint8 startcolumn,uint8 rectangleline,uint8 rectanglecolumn,uint8 flag)
{
	uint8 endline,endcolumn;
	
	startline = startline % 160;
	startcolumn = startcolumn % 54;
	endline = startline + rectangleline;
	endcolumn = startcolumn*3 + rectanglecolumn;//0--160
	if(flag == TRUE)
	{
		CleanScreen(startline,startcolumn,rectangleline,rectanglecolumn/3);//大概清除矩形里面的内容
	}
	//显示矩形的两个竖线
	ShowColumn(startline,endcolumn,rectangleline);//左
	ShowColumn(startline,startcolumn*3,rectangleline);//右		
	
	//显示矩形的两个横线
	ShowLine(startline,startcolumn,rectanglecolumn);//上
	ShowLine(endline-1,startcolumn,rectanglecolumn);//下
}


/**********************************************************************************
*函数名：ShowGPRSSignal
*功能：显示一个16*16的信号强度
*入口参数：line:开始的行(0--10)
		   column:开始的列(0--54)
*返回参数：无
***********************************************************************************/
void ShowGPRSSignal(uint8 line,uint8 column)
{
	uint8 GPRSSignal;
	uint8 lu8Csq = 0;
	GPRS_RUN_STA luGPRSSta;
	uint8 lu8data1 = 0;
	uint8 lu8data2 = 0;

	ReadGprsRunSta(&luGPRSSta);
	lu8Csq = luGPRSSta.Csq;

	if((lu8Csq == 99) || (lu8Csq < 4))//对信号强度分等级。99表示不明情况。
		GPRSSignal = 0;
	else if(lu8Csq < 8)
		GPRSSignal = 1;
	else if(lu8Csq < 14)
		GPRSSignal = 2;
	else if(lu8Csq < 19)  //
		GPRSSignal = 3;
	else
		GPRSSignal = 4;
	
	line = line << 4;
	//GPRSSignal = 2;//GetGprsCSQ();//此处以后替换为得到GRPS信号强度的函数
	ShowPicture(line,column,0x10,0x10,&StateSignalPic[GPRSSignal][0]);

	lu8data1 = lu8Csq / 10;
	lu8data2 = lu8Csq % 10;
	ShowChar(line,column+6,lu8data1+0x30);
	ShowChar(line,column+9,lu8data2+0x30);
	
}

/**********************************************************************************
*函数名：ShowGC
*功能：显示信号类型 GPRS或者CDMA 16*16
*入口参数：line:开始的行(0--10)
		   column:开始的列(0--54)
*返回参数：无
***********************************************************************************/
void ShowGC(uint8 line,uint8 column)
{
	uint8 SignalType;
	/*
	if(gGprsRunSta.Connect)
	{
		SignalType = GetChannelType() + 1;//得到网络类型的函数，即当前通信通道类型
	}
	else
	{
		SignalType = 0;//空白
	}
	*/
//	SignalType = GetChannelType() + 1;//得到网络类型的函数，即当前通信通道类型
	SignalType =  1;//得到网络类型的函数，即当前通信通道类型
	line = line << 4;
	ShowPicture(line,column,0x10,0x10,&GCPic[SignalType][0]);
	 
}

/**********************************************************************************
*函数名：ShowAbnormal
*功能：显示异常情况图标 16*16
*入口参数：line:开始的行(0--10)
		   column:开始的列(0--54)
		   abnormaltype:异常类型  1-MBUS短路，2-SD卡异常。
		   
*返回参数：无
***********************************************************************************/
void ShowAbnormal(uint8 abnormaltype,uint8 line,uint8 column)
{
	uint8 i = 0;
	uint8 lu8MBusErrFlag = 0;
	uint8 lu8SDErrFlag = 0;
	
	switch(abnormaltype){
		case 1:{
			for(i=0;i<METER_CHANNEL_NUM;i++){
				if(gu8MBusShortFlag[i] == 1){
					ShowHZ( line, column, 0x01ac );  //显示MS，MBUS Short.
					column += 5;
					ShowChar(line,column,i+1+0x30);  //显示短路通道号。

					lu8MBusErrFlag = 1;

					break;    //如果有多通道短路，则只显示最先查到的，只显示一个。
				}
			}
		}
		break;
		
		case 2:{
			if(gu8SDStatus != 0){
				ShowHZ( line, column, 0x01ab );  //显示SDE.
				column += 5;
				ShowChar(line,column,gu8SDStatus+0x30);  //显示gu8SDStatus

				lu8SDErrFlag = 1;
			}
			
		}
		break;

		default:
			break;

	}
	
		if((lu8SDErrFlag==1) || (lu8MBusErrFlag==1)){
			gu8ErrFlag = 1;
		}
		else{
			gu8ErrFlag = 0;
			gu16ErrOverTime = 0;

		}
}

/**********************************************************************************
*函数名：ShowAbnormalNum
*功能：显示异常告警号 18*16 (2 ascii字符)
*入口参数：line:开始的行(0--10)
		   column:开始的列(0--54)
*返回参数：无
***********************************************************************************/
void ShowAbnormalNum(uint8 line, uint8 column)
{
	uint8 AbnormalNum[] = {0x30,0x30};
	//此处添加告警号转换和赋值给AbnormalNum
	//AbnormalNum[0] =HexToAscii(GlobalVariable.ErrCode / 10);
	//AbnormalNum[1] =HexToAscii(GlobalVariable.ErrCode % 10);
	ShowLineChar(line,column,&AbnormalNum[0],2);
}

/**********************************************************************************
*函数名：ShowSysDate
*功能：显示集中器日期，分两种格式显示
*入口参数：line:开始的行(0--10)
		   column:开始的列(0--54)
		   datetype：显示格式类型，0为YY-MM-DD，1为YY年MM月DD日
*返回参数：无
***********************************************************************************/
void ShowSysDate(uint8 line,uint8 column,uint8 datetype)
{
	//uint8 i;
	uint8 datebuf[10];
	uint8 data1,data2;
	
	line = line % 10;
	
	//暂时用自己写入的固定值(08年09月3日)，以后换作后面的从集中器读时间函数
	//datebuf[3] = 0x03;//日
	//datebuf[4] = 0x09;//月
	//datebuf[5] = 0x08;//年
	
	//UserReadDateTime(&datebuf[0]);//秒分时日月年
	ReadDateTime(&datebuf[0]);
	datebuf[4] = datebuf[4]&0x1F;//去掉星期
	
		//年
	//====ZXL 08.09.27添加年的前两个字节20
	ShowChar(line,column,0x32);
	ShowChar(line,column+3,0x30);
	column = column + 6;//为了不影响后面程序的改动，此处column值变动
	//====================================
	data1 = datebuf[5]/0x10;
	data2 = datebuf[5]%0x10;
	ShowChar(line,column,data1+0x30);
	ShowChar(line,column+3,data2+0x30);
	if(datetype == DATETYPE_CH)
	{
		ShowChar(line,column+6,0x2d);//"-"
		column = column + 9;
	}
	else
	{
		ShowHZ(line,column+6,0xfb);//"年"
		column = column + 12;
	}
		//月
	data1 = datebuf[4]/0x10;
	data2 = datebuf[4]%0x10;
	ShowChar(line,column,data1+0x30);
	ShowChar(line,column+3,data2+0x30);
	if(datetype == DATETYPE_CH)
	{
		ShowChar(line,column+6,0x2d);//"-"
		column = column + 9;
	}
	else
	{
		ShowHZ(line,column+6,0xfc);//"月"
		column = column + 12;
	}
	
		//日			
	data1 = datebuf[3]/0x10;
	data2 = datebuf[3]%0x10;
	ShowChar(line,column,data1+0x30);
	ShowChar(line,column+3,data2+0x30);
	
	if(datetype == DATETYPE_HZ)
	{
		ShowHZ(line,column+6,0xf3);//"月"		
	}		
	return;
}

/**********************************************************************************
*函数名：ShowSysTime
*功能：显示集中器时间，分两种格式显示
*入口参数：line:开始的行(0--10)
		   column:开始的列(0--54)
		   timetype：显示格式类型，0为HH:MM，1为HH:MM:SS
*返回参数：无
***********************************************************************************/
void ShowSysTime(uint8 line,uint8 column,uint8 timetype)
{
//	uint8 i;
	uint8 datebuf[10];
	uint8 data1,data2;
	
	line = line % 10;
	
	//暂时用自己写入的固定值(23点44分10秒)，以后换作后面的从集中器读时间函数
	//datebuf[0] = 0x10;
	//datebuf[1] = 0x44;
	//datebuf[2] = 0x23;
	
	//UserReadDateTime(&datebuf[0]);//秒分时日月年
	ReadDateTime(&datebuf[0]);
	datebuf[4] = datebuf[4]&0x1F;//去掉星期

//判断时钟是否正常，如果不正常则提示TimeErr.
	data1 = BCD_TO_HEX(datebuf[5]);
	if(data1 < 15){  //靠年份判断，小于15年则认为异常，没有电池或没有校时。
		ShowLineChar_HZ(line,column-6,ZCTimeErr,7);
	}
	else{	//否则显示时间。
	
		//时
		data1 = datebuf[2]/0x10;
		data2 = datebuf[2]%0x10;
		ShowChar(line,column,data1+0x30);
		ShowChar(line,column+3,data2+0x30);
		
		ShowChar(line,column+6,0x3a);//冒号
		
		//分
		data1 = datebuf[1]/0x10;
		data2 = datebuf[1]%0x10;
		ShowChar(line,column+9,data1+0x30);
		ShowChar(line,column+12,data2+0x30);
	
		
		//秒			
		if(timetype == 1)
		{
			ShowChar(line,column+15,0x3a);//冒号
			data1 = datebuf[0]/0x10;
			data2 = datebuf[0]%0x10;
			ShowChar(line,column+18,data1+0x30);
			ShowChar(line,column+21,data2+0x30);
		
		}
	}

}
/**********************************************************************************
*函数名：ShowDate
*功能：显示日期，分两种格式显示
*入口参数：line:开始的行(0--10)
		   datebuf:日 月 年
		   column:开始的列(0--54)
		   datetype：显示格式类型，0为YY-MM-DD，1为YY年MM月DD日
*返回参数：无
***********************************************************************************/
void ShowDate(uint8 line,uint8 column,uint8* datebuf,uint8 datetype)
{
	//uint8 i;
	//uint8 datebuf[10];
	uint8 data1,data2;
	
	line = line % 10;
	
	datebuf[1] = datebuf[1]&0x1F;//去掉星期
	
		//年
	//ShowChar(line,column,0x32);
	//ShowChar(line,column+3,0x30);
	//column = column + 6;//为了不影响后面程序的改动，此处column值变动
	//====================================
	data1 = datebuf[2]/0x10;
	data2 = datebuf[2]%0x10;
	ShowChar(line,column,data1+0x30);
	ShowChar(line,column+3,data2+0x30);
	if(datetype == DATETYPE_CH)
	{
		ShowChar(line,column+6,0x2d);//"-"
		column = column + 9;
	}
	else
	{
		ShowHZ(line,column+6,0xfb);//"年"
		column = column + 12;
	}
		//月
	data1 = datebuf[1]/0x10;
	data2 = datebuf[1]%0x10;
	ShowChar(line,column,data1+0x30);
	ShowChar(line,column+3,data2+0x30);
	if(datetype == DATETYPE_CH)
	{
		ShowChar(line,column+6,0x2d);//"-"
		column = column + 9;
	}
	else
	{
		ShowHZ(line,column+6,0xfc);//"月"
		column = column + 12;
	}
	
		//日			
	data1 = datebuf[0]/0x10;
	data2 = datebuf[0]%0x10;
	ShowChar(line,column,data1+0x30);
	ShowChar(line,column+3,data2+0x30);
	
	if(datetype == DATETYPE_HZ)
	{
		ShowHZ(line,column+6,0xf3);//"月"		
	}		
	return;
}

/**********************************************************************************
*函数名：ShowTime
*功能：显示任意时间，分两种格式显示
*入口参数：line:开始的行(0--10)
		   column:开始的列(0--54)
		   datebuf:秒 分 时
		   timetype：显示格式类型，0为HH:MM，1为HH:MM:SS
*返回参数：无
***********************************************************************************/
void ShowTime(uint8 line,uint8 column,uint8* datebuf,uint8 timetype)
{
//	uint8 i;
	//uint8 datebuf[10];
	uint8 data1,data2;
	
	line = line % 10;
	
		//时
		data1 = datebuf[2]/0x10;
		data2 = datebuf[2]%0x10;
		ShowChar(line,column,data1+0x30);
		ShowChar(line,column+3,data2+0x30);
		
		ShowChar(line,column+6,0x3a);//冒号
		
		//分
		data1 = datebuf[1]/0x10;
		data2 = datebuf[1]%0x10;
		ShowChar(line,column+9,data1+0x30);
		ShowChar(line,column+12,data2+0x30);
	
		
		//秒			
		if(timetype == 1)
		{
			ShowChar(line,column+15,0x3a);//冒号
			data1 = datebuf[0]/0x10;
			data2 = datebuf[0]%0x10;
			ShowChar(line,column+18,data1+0x30);
			ShowChar(line,column+21,data2+0x30);
		
		}

}

/**********************************************************************************
*函数名：ShowIniDate
*功能：显示初始化日期
*入口参数：line:开始的行(0--9)
		   column:开始的列(0--54)
		   IniDate:初始化日期数组地址
*返回参数：无
***********************************************************************************/
void ShowIniDate(uint8 line,uint8 column,uint8* IniDate)
{
	ShowChar(line,column,IniDate[0]);
	ShowChar(line,column+3,IniDate[1]);
	ShowHZ(line,column+6,0xfb);//年
	ShowChar(line,column+11,IniDate[2]);
	ShowChar(line,column+14,IniDate[3]);
	ShowHZ(line,column+17,0xfc);//月
	ShowChar(line,column+22,IniDate[4]);
	ShowChar(line,column+25,IniDate[5]);
	ShowHZ(line,column+28,0xf3);//日
}

/**********************************************************************************
*函数名：ShowIniTime
*功能：显示初始化时间
*入口参数：line:开始的行(0--9)
		   column:开始的列(0--54)
		   IniDate:初始化时间数组地址
*返回参数：无
***********************************************************************************/
void ShowIniTime(uint8 line,uint8 column,uint8* IniTime)
{

	ShowChar(line,column,IniTime[0]);
	ShowChar(line,column+3,IniTime[1]);
	ShowHZ(line,column+6,0xd1);//时
	ShowChar(line,column+11,IniTime[2]);
	ShowChar(line,column+14,IniTime[3]);
	ShowHZ(line,column+17,0xfd);//分
	ShowChar(line,column+22,IniTime[4]);
	ShowChar(line,column+25,IniTime[5]);
	ShowHZ(line,column+28,0xfe);//秒
}
void ShowTimeMS(uint8 line,uint8 column,uint8* Time)
{

	uint8 data1,data2;
	
	line = line % 10;
	
		//时
		data1 = Time[1]/0x10;
		data2 = Time[1]%0x10;
		ShowChar(line,column,data1+0x30);
		ShowChar(line,column+3,data2+0x30);
		
		ShowHZ(line,column+6,0xd1);//时
		
		//分
		data1 = Time[0]/0x10;
		data2 = Time[0]%0x10;
		ShowChar(line,column+11,data1+0x30);
		ShowChar(line,column+14,data2+0x30);
		ShowHZ(line,column+18,0xfd);//分
		

}
/**********************************************************************************
*函数名：ShowLine
*功能：显示横线，长度由columnnum规定
*入口参数：line:开始的行(0--160)
		   column:开始的列(0--54)
		   columnnum:横线的长度(0-160)
*返回参数：无
***********************************************************************************/
void ShowLine(uint8 line,uint8 column,uint8 columnnum)
{
	uint8 i;
	uint8 lastbit;
	uint8 lastnum;
	
	line = line % 160;
	
	lastbit = columnnum % 3;
	columnnum = columnnum / 3;
/*	
	if(lastbit != 0)
	{
		columnnum += 1;
	}
*/	
	lastnum = columnnum % 2;
	columnnum = columnnum/2;
	SetLine1(line);
	SetColumn(column);
	for(i = 0;i < columnnum;i++)//以6个象素点为单位进行
	{
		writed(0xff);
		writed(0xff);
		writed(0xff);
	}
#ifdef UPDOWN_DIS
	if(lastnum != 0)
	{
		writed(0xff);
		if(lastbit == 1)
		{
			writed(0xf0);
			writed(0x0f);
		}
		else if(lastbit == 2)
		{
			writed(0xf0);
			writed(0xff);
		}
		else
		{
			writed(0xf0);
		}		
	}
	else
	{
		if(lastbit == 1)
		{
			writed(0x00);
			writed(0xf0);
		}
		else if(lastbit == 2)
		{
			writed(0x0f);
			writed(0xf0);
		}		
	}

#else	
	if(lastnum != 0)//处理不足6个象素点的情况
	{
		writed(0xff);
		
		if(lastbit == 1)
		{
			writed(0xff);
			writed(0x00);
		}
		else if(lastbit == 2)
		{
			writed(0xff);
			writed(0xf0);
		}
		else
		{
			writed(0xf0);
		}

	}
	else
	{
		if(lastbit == 1)
		{
			writed(0xf0);
			writed(0x00);
		}
		else if(lastbit == 2)
		{
			writed(0xff);
			writed(0x00);
		}

	}
#endif
}

/**********************************************************************************
*函数名：ShowColumn
*功能：显示竖线，长度由linenum规定
*入口参数：line:开始的行(0--160)
		   column:开始的列(0--160)
		   linenum:横线的长度(0-160)
*返回参数：无
***********************************************************************************/
void ShowColumn(uint8 line,uint8 column,uint8 linenum)
{
	uint8 i;
	uint8 lastbit;
	//uint16 seg1,seg2,seg3;
	
	line = line % 160;
	column = column % 160;
	
	lastbit = column % 3;
	column = column / 3;
	
#ifdef UPDOWN_DIS
	if(lastbit == 0)
	{
		//seg1 = 0xf0;
		//seg2 = 0x00;
		//seg3 = 0x0f;
		for(i = 0; i < linenum; i++)
		{
			SetLine1(line + i);
			SetColumn(column);
			writed(0x00);
			writed(0xf0);
			//writed(0x00);
		}	
	}
	else if(lastbit == 1)
		{
			//seg1 = 0x0f;
			//seg2 = 0xf0;
			//seg3 = 0x00;	
			for(i = 0; i < linenum; i++)
			{
				SetLine1(line + i);
				SetColumn(column+1);
				writed(0xf0);
				writed(0x00);
				//writed(0x00);					
			}	
		}
		else
		{
			//seg1 = 0x00;
			//seg2 = 0x0f;
			//seg3 = 0xf0;
			for(i = 0; i < linenum; i++)
			{
				SetLine1(line + i);
				SetColumn(column+1);
				writed(0x0f);
				writed(0x00);
				//writed(0x0f);
			}			
		}
#else	
	if(lastbit == 0)
	{
		//seg1 = 0xf0;
		//seg2 = 0x00;
		//seg3 = 0x0f;
		for(i = 0; i < linenum; i++)
		{
			SetLine1(line + i);
			SetColumn(column);
			writed(0xf0);
			writed(0x00);
			//writed(0x00);
		}	
	}
	else if(lastbit == 1)
		{
			//seg1 = 0x0f;
			//seg2 = 0xf0;
			//seg3 = 0x00;	
			for(i = 0; i < linenum; i++)
			{
				SetLine1(line + i);
				SetColumn(column);
				writed(0x0f);
				writed(0x00);
				//writed(0x00);					
			}	
		}
		else
		{
			//seg1 = 0x00;
			//seg2 = 0x0f;
			//seg3 = 0xf0;
			for(i = 0; i < linenum; i++)
			{
				SetLine1(line + i);
				SetColumn(column);
				writed(0x00);
				writed(0xf0);
				//writed(0x0f);
			}			
		}
#endif
/*		
	for(i = 0; i < linenum; i++)
	{
		SetLine1(line + i);
		SetColumn(column);
		writed(seg1);
		writed(seg2);
		writed(seg3);		
	}	
*/
}

/**********************************************************************************
*函数名：CleanScreen
*功能：清屏幕，
*入口参数：startline:开始的行(0--160)
		   startcolumn:开始的列(0--54)
		   linenum：清掉的行数（0--160）
		   columnnum:清掉的列数(0-54)
*返回参数：无
***********************************************************************************/
void CleanScreen(uint8 startline,uint8 startcolumn,uint8 linenum,uint8 columnnum)
{
	uint8 i,j;
	startline = startline % 160;
	startcolumn = startcolumn % 54;
	
	if(columnnum%2)
	{
		columnnum = columnnum/2 + 1;
	}
	else
	{
		columnnum = columnnum/2;
	}
	
	for(i=0;i<linenum;i++)
	{
		SetLine1(startline+i);
		SetColumn(startcolumn);
		for(j=0;j<columnnum;j++)
		{
			writed(0x00);
			writed(0x00);
			writed(0x00);
		}
		
	}
}
/**********************************************************************************
*函数名：BlackScreen
*功能：将屏幕充黑，
*入口参数：startline:开始的行(0--160)
		   startcolumn:开始的列(0--54)
		   linenum：写黑的行数（0--160）
		   columnnum:写黑的列数(0-54)
*返回参数：无
***********************************************************************************/
void BlackScreen(uint8 startline,uint8 startcolumn,uint8 linenum,uint8 columnnum)
{
	uint8 i,j;
	startline = startline % 160;
	startcolumn = startcolumn % 54;
/*	
	if(columnnum%2)
	{
		columnnum = columnnum/2 + 1;
	}
	else
	{
		columnnum = columnnum/2;
	}
	
	for(i=0;i<linenum;i++)
	{
		SetLine1(startline+i);
		SetColumn(startcolumn);
		for(j=0;j<columnnum;j++)
		{
			writed(0xff);
			writed(0xff);
			writed(0xff);
		}
		
	}
*/

	for(i=0;i<linenum;i++)
	{
		SetLine1(startline+i);
		SetColumn(startcolumn);
		for(j=0;j<(columnnum/2);j++)
		{
			writed(0xff);
			writed(0xff);
			writed(0xff);
		}
		for(j=0;j<(columnnum%2);j++)
		{
			writed(0xff);
			writed(0xff);
		}
		
	}
}
/*=====================================
以下为菜单函数，可以另外创建文件
=======================================*/
void ShowMenuTop(void)
{
	CleanScreen(0,0,16,54);
	ShowGPRSSignal(0,1);	//显示信号塔标。
 	//ShowGC(0,13);	   //显示黑底Z标。
 	//ShowAbnormalNum(0,18);	   //显示00.
 	ShowAbnormal(1,0,16);	   //显示MBUS短路异常指示。
 	ShowAbnormal(2,0,25);	   //显示SD卡异常。
 	//ShowRotatePoint(0,26);
 	
 	ShowSysTime(0,38,FALSE);  //显示时间。
 	ShowLine(15,0,160);
}

void ShowMenuBottom(uint8 Tip)
{
	//Tip = 1;
	ShowPicture(144,0,16,160,&MenuBottomPic[Tip][0]);
	ShowCommunication();//显示 "已登录"或"未登录"
	ShowLine(145,0,160);
}

void ShowCommunication(void)
{
	GPRS_RUN_STA luGPRSSta;
	
	ReadGprsRunSta(&luGPRSSta);

   if(luGPRSSta.Connect)
   ShowPicture(145,41,14,36,&CommuniReg[0][0]);
   else
   ShowPicture(145,41,14,36,&CommuniReg[1][0]);

}

void ClearCommunication(void)
{
   ShowPicture(145,41,14,36,&CommuniReg[2][0]);
}


//显示开机界面
void ShowStartPic(void)
{
	display_white();
#if SC_EXPROCAL_EN == 1
	ShowPicture(28,9,85,112,&RisesunLogo_Only[0]);//图片
	ShowPicture(115,11,16,96,&SystemStart[0]);//文字
#else
	ShowPicture(28,9,104,104,&StartPic[0]);	//晨泰带中文的图标
#endif
}

void zx_test(void)
{
	//uint32 i;
//	display_black();
//   	delay(0x005ffFFF);
// for(i=0;i<10000;i++)  	
// {   	
 	display_white();
//  	ReverseShowChar(1,0);

//   	delay(0x005ffFFF);
 
	ShowMenuTop();
	ShowMenuBottom(1);

 
/* 
	//ShowChar(0,0,0x30);
	//ShowChar(0,3,0x31);
	ShowChar(0,0,0x33);
	//ReverseShow38(0,6);
	ReverseShowChar(0,0);
	//ShowChar(0,9,0x34);
	ShowChar(0,12,0x35);
	ShowChar(0,15,0x36);
    ShowChar(0,18,0x37);
	ShowChar(0,21,0x38);
	ShowChar(0,24,0x39);
	ShowChar(0,27,0x41);
	ShowChar(0,30,0x42);
	ShowChar(0,33,0x61);
	ShowChar(0,36,0x62);

	
	ShowHZ(2,0,0x90);
	ShowHZ(2,5,0x92);
	ShowHZ(2,10,0x93);
	ShowHZ(2,15,0x94);
	ShowHZ(2,20,0x95);
	ShowHZ(2,25,0x96);
	 
*/		
	//ShowLineHZ(8,0,10,LineHZ1,sizeof(LineHZ1));
//	ShowLineChar(8,0,LineChar1,sizeof(LineChar1));
/*		
	for(i=0;i<10;i++)
	{
		ReverseShowLine(i);
	}
*/	
/*	ReverseShowChar(1,0);
	ReverseShowHZ(7,0);
	ReverseShowHZ(2,5);
	ReverseShowLine(8);
*/
//	ShowPicture(16,0,64,160,&MenuPic1[0]);
		delay(0x005ffFFF);
			delay(0x005ffFFF);
//	ShowPicture(16,0,112,160,&MenuPic1_2[0]);	
		delay(0x005ffFFF);
			delay(0x005ffFFF);
//	ShowPicture(16,0,128,160,&MenuPic1_2_11[0]);
		delay(0x005ffFFF);
	CleanScreen(16,0,128,54);
//	ShowPicture(16,0,80,160,&MenuPic1_2_12[0]);	
	delay(0x005ffFFF);
		delay(0x005ffFFF);
		
//	SetLine(6);
//	SetColumn(0);
	
//	writei(0xa5); //Set All-Pixel-ON 全屏为黑色
//	writei(0xa7);  //Set Inverse Display //全屏幕取反色
//	writei(0xc8);  // Set N-Line Inversion
//	writei(0x1A);
	
//	writei(0xc8);  // Set N-Line Inversion
//	writei(0x1d);	
//	writei(0xa7);  //Set Inverse Display //全屏幕取反色
	
	delay(0x005ffFFF);
	delay(0x005ffFFF);
//  }	
/*	
    delay(0x005ffFFF);
   	delay(0x005ffFFF);
   	delay(0x005ffFFF);
   	delay(0x005ffFFF);
   	delay(0x005ffFFF);
   	
   	vertical();
    delay(0x005fFFFF);

	vertical1();
    delay(0x005fFFFF);
      
      
	horizontal2();
    delay(0x005fFFFF);
      

    snow();
    delay(0x005fFFFF);

    network();
    delay(0x005fFFFF);

    frame();
    delay(0x005fFFFF);
 */   
 
 
}

uint8 CheckDateAndTime(uint8* DTBcd)
{
	uint8 month1;
	uint8 day1;
	uint8 days[13];
	uint8 i;
	
	month1 = DTBcd[3]&0x1F;
	day1 = DTBcd[2];
	
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
	
	for(i=0;i<4;i++){
		if(BcdCheck(DTBcd[i])==FALSE){
			return (1);
		}
	}
	
	if(BcdToHex(DTBcd[0])>60){
		return 3;
	}
	if(BcdToHex(DTBcd[1])>23){
		return 4;
	}
	
	if( (month1 == 0) || (month1>12) )
	{
		return 6;
	}
	
	if( month1!= 2)
	{
		if ( (day1 > days[month1]) || (day1 ==0) )
		return 5;
	}
	
	if(month1 == 2)
	{
		if(day1 > 29)
		{
			return 6;
		}
	}
	
	return (0);//日期六个字节全为合法BCD码，并在合法取值范围内
}
