#ifndef				_RX8025_H
#define				_RX8025_H				
#ifdef   _MBUS_DETACH_
#define		RX8025_ADDR 			0x64
#endif
#define		I2C_MSG_GRP_SIZE		32

#define I2C_MSG_CMD_RD			0x01 //请求读出RX8025从地址0到15，除地址13外的所有字节
#define I2C_MSG_CMD_SD			0x02 //请求设置日期时间
#define I2C_MSG_CMD_INIT_SD		0x03 //请求设置日期时间并初始化8025

typedef struct {
	uint8 cmd;
	uint8 datbuf[I2C_MSG_GRP_SIZE];
} I2C_MSG;

extern	OS_EVENT *I2cReqSem; //申请向I2cReqMbox发消息前必须先获得此信号量，直到收到I2cAnswMbox才释放该信号量
extern	OS_EVENT *I2cReqMbox; //申请使用I2c的邮箱
extern	OS_EVENT *I2cAnswMbox; //I2c返回给申请任务使用的邮箱

//=============只能由TaskI2cManagement调用的函数
extern void Rd8025(uint8* retbuf);
extern void Init8025(void);
extern void RX8025_ClearXST(void);
extern void WrDt8025(uint8* datebuf);
//=============只能由TaskI2cManagement调用的函数


//返回值 ： 0表示成功；非0表示失败
//retbuf： 返回的日期时间，秒-年
extern	uint8 UserReadDateTime(uint8* retbuf);
//RX8025_EXT	uint8 UserReadDateTime1(uint8* retbuf);

//datebuf： 秒-年
//返回值 ： 0表示成功；非0表示失败
extern	uint8 UserSetDateTime(uint8* datebuf);
extern uint8 UserSetDateTime(uint8* datebuf);


extern u8 clock_state ;
//RX8025_EXT	uint8 UserSetDateTime_1(uint8* datebuf); //for test


//=================ncq828
//RX8025_EXT void Recover8025_PostPend(uint8* DateTime_Rec,uint8* DateTime_Bak); 
//////RX8025_EXT void Recover8025(void);
//=================ncq828


//RX8025_EXT uint8 CheckDateTime(uint8* DateTime);//ncq1006


#endif
