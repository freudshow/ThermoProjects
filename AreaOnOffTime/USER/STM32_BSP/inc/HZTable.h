#ifndef			_HZTABLE_H
#define			_HZTABLE_H

extern uint8 LineHZ1[] ; //西安晨泰科大科技研
extern uint8 LineChar1[] ;//1 //西安晨泰科大科

//命名规则：第一个数字表示当前菜单的级别，第2个数字表示当前菜单下的第几项，下划线后面的数字表示下一级菜单的级别，第2个数字表示当前菜单
//下的第几项，依次类推

//一级菜单
extern uint16 MenuHZ11[] ;//测量点数据显示
extern uint16 MenuHZ12[] ;//参数设置与查看
extern uint16 MenuHZ13[] ;//集中器管理与维护
extern uint16 MenuHZ14[] ;//时间通断面积法
extern uint16 MenuHZ15[] ;//选择计量方法
extern uint16 MenuHZ16[] ;//维护 实际应该写成出厂测试muxiaoqing 20140328


//二级菜单
//暂时缺少1级菜单下第一项的二级菜单
extern uint16 TestPoint1[];//测量点1
extern uint16 TestPoint2[];//测量点2
extern uint16 TestPoint3[];//测量点3
extern uint16 TestPoint4[];//测量点4
extern uint16 TestPoint5[];//测量点5
extern uint16 TestPoint6[];//测量点6
extern uint16 TestPoint7[];//测量点7
extern uint16 TestPoint8[];//测量点8

extern uint16 MenuHZ12_21[];//1.通信通道设置
extern uint16 MenuHZ12_22[];//立即抄表
extern uint16 MenuHZ11_22[];//2.台区电表参数设置
extern uint16 MenuHZ12_23[];//3.集抄电表参数设置
extern uint16 MenuHZ12_24[];  //失败信息查询

extern uint16 MenuHZ11_24[] ;//4.终端时间设置
extern uint16 MenuHZ12_25[] ;//5.界面密码设置
extern uint16 MenuHZ11_26[] ;//6.终端编号设置
extern uint16 MenuHZ11_27[] ;//7.抄表时间点设置
extern uint16 MenuHZ11_28[] ;//GPRS参数设置
extern uint16 MenuHZ11_29[] ;//IP及端口号设置


extern uint16 MenuHZ_TimeNum[] ;//第  抄表时间点:
//extern uint16 MenuHZ12_27[] ;//轮显设置
extern uint16 MenuHZ12_28[] ;//8.测量点设置
extern uint16 MenuHZ12_29[] ;//9.界面密码设置
extern uint16 MenuHZ12_2A[];//10.远程通信方式设置
extern uint16 MenuHZ12_2B[];//11.测量点参数查看
extern uint16 PswStar[] ;//******

extern uint16 MenuHZ13_21[];//重启终端
extern uint16 MenuHZ13_22[] ;//数据初始化
extern uint16 MenuHZ13_23[] ;//参数初始化
extern uint16 MenuHZ13_24[] ;//4.轮显参数设置
extern uint16 MenuHZ13_25[] ;//5.终端版本信息
extern uint16 MenuHZ13_26[] ;//6.液晶对比度调节
extern uint16 MenuHZ13_27[] ;//7.其他参数设置

extern uint16 MenuHZ15_1[];//热量法
extern uint16 MenuHZ15_3[];//热分配表法
extern uint16 MenuHZ15_4[];//当前计量方法
extern uint16 MenuHZ16_21[] ;//手动测试muxiaoqing 20140328
extern uint16 MenuHZ16_22[] ;//自动测试muxiaoqing 20140328

#if 0
extern uint16 MenuHZ16_21[] ;//"SD 卡数据写入和读出对比测试"
extern uint16 MenuHZ16_22[] ;//"时钟芯片的写入和读出对比测试"
extern uint16 MenuHZ16_23[] ;//"GPRS 指令测试"
extern uint16 MenuHZ16_24[] ;//"调试串口命令测试"
extern uint16 MenuHZ16_25[] ;//"抄表串口1命令测试"
extern uint16 MenuHZ16_26[] ;//"GPRS通讯命令测试"
extern uint16 MenuHZ16_27[] ;//"RS485 通讯1命令测试"
extern uint16 MenuHZ16_28[] ;//"RS485 通讯2命令测试"
#else
#ifdef DEMO_APP
extern uint16 MenuHZ16_21_31[] ;//"德鲁"
extern uint16 MenuHZ16_21_32[] ;//"兰吉尔"
extern uint16 MenuHZ16_21_33[] ;//"天罡"
extern uint16 MenuHZ16_21_34[] ;//"预留"
extern uint16 Meter_ST_OK_1[] ;//"德鲁抄表: 成功"
extern uint16 Meter_ST_OK_2[];//"兰吉尔抄表: 成功"
extern uint16 Meter_ST_OK_3[];//"天罡抄表: 成功"
extern uint16 Meter_ST_FAL_1[];//"德鲁抄表:失败"
extern uint16 Meter_ST_FAL_2[];//"兰吉尔抄表:失败"
extern uint16 Meter_ST_FAL_3[];//"天罡抄表:失败"
extern uint16 Meter_ST_WAIT[] ;//"请稍候..."
extern uint16 Meter_ST_AIM[] ;//"按向下键进入数据显示页面"
extern uint16 Meter_ST_AIM_CHeat[] ;//"当前热量:"
extern uint16 Meter_ST_AIM_Power[];//"热功率:"
extern uint16 Meter_ST_AIM_Flow[];//"流量:"
extern uint16 Meter_ST_AIM_CFlow[];//"累积流量:"
extern uint16 Meter_ST_AIM_INwater[];//"供水温度:"
extern uint16 Meter_ST_AIM_Owater[];//"回水温度:"
extern uint16 Meter_ST_Building[] ;//"楼号:"
extern uint16 Meter_ST_Unit[];//"单元:"
extern uint16 Meter_ST_MeterNum[];//"表号:"
extern uint16 Meter_ST_CH[];//"通道:"
extern uint16 IPVer[]  ;//协议号

#else
extern uint16 MenuHZ16_21_31[] ;//"SD 卡数据写入和读出对比测试"
extern uint16 MenuHZ16_21_32[] ;//"时钟芯片的写入和读出对比测试"
extern uint16 MenuHZ16_21_33[] ;//"GPRS 指令测试"
extern uint16 MenuHZ16_21_34[] ;//"调试串口命令测试"
extern uint16 MenuHZ16_21_35[] ;//"抄表串口1命令测试"
extern uint16 MenuHZ16_21_36[] ;//"GPRS通讯命令测试"
extern uint16 MenuHZ16_21_37[] ;//"RS485 通讯1命令测试"
extern uint16 MenuHZ16_21_38[] ;//"RS485 通讯2命令测试"
#endif
#endif



//三级菜单

extern uint16 MenuHZ11_21_31[] ;//当前电量
extern uint16 MenuHZ11_21_32[] ;//表序号:
/*
extern uint16 MenuHZ12_21_31[] ;//1.当前通道类型设置
extern uint16 MenuHZ12_21_32[] ;//2.主IP和端口设置
extern uint16 MenuHZ12_21_33[] ;//3.备IP和端口设置
extern uint16 MenuHZ12_21_34[] ;//4.APN设置
extern uint16 MenuHZ12_21_35[] ;//5.远程通信方式设置
*/
extern uint16 MenuHZ12_21_33[] ;
extern uint16 MenuHZ12_21_31[] ;//1.远程通信方式设置
extern uint16 MenuHZ12_21_32[] ;//2.当前通道类型设置
extern uint16 MenuHZ12_21_33[] ;//3.配置类型选择
extern uint16 MenuHZ12_21_34[] ;//详细设置

extern uint16 MenuHZ12_22_31[] ;//测量点号：
extern uint16 MenuHZ12_22_32[] ;//电表地址：
extern uint16 MenuHZ12_22_33[] ;//电表类型：
extern uint16 MenuHZ12_22_34[] ;//采集器号：
extern uint16 MenuHZ12_22_35[] ;//通信方式：
extern uint16 MenuHZ12_22_36[] ;//多功能总表
extern uint16 MenuHZ12_25_31[] ;//旧密码：
extern uint16 MenuHZ12_25_32[] ;//新密码：

extern uint16 MenuHZ12_27_31[] ;//1.最大载波延时设置
extern uint16 MenuHZ12_27_32[] ;//2.电表数量查看
extern uint16 MenuHZ12_27_33[] ;//3.抄表状态查看
extern uint16 MenuHZ12_27_34[] ;//4.采集数据项设置

extern uint16 MenuHz_DeviceNum[] ;//设备编号:
extern uint16 MenuHz_ProtocalType[] ;//规约:
extern uint16 MenuHz_BigTypeNo[] ;//大类号:
extern uint16 Menu_BigTypeNo[][3];//大类号选项内容：1--6
extern uint16 MenuHz_CommPort[] ;//通信端口号:
extern uint16 MenuHz_BaudRate[] ;//波特率：
extern uint16 Menu_BaudRate[][5] ;//波特率选项内容

extern uint16 MenuHZTesting[];//muxiaoqing test
extern uint16 MenuHZTestingPleaseWaite[];//muxiaoqing test

extern uint16 SdErr[];
extern uint16 ClockErr[];
extern uint16 ClockErrr[];
extern uint16 ClockErrrr[];
extern uint16 ClockErrrrr[];
extern uint16 ClockErrrrrr[];

extern uint16 chose_user_display[];//选择查看对象
extern uint16 MenuHZ_user[];//第  用户

extern uint16 MenuHZ_user_num[];//用户编号
extern uint16 MenuHZ_device_addr[];//设备地址
extern uint16 MenuHZ_user_area[];//用户面积
extern uint16 MenuHZ_open_time[];//开阀时间
extern uint16 MenuHZ_indoor_temp[];//室内温度
extern uint16 MenuHZ_water_temp_in[];//进水温度
extern uint16 MenuHZ_water_temp_out[];//回水温度
extern uint16 MenuHZ_proportion_energy[];//分摊热量
extern uint16 MenuHZ_total_energy[];//总热量

extern uint16 Sdtest[];//muxiaoqing test

//600
//1200
//2400
//4800
//7200
//9600
//19200
extern uint16 MenuHz_RateNum[] ;//费率数:
extern uint16 Menu_RateNum[][3] ;//费率数选项内容：1--4

extern const uint16 MeterStateInfo[][9];
extern const uint16 MeterState[][7];

extern uint16 InputPsw[] ;//请输入密码：
extern uint16 MenuHZ12_21_32[];//2.当前通道类型设置
extern uint16 MenuHZ13_2_31[] ;//按确认键重启
extern uint16 MenuHZ13_2_32[] ;//按确认键初始化
extern uint16 MenuHZ13_2_33[] ;//按返回键取消
extern uint16 IniSelect[] ;//初始化内容选择：
extern uint16 Menu_IniSelect[][4] ;//初始化选项的内容,包括 1.历史数据 2.电表档案 3.全部数据

extern uint16 FirmRotateParaSet[] ;//按确认键设置
extern uint16 QuitRotateParaSet[] ;//按返回键退出

extern uint16 MenuHZ13_24_31[] ;//1.显示内容设置
extern uint16 MenuHZ13_24_32[] ;//2.显示方式设置
extern uint16 MenuHZ13_24_33[] ;//3.轮显测量点设置

extern uint16 MenuHZ13_27_31[] ;//1.最大载波延时设置

//zengxl20101027电表数量信息查看
//总数量:
//RS485表:
//载波表:
extern uint16 Menu_MeterNum[][7];//电表数量查看

extern uint16 InputTime[] ;//请输入查询时间：

//zengxl20101022测量点抄读数据类型选择，实时数据或者历史数据
extern uint16 RealOrHisType[] ;//查询类型:
//实时电量
//当日冻结电量
extern uint16 Menu_RealOrHis[][6] ;//抄读数据类型选择


//zengxl20101020添加终端版本信息
//版本信息:
//适用省份:
//编译日期:
//其他信息：
extern const uint16 Menu_VerInfo[][5] ;//终端版本信息
extern const uint16 Menu_VerInfo1[][8];//具体信息
extern const uint8 date[];             //shijian 

//zengxl20101012添加通信通道配置类型
//主站ip配置
//APN配置
//心跳周期配置
//以太网IP配置
extern const uint16 Menu_ConfigType[][10];//配置类型，如上所示

//zengxl20100819添加终端编码类型及其值
extern uint16 AddrCodeType[];//编码类型：

//BCD码
//HEX码
extern const uint16 Menu_AddrCodeType[][4];//编码类型，如上所示

//普通485表
//载波表
//简易多功能
//多功能总表
//中继器
extern uint16 Menu_MeterType[][5];//电表类型，如上所示
extern uint16 Menu_ProtocalType[][12];//规约类型，DL/T645-2007和DL/T645-1997两种
extern uint16 Menu_CommType[][5];//通信方式，RS485 和 载波 两种

//1.终端地址
//2.终端时间
//3.以太网参数
//4.上行通信参数
//5.主站参数
//6.终端版本
extern const uint16 Menu_TermParaShow[][8] ; //终端参数查看 菜单 如上所示



//四级菜单

//行政区码:
//终端编号:
//10进制:
//16进制:
extern uint16 Menu_TermAddrShow[][5] ; //终端地址查看 如上所示

//通道类型:
//通信方式:
// APN:
//心跳周期:
extern uint16 Menu_UpCommuParaShow[][5] ; //上行通信参数查看 如上所示

//移动网络
//联通网络
//以太网  
//短信    
extern uint16 Menu_ChannelType[][6];//当前通道类型，如上所示

extern const uint16 Menu_RemoteCommType[][5];//远程通信方式，UDP和TCP两种

extern uint16 MainIP[] ;//主IP:
extern uint16 SecondIP[] ;//备IP:
extern uint16 MainPort[] ;//主IP端口:
extern uint16 SecondPort[] ;//备IP端口:
extern uint16 Minute[] ;//分钟
extern uint16 EthernetIP[] ;//IP地址:(以太网IP)
extern uint16 SubNetMask[] ;//子网掩码:
extern uint16 DefaultGateway[] ;//默认网关:
extern uint16 MACAddr[] ;//MAC地址
extern uint16 AreaCode[] ;//区号
extern uint16 BuildingCode[];//楼号
extern uint16 DeviceCode[];//设备号

//抄表状态信息,包括
////已抄电表数量:
////未抄电表数量:
////最后抄表时间:
////正在抄读电表:
extern const uint16 MeterStateInfo[][9] ; //抄表状态信息，包括已抄电表、未抄电表、最后抄表时间、正在抄读电表
extern uint16 SelectMeterType[] ;//选择电表类型：
extern uint16 Menu_MeterType_DataItem[][3] ;//需要设置采集数据项的电表类型，分为普通表和重点表

extern uint16 Menu_RotateMode[][5];//轮显方式，包括显示一次值和显示二次值
extern uint16 RotatePoint[] ;//请输入轮显测量点：
extern uint16 MaxCarrierDelay_1[] ;//请输入最大载波延时：
extern uint16 MaxCarrierDelay_2[] ;//(15<=MAX<=60)

//轮显
extern uint16 Rotate1[] ;//正向有功电量
extern uint16 Rotate2[] ;//反向有功电量
extern uint16 RotateZXWG[] ;     //正向无功电量  yb add 100519
extern uint16 RotateFXWG[] ;//反向无功电量
extern uint16 TotalEle[] ;//总:
extern uint16 Ele1[] ;//尖:
extern uint16 Ele2[] ;//峰:
extern uint16 Ele3[] ;//平:
extern uint16 Ele4[] ;//谷:
extern uint16 PreMonth[] ;//(上月)
extern uint16 Current[] ;//(当前)

extern uint16 Rotate3[] ;//功率及功率因数
extern uint16 Rotate3_1[] ;//有功功率(Kw):
extern uint16 Rotate3_2[] ;////无功功率(Kvar):
extern uint16 Rotate3_3[] ;//功率因数

extern uint16 Rotate4[] ;//最大需量及发生时间
extern uint16 Rotate4_1[] ;//正向有功(Kw):
extern uint16 Rotate4_2[] ;//反向有功(Kw):
extern uint16 Rotate4_3[] ;//发生时间:
extern uint16 Rotate4_4[] ;//正向无功(Kw):    ybrepair  100519
extern uint16 Rotate4_5[] ;//反向无功(Kw):

extern uint16 Rotate5[];//电压和电流

extern uint16 Rotate5_1[] ;//电压(V)
extern uint16 Ua[] ;//Ua= 
extern uint16 Ub[] ;//Ub= 
extern uint16 Uc[] ;//Uc= 
extern uint16 Rotate5_2[] ;//电流(A)
extern uint16 Ia[] ;//Ia= 
extern uint16 Ib[] ;//Ib= 
extern uint16 Ic[] ;//Ic= 

extern uint8 NoValue[] ;//--------
extern uint16 RotateWGDL[] ;//总无功电量
extern uint16 ZX[] ;//正向:
extern uint16 FX[] ;//反向:
extern uint16 MenuHz_DeviceAddr[] ;//设备地址:
extern uint16 MenuHz_MeterNum[] ;//电表表号:
extern uint16 MenuHz_SysTime[] ;//终端时间:
extern uint16 RotateYFFItem[] ;//预付费数据项

extern uint16 MenuHz_QuadrantWGDL[] ;//四象限无功总电量
extern uint16 Quadrant_1[] ;//第1象限:
extern uint16 Quadrant_2[] ;//第2象限:
extern uint16 Quadrant_3[] ;//第3象限:
extern uint16 Quadrant_4[] ;//第4象限:

//出厂初始值
extern uint8  IniTestAddr[];//初始测试点电表地址 000000000001
extern uint8  IniDate[] ;//08年09月01日
extern uint8  IniTime[] ; //00时00分00秒
extern uint8  IniIp[]  ;//192.168.000.001
extern uint16  Dot[]	;  //. . . 
extern uint8  IniPort[] ;//08001
extern uint8  IniSecIp[] ;
extern uint8  IniSecPort[] ;
extern uint8  IniEthenetIP[] ;//192.168.1.2初始以太网IP地址
extern uint8  IniSubNetMask[] ;//255.255.255.255初始子网掩码
extern uint8  IniGateWay[] ;//192.168.1.1初始网关
extern uint8  IniMACAddr[] ;//00:00:00:00:00:01 初始MAC地址

extern uint8  IniApn[]  ;//cmnet
extern uint8  IniAddr[];//FF FF FF FF
extern uint8  IniInterfacePsw[] ;//000000 此值作为输入密码时,内存的初始值
extern uint8  IniInterfacePsw_1[] ;//111111//初始化后的界面密码值

extern uint8  IniTestPointNum[];//00001测量点号
extern uint8  IniCollectAdd[];//0000测量点采集器地址
extern uint8  IniTestPointNo[];//0001测量点号
extern uint8  IniDeviceNum[];//0001 初始化设备号
extern uint8  IniCommPort[];// 001 初始化通信端口号
//extern uint8  IniCarrierDelay[];//15 初始化最大载波延时时间

extern uint16 IPErr[];//IP设置ERR
extern uint16 PortErr[];//端口设置ERR

extern const uint16 RegistString[7];


//begin:为增加抄表轮显与查询增加。zjjin,20150423.
extern uint16 ZCReBiaoHao[4];    //字串  "热表号:"
extern uint16 ZCFaKongHao[4];
extern uint16 ZCZongReLiang[4];   //字串"累计热量:"
extern uint16 ZCZongLiuLiang[4];   //字串"累计热量:"
extern uint16 ZCLiuSu[3]; //字串"流速:"
extern uint16 ZCkWh[3]; //字串"kWh"
extern uint16 ZCMWh[3];
extern uint16 ZCLiFangMiperH[3];//字串"m3/h"
extern uint16 ZCTongDaoHao[4];
extern uint16 ZCZhuHuXinXi[5];
extern uint16 ZCChaoFaKong[4]; //字串"抄阀控:"
extern uint16 ZCChengGong[2];  //字串"成功"
extern uint16 ZCShiBai[2];	 //字串"失败"
extern uint16 ZCShiWen[2]; //字串"室温"
extern uint16 ZCFaKai[2];  //字串"发开"
extern uint16 ZCTimeErr[7];
extern uint16 ZCBuChaoBiao[6];  //补抄表参数设置。
extern uint16 ZCBiaoBuChaoCiShu[6];  //表补抄次数。
extern uint16 ZCBiaoJianGe[8];  //表时间间隔(ms).
extern uint16 ZCFaBuChaoCiShu[6];  //阀补抄次数。
extern uint16 ZCFaJianGe[8];  //阀时间间隔(ms).

extern uint16 ZCMeterFail[6];
extern uint16 ZCValveFail[6];

extern uint16 ZCJiLiangDian[4];
extern uint16 ZCNoNodeData[8];
extern uint16 ZCNoSetNode[8];
extern uint16 ZCThenRe[5];

extern uint16 ZCValveOpenTime[4];

extern uint16 ZCApnSet[5];  //APN接入点设置
extern uint16 ZCIPAndPortSet[7];
extern uint16 ZCAPN1[7];  //0-cmnet.
extern uint16 ZCAPN2[11];  //1-unicomM2M.
extern uint16 ZCAPN3[12];  //2-whrd.wh.sd.
extern uint16 ZCSelectBySN[7];  //请输入相应序号。
extern uint16 ZCCueRestart[7];





//end:为增加抄表轮显与查询增加。




/*字库汉字如下：
曾西安晨泰科大科技研发有限责任公司注册执行当前功率和电量压流正反向有无总尖峰平谷相因参数据测试点显示设置与查看集中器管理维护信息终端时间行政区码主站及端口号码重起初始化请输入要询的表序地址软件版本编译日期主用备用卡本年月分秒启按确认键返回取消显
*/
extern const uint8 HZTable[][32];




#endif
