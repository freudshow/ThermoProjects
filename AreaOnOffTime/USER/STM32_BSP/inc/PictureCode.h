#ifndef        _PICTURE_CODE_H
#define		   _PICTURE_CODE_H

#ifndef IN_PIC_CODE

//计量表项中，编程按钮按下后的图标
extern uint8 ProgramePic[] ;
//计量表项中，交流数据项标志
extern uint8 ACPic[];

//GPRS信号强度图标，数字为强度大小
extern uint8 GPRSSignalPic[][32];//自己定义的信号强度显示
extern uint8 StateSignalPic[][32];//国网要求的信号强度显示

//GPRS或者CDMA信号图标，0（G）为GPRS，1（C）为CDMA
extern uint8 GCPic[][32];
//异常情况图标，0为正常，1为异常情况
extern uint8 AbnormalPic[][32];

//菜单底部提示栏提示图片，为了统一数组，均扩为16*160大小
extern uint8 MenuBottomPic[][320];

//开机界面图片，RISESUN标志,包括中文文字104*104
extern uint8 StartPic[];
//开机界面图片2，只有RISESUN标志,没有任何文字 112*85
extern uint8 RisesunLogo_Only[];
//开机文字 系统启动中...  16*96
extern uint8 SystemStart[];

//注册界面图片 16*160
extern uint8 RegisterPic[];

//ICO标志图片（上下左右箭头图标） 16*16
extern uint8  IcoPic[][32];

//温州晨泰集团
extern uint8 Risesun[];

//与主站连接上的提示：connect
extern uint8 CommuniReg[][70];

//终端地址提示：
extern uint8 AddrIdex[];

//菜单图片
extern uint8 MenuPic_OK[];
extern uint8 MenuPic1[];
extern uint8 MenuPic1_1[];
extern uint8 MenuPic1_2[];
extern uint8 MenuPic1_3[];
extern uint8 MenuPic1_2_11[];
extern uint8 MenuPic1_2_12[];
extern uint8 MenuPic1_2_2[];
extern uint8 MenuPic1_2_3[];
extern uint8 MenuPic1_2_4[];
extern uint8 MenuPic1_2_5[];
extern uint8 MenuPic1_2_6[];
extern uint8 MenuPic_TestPointSet[];//MenuPic1_2_7[];

extern uint8 MenuPic1_3_1[];
extern uint8 MenuPic1_3_2[];
extern uint8 MenuPic1_3_3[];

//轮显图片
extern uint8 RotatePic1[];
extern uint8 RotatePic2[];
#endif

#endif
