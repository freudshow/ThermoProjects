#ifndef        _METER_TABLE_H
#define		   _METER_TABLE_H

#define     COMSET_1		0
#define		COMSET_2		1
#define		COMSET_3		2    /*yangfei added for support leye 485 meter*/
#define     COMSET_4        3    /*yangfei added for support 万华大表*/
#define     COMSET_5        4
#define     COMSET_6        5			//林晓彬添加PLC
#define     COMSET_7        6


#define DE_LU			COMSET_1
#define TIAN_GANG		COMSET_1
#define	LI_CHUANG		COMSET_1
#define DAN_FUSEN		COMSET_1	
#define WAN_HUA			COMSET_2

#define WANHUA_VER			 1		//天津万华。
#define ENGELMANN_VER	     6
#define ZENNER_VER           7    /*真兰热表*/
#define LANDISGYR_VER        8   /*兰吉尔*/
#define HYDROMETER775_VER    9   /*恩乐曼*/


//林晓彬添加
#define NUANLIU_VER		    17/*暖流*/
#define LIANQIANG_VER485              19   /*联强485 嘉洁能485*/

#define HAIWEICI_VER              20   /*海威茨*/
#define JINGWEI_VER              22   /*经纬*/
#define LIANQIANG_VERMBUS   23	   /*联强mbus*/
#define	LI_CHUANG_2		24 //力创第二种表
#define	JICHENG_VER		25 //积成热量表
#define	ZHENYU_VER_2		26 //ZHENYU2热量表
#define JINGWEIFE_VER  27 //经纬带标识FE版本协议

//begin: added by zjjin.
#define YITONGDA_VER			28				//天津易通达

#define WANHUA_TO_DELU_VER      30		//天津万华转成德鲁协议上传
//end: added by zjjin.


//#define ENGELMANN_VER              9   /*恩乐曼*/



/*begin:201303141153 yangfei modified 2013-03-14 for added lichuang PROTOCOL*/
 /*yangfei added 2013-03-25 for support leye 485 meter*/
#define PROTOCOL_MAX	80  //原为8 林晓彬改为80
/*end:yangfei modified 2013-03-14 for added lichuang PROTOCOL*/
extern void (*METER_ComParaSetArray[])(void);
extern uint16 gMETER_Table[][4];

#endif

