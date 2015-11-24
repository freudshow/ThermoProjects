

#include <includes.h>

u8 clock_state = TRUE;

#define		RX8025_ADDR 			0x64
#define		TRAN_MODE				0
#define		CVT_8025_RA(reg_a)		((reg_a << 4) | TRAN_MODE)

OS_EVENT *I2cReqSem; //申请向I2cReqMbox发消息前必须先获得此信号量，直到收到I2cAnswMbox才释放该信号量
OS_EVENT *I2cReqMbox; //申请使用I2c的邮箱
OS_EVENT *I2cAnswMbox; //I2c返回给申请任务使用的邮箱

extern uint8 I2C_ReadByte(uint8* pBuffer, uint8 length, uint8 ReadAddress, uint8 DeviceAddress);
extern uint8 I2C_Write(uint8 *pSendByte, uint8 SendNums, uint8 DeviceAddress);

//extern uint32 gRx8025tstn[];

//返回8025地址0x0e开始的2个字节  
void __Rd8025CR(uint8* retbuf)
{
	uint8 tmp,num;
	
	num=2;
	while(1){
	
		retbuf[0]=CVT_8025_RA(0x0e);//字地址
		tmp=I2C_ReadByte(retbuf, num, retbuf[0], RX8025_ADDR);//1:字地址是1个字节，要读num个字节数据

    	if(tmp==1){
    		break;
    	}
    	else{
    		//gRx8025tstn[0]++;
    		//OSTimeDly(OS_TICKS_PER_SEC/10); 
    	}
    }
	//OSTimeDly(2);
}

//返回8025从地址0到15，除地址13外的所有字节
void Rd8025(uint8* retbuf)
{
	uint8 tmp,num;
	uint8 datbuf[17];  //多读一个测试用。
	num = 17;
	//num = 7;			//只读7个字节
	while(1){
	
		datbuf[0]=CVT_8025_RA(0);//字地址
		tmp = I2C_ReadByte(datbuf, num, datbuf[0], RX8025_ADDR);//1:字地址是1个字节，要读num个字节数据

    	if(tmp==1){
				memcpy(retbuf,datbuf,17);
	
    		break;
    	}
    	else{
				clock_state = FALSE;
				break;
    		//gRx8025tstn[1]++;
    		//OSTimeDly(OS_TICKS_PER_SEC/10); 
    	}
    }
	
	//OSTimeDly(2);
	 		//__Rd8025CR(&datbuf[13]);
			//memcpy(retbuf,datbuf,15);
}

void Wr8025(uint8* buf,uint8 n)
 {
	uint8 tmp;
	
	while(1){
			tmp=I2C_Write(buf, n, RX8025_ADDR);
    	
    	if(tmp==1){
    		
    		break;
    	}
    	else{
				clock_state = FALSE;
				break;
    		//gRx8025tstn[2]++;
    		//OSTimeDly(OS_TICKS_PER_SEC/10); 
    
    	}
    }
	//OSTimeDly(2);
}
void Init8025(void)
{
	uint8 iicbuf[10];
		
	iicbuf[0] = CVT_8025_RA(0x07);//字地址
	iicbuf[1] = 0x00; //Digital Offset bit7= 0("务必写0的位"); F6-F0=0
	iicbuf[2] = 0x00; //Alarm_W:Minute
	iicbuf[3] = 0x00; //Alarm_W:Hour
	iicbuf[4] = 0x00; //Alarm_W:Weekday
	iicbuf[5] = 0x00; //Alarm_D:Minute
	iicbuf[6] = 0x00; //Alarm_D:Hour

	Wr8025(iicbuf,7);//写7个字节的数据(包括字地址在内)
	
	iicbuf[0] = CVT_8025_RA(0x0e);//字地址
	iicbuf[1] = 0x23; //control1: WALE=0;DALE=0;/12,24 = 1,BIT4=0;TEST=0;CT2CT1CT0=3(/INTA输出1HZ方波，秒脉冲输出)
	iicbuf[2] = 0x20; //control2: VDSL =0 (电源降低检测功能的标准电压值设定为2.1V); VDET=0;/XST=1;
	                         //PON = 0; BIT3=0;CTFG=0;WAFG=0;DAFG=0;
	Wr8025(iicbuf,3);//写3个字节的数据(包括字地址在内)
}

//检测到邋RX8025有震动停止，清楚该位。added by zjjin.
void RX8025_ClearXST(void)
{
	uint8 iicbuf[10];

	iicbuf[0] = CVT_8025_RA(0x0e);//字地址
	iicbuf[1] = 0x23; //control1: WALE=0;DALE=0;/12,24 = 1,BIT4=0;TEST=0;CT2CT1CT0=3(/INTA输出1HZ方波，秒脉冲输出)
	iicbuf[2] = 0x20; //control2: VDSL =0 (电源降低检测功能的标准电压值设定为2.1V); VDET=0;/XST=1;
						 //PON = 0; BIT3=0;CTFG=0;WAFG=0;DAFG=0;
	Wr8025(iicbuf,3);//写3个字节的数据(包括字地址在内)

}


void WrDt8025(uint8* datebuf)
{
	uint8 iicbuf[10];
	
	iicbuf[0] = CVT_8025_RA(0);//字地址
	iicbuf[1] = datebuf[0]; //秒
	iicbuf[2] = datebuf[1]; //分
	iicbuf[3] = datebuf[2]; //时
	iicbuf[4] = datebuf[4]>>5;			//星期
	iicbuf[5] = datebuf[3]; //日
	iicbuf[6] = datebuf[4]&0x1f; //月
	iicbuf[7] = datebuf[5]; //年
	
	Wr8025(iicbuf,8); //写8个字节的数据(包括字地址在内)
}


//请求读出RX8025从地址0到15，除地址13外的所有字节
void Rd8025_PostPend(uint8* retbuf) //可重入
{
	uint8 err;
	I2C_MSG* cp;
	I2C_MSG I2cMsg;
	
	OSSemPend(I2cReqSem, 0, &err);
	
	I2cMsg.cmd = I2C_MSG_CMD_RD;//表示读操作;

	OSMboxPost(I2cReqMbox,(void *)(&I2cMsg));
	cp = (I2C_MSG *)OSMboxPend(I2cAnswMbox, 0, &err); //死等
	if(cp!=&I2cMsg){
		while(1);
	}
	memcpy(retbuf,cp->datbuf,17);
	
	OSSemPost(I2cReqSem);
}

uint8 SetDateTime_PostPend(uint8* datebuf,uint8 init) //可重入
{
	uint8 err;
	I2C_MSG* cp;
	I2C_MSG I2cMsg;
	
	OSSemPend(I2cReqSem, 0, &err);

	if(init==TRUE){
		I2cMsg.cmd = I2C_MSG_CMD_INIT_SD;
	}
	else{
		I2cMsg.cmd = I2C_MSG_CMD_SD;
	}
	
	memcpy(I2cMsg.datbuf,datebuf,6);
	
	OSMboxPost(I2cReqMbox,(void *)(&I2cMsg));
	cp = (I2C_MSG *)OSMboxPend(I2cAnswMbox, 0, &err); //死等
	//说明，在上一行断点的话会导致 一次IIC错误， 因为运行到这里是由于IIC读写函数因PENDIIC中断服务程序发的信号量而挂起IIC管理任务，也就是说这里断点刚好打断了IIC总线进程
	if(cp!=&I2cMsg){
		while(1);
	}
	
	OSSemPost(I2cReqSem);
	return (0);
}

uint8 UserSetDateTime(uint8* datebuf)  //可重入
{
	//日期时间合法性判断
	if(CheckDT(datebuf)){
		return 1;
	}
	
	SetDateTime_PostPend(datebuf,TRUE);
	
	TIME_UpdateSystemTime();
	
	return (0);
} 
/*
uint8 UserSetDateTime_1(uint8* datebuf)  //可重入
{
	//日期时间合法性判断
	if(CheckDT(datebuf)){
		return 1;
	}
	
	SetDateTime_PostPend(datebuf,FALSE);
	return (0);
}*/

uint8 UserReadDateTime(uint8* retbuf)  //可重入
{
//	uint8 err = 0;
	uint8  datebuf[17] = {0};

	Rd8025_PostPend(datebuf);

	memcpy(retbuf,datebuf,3);
	memcpy(&retbuf[3],&datebuf[4],3);
	return 0;
}
/*
//星期（月的高三位）为 1-7 （7为星期日）
uint8 UserReadDateTime1(uint8* retbuf)  //可重入
{
	uint8  datebuf[16];

	Rd8025_PostPend(datebuf);

	memcpy(retbuf,datebuf,3);
	memcpy(&retbuf[3],&datebuf[4],3);
		
	retbuf[4]&=0x1f;
	if(datebuf[3]==0){ //星期
		datebuf[3]=7;
	}
	retbuf[4]|=(datebuf[3]<<5);
	return 0;
}

//=================ncq828
void Recover8025_PostPend(uint8* DateTime_Rec,uint8* DateTime_Bak) 
{
	
	//ncq1006 屏蔽原来if(DateTime_Rec[5]==0x07||DateTime_Rec[5]==0x08){
	if(CheckDateTime(DateTime_Rec)==TRUE){//ncq1006
		UserSetDateTime(DateTime_Rec);
	}
	else{
		UserSetDateTime(DateTime_Bak);
		
	}

	
}

void Recover8025(void) 
{
	uint8 iicbuf[5];
	iicbuf[0] = 0x00;//字地址  
	iicbuf[1] = 0x00;  //TEST1 TESTC  STOP  SET TO 0   
	Wr8025(iicbuf,2);//写2个字节的数据(包括字地址在内)  
	OSTimeDly(2);
}*/
//=================ncq828


//========ncq1006
/*
uint8 CheckDateTime(uint8* DateTime)
{
	if(DateTime[5]<0x07||DateTime[5]>0x19){
		return FALSE;	
	}
	else{
		return TRUE;
	}
} */
//========ncq1006

