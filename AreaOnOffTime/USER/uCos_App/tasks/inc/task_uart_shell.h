#ifndef __COMMAND_H
#define __COMMAND_H

typedef enum{UartStateInput,UartStateExe}UartCommandState; //命令状态机类型定义
typedef void ( *UartCmdFunction) (u8 argc,void **argv); //函数指针定义
typedef struct{
u8 Used;   //该命令项是否占用，可用于以后的扩展。已经使用为1，未用为0.
u8* UartCmdStr;	   //命令字符串
UartCmdFunction UartCmdFunc;  //对应的函数指针，可以看成一个回调函数
}UartCmdDef;			  //一个命令定义

#define MaxArgv 5
#define MaxChar 20
typedef struct{
u8 Argc;                                         /*指定参数个数*/
u8 Argv[MaxArgv][MaxChar+1];  /*命令字符串和参数字符串数组*/
}WordTable;	
/*begin:yangfei added 2013-01-23*/
extern void Task_UartCmd(void *pdata);
/*end:yangfei added 2013-01-23*/
#endif /* __COMMAND_H */
