#define MAX_IPSEND_GTM900C_BYTENUM 	1024  //GTM900C一次发送(AT%IPSEND)最多为1024字节(引号里是2048个可显字符)
#define MAX_IPDATA_GTM900C_BYTENUM	2048  
//测试先是发现900C接收到IPDATA后发过来的最大包是1072字节(多了会按1072拆成多个IPDATA发过来),但后来又发现会出现大于1072的情况
//所以这个值搞不清,字节定义了一个大值,原来900B好像是按1400拆包的
//
#define MAX_IPSEND_MC52i_BYTENUM	1500  //MC52i一次发送(AT^SISW)最多为1500字节,二进制格式发送
#define MAX_REC_MC52i_BYTENUM	1500  //MC52i一次最多可读来自网络的数据也是1500字节,不过它存在自己一个比1500大的多得缓冲区里,等CPU去要

//宏定义支持的APN数量，当添加APN接入点支持时，要同时修改下面的Ats_SICS_apn[][]和Ats_CGDCONT[][].
#define    SUPPORT_APN_NUM	3  



typedef struct {
	uint8  ModuId;//MODU_ID_HW or MODU_ID_XMZ
	uint8  Ready;//TRUE or FALSE
	uint8  Connect;////TRUE or FALSE
	uint8  SocketType;//0:TCP;1:UDP
	uint8  IpCloseNum;//TRUE or FALSE 
	uint8  IpSendFailNum;
	uint8  IpSendRemnBufNum;//上一次IPSEND时GPRS模块的剩余缓冲区数量
	uint8  Csq;//信号强度
	
	uint32 FrmRecTimes;//接收帧次数 //测试观察用
	uint32 FrmSndTimes;//发送帧次数 //测试观察用
	
	uint32 FrmRecBs;//接收字节数
	uint32 FrmSndBs;//发送字节数
	
	uint32  RecDog; //监视GPRS接收的看门狗
	uint32  SndDog; //监视GPRS发送的看门狗
} GPRS_RUN_STA;


#define MODU_ID_UNKNOWN		0x55
#define MODU_ID_HW			0x12
#define MODU_ID_XMZ			0x21

#define LC_PORT_MIN		10000
#define LC_PORT_RANGE  	10000
#ifndef IN_GPRS
extern OS_EVENT *GprsXmzSem;

extern uint8 gStartTime[];
extern uint16  RandFirRand16(void);
extern uint8 nAsciiToHex(char* str);
extern uint8 ModelIdentify(void);
extern uint8 GprsInit_hw(void);
extern uint8 GprsInit_xmz(void);
extern uint8 CMD_AT_RP(char* AtComd,char* AtRightAnsw,char* OrAtRightAnsw,uint16 OutTime,uint32 rp,uint8 ATEMode);//muxiaoqing test

extern uint8 GprsGetIPDATA_xmz(char* ipdata,uint16 OutTime,uint16* StrLen);
extern uint8 GprsGetIPDATA_jh(char* ipdata,uint16 OutTime,uint16* StrLen);
extern uint8 GprsIPSEND_hw(uint8* ipdata,uint16 len,uint16* pBufNum);
extern uint8 GprsIPSEND_xmz(uint8* ipdata,uint16 len,uint16* pBufNum);
extern uint8 CMD_IpClose_hw(void);
extern uint8 CMD_IpClose_xmz(void);
extern uint8 GprsGetIPDATA(char* ipdata,uint16 OutTime,uint16* StrLen);
extern uint8 GprsIPSEND(uint8* ipdata,uint16 len,uint16* pBufNum);
extern uint8 CMD_IpClose(void);
extern uint8 CMD_Csq(uint8* csq);


extern void InitGprsRunSta(void);
extern void ReadGprsRunSta(GPRS_RUN_STA* Sta);
extern uint8 ReadCurSType(void);
extern void UpdGprsRunSta(GPRS_RUN_STA* Sta);
extern void UpdGprsRunSta_Ready(uint8 val);
extern void UpdGprsRunSta_IpCloseNum(uint8 val);
extern void UpdGprsRunSta_IpSendRemnBufNum(uint8 val);
extern void UpdGprsRunSta_Csq(uint8 val);
extern void UpdGprsRunSta_AddIpCloseNum(void);
extern void UpdGprsRunSta_AddIpSendFailNum(void);
extern void UpdGprsRunSta_FeedRecDog(void);
extern void UpdGprsRunSta_AddRecDog(void);
extern void UpdGprsRunSta_FeedSndDog(void);
extern void UpdGprsRunSta_AddSndDog(void);
extern void UpdGprsRunSta_AddFrmRecTimes(uint32 n);
extern void UpdGprsRunSta_AddFrmSndTimes(uint32 n);
extern void UpdGprsRunSta_Stype(uint8 val);
extern void UpdGprsRunSta_Cont(uint8 val);
extern void UpdGprsRunSta_ModuId(uint8 val);
extern uint8 GetGprsRunSta_ModuId(void);


//#if TEST_SELF_CHECK > 0
//extern uint8 SetXmzBsp(void);
//#endif


extern uint8 GetGprsCSQ(void);//获取GPRS信号强度  返回值范围为0(小)--4(大)，分别表示GPRS的强度大小



#endif

