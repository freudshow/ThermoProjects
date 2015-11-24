#ifndef        _LCD_DRIVER_H
#define		   _LCD_DRIVER_H

#define   LDM_R_ADDR  0x60000000
#define   LDM_D_ADDR  0x60000001

#define LCM_LIGHT_ENABLE	GPIO_SetBits(GPIOG , LCM_LIGHT)
#define LCM_LIGHT_DISABLE	GPIO_ResetBits(GPIOG , LCM_LIGHT)

#define	  DATETYPE_CH		0  //日期显示格式：YY-MM-DD
#define	  DATETYPE_HZ	    1  //日期显示格式：YY年MM月DD日
#define   COLUMN_OFFSET		0x26//液晶显示中，列偏移的大小，即从第几列作为边缘的开始

//#define   UPDOWN_DIS			//倒立显示标记
#define   LCD_CON_VALUE		0x96 //液晶对比度默认值
#define   BLIGHT_DOG_CNT_THRD 60

//#ifndef  IN_LCMDRV
#define 		KEY_MSG_SIZE    	6 //键盘消息队列的指针数组的大小
extern OS_EVENT *KeyMessageQ;
extern OS_EVENT *LcmSem;
extern BLT_CTR gBltCtr;

extern uint8 GetKeyStatus(void);
extern uint8 GetKeyStatuschaobiao(void);

 void display_black(void);

 void delay(uint32 tt);
 void display_black(void);
 void display_white(void);

 void vertical(void);
 void horizontal(void);
 void vertical1(void);
 void horizontal1(void);
 void horizontal2(void);

 void snow(void);
 void frame(void);
 void window_program(void);
 void lcd_initial(void);
 void network(void);

 void writei(unsigned char ins);
 void writed(unsigned char ins);

 uint8 readi(void);
 uint8 readd(void);


 void zx_test(void);
//#endif

//驱动
 void SetContrast(uint8 ContrastValue);//功能：设置液晶对比度
 void SetLine1(uint16 line); // 0<=line<=159    1 line = 1 row
 void SetLine(uint16 line);
 void SetColumn(uint16 column);
 void Show98(uint16 line,uint16 column,const uint8 addr[]);
 void ShowChar(uint16 line,uint16 column,uint8 character);
 void ShowChar1(uint16 line,uint16 column,uint16 character);

 void ShowHZ(uint8 line,uint8 column,uint16 hzcode);
 void ShowLineHZ(uint8 line,uint8 column,uint16* linehzaddr,uint8 len);
 void ShowLineChar(uint8 line,uint8 column,uint8* linecharaddr,uint8 len);
void ShowLineChar1(uint8 line,uint8 column,const uint8* linecharaddr,uint8 len);

void ShowLineChar2(uint8 line,uint8 column, uint16* linecharaddr,uint8 len);

 void ShowTimeChar(uint8 line,uint8 column,uint8* linecharaddr,uint8 len);
 void ShowLineChar_HZ(uint8 line,uint8 column,const uint16* linecharaddr,uint8 len);
 void ShowAnyChar(uint8 line,uint8 column,uint8* linecharaddr,uint8 len);
 void ReverseShow31(uint8 line, uint8 column);
 void ReverseShow38(uint8 line,uint8 column);
 void ReverseShow98(uint8 line, uint8 column);
 void ReverseShowChar(uint8 line, uint8 column);
 void ReverseShowHZ(uint8 line, uint8 column);
 void ReverseShowLine(uint8 line);
 void ReverseShowLine_len(uint8 startline,uint8 startcolumn,uint8 columnnum);
 void ShowPicture(uint8 startline,uint8 startcolumn,uint8 pictureline,uint8 picturecolumn,uint8* pictureaddr);
 void ShowRectangle(uint8 startline,uint8 startcolumn,uint8 rectangleline,uint8 rectanglecolumn,uint8 flag);
 void ShowGPRSSignal(uint8 line,uint8 column);
 void ShowGC(uint8 line,uint8 column);
 void ShowAbnormal(uint8 abnormaltype,uint8 line,uint8 column);
 void ShowLine(uint8 line,uint8 column,uint8 columnnum);
 void ShowColumn(uint8 line,uint8 column,uint8 linenum);//显示竖线
 void ShowSysTime(uint8 line,uint8 column,uint8 timetype);
 void ShowSysDate(uint8 line,uint8 column,uint8 datetype);
 void ShowDate(uint8 line,uint8 column,uint8* datebuf,uint8 datetype);
 void ShowTime(uint8 line,uint8 column,uint8* datebuf,uint8 timetype);
 void ShowIniDate(uint8 line,uint8 column,uint8* IniDate);//显示初始化日期
 void ShowIniTime(uint8 line,uint8 column,uint8* IniTime);//显示初始化时间
 void ShowTimeMS(uint8 line,uint8 column,uint8* Time);
 void CleanScreen(uint8 startline,uint8 startcolumn,uint8 linenum,uint8 columnnum);//清屏函数
 void BlackScreen(uint8 startline,uint8 startcolumn,uint8 linenum,uint8 columnnum);//写黑屏幕函数
 //
 void ShowMenuTop(void);//液晶顶部状态显示
 void ShowMenuBottom(uint8 Tip);//液晶底部状态显示
 void ShowStartPic(void); //显示开机界面
 void ShowCommunication(void);//显示集中器（GPRS）已连接上主站
 extern void ClearCommunication(void);
 uint8 CheckDateAndTime(uint8 *DTBcd);	//检验时间的有效性

#endif
