/*
该键盘扫描程序的作用：实现了数字、字符键的单击和连击，而系统键PB2和PB3则只在按下
时发送按下信息。
*/
#include <includes.h>

#include "task_uart_shell.h"


/*用户的常数定义放在这里*/
#define UartCharMaxCount 30		//命令字符串字符数限制值
#define UartCmdMaxCount 17		//现在支持10个内部命令
#define UartMaxPara 10
													
/* 为了方便定义，这里可用typedef定义一些特殊指针、结构体、联合体或枚举变量*/

/* 用户声明的外部变量放在这里，按意义进行组合*/

/* 用户自己的全局变量在这里定义，按意义进行组合*/
UartCommandState UartCmdState=UartStateInput; /*状态机变量定义*/
u8 UartCmdBuf[UartCharMaxCount+1]; /*命令缓冲数组，字符数有限定*/

UartCmdDef UartCmdTable[UartCmdMaxCount];  //串口命令结构数组
u8* UartCmdStrTable[UartCmdMaxCount]={	   //串口命令字符串
"help","cls","ls","rm","mv","pwd","cd","cat","mkdir","sdwr","new","time","set","reset","cp","test","channel"};

WordTable CmdArg;
void *UartMsg_Tbl[30];          /*按键消息指针数组*/
OS_EVENT *UartMsgOSQ;	/*按键消息队列*/

/* 用户声明的外部函数放在这里*/

/* 用户实现的函数在这里声明*/

void UartCmdTable_Init(void)  /*命令表的初始化。*/
{
  u8 i;
  for (i=0; i<UartCmdMaxCount; i++ )
  { 
     UartCmdTable[i].Used=1; 
     UartCmdTable[i].UartCmdStr=UartCmdStrTable[i];
  }
  UartCmdTable[0].UartCmdFunc=UartCmdHelp;
  UartCmdTable[1].UartCmdFunc=UartCmdCls;
  UartCmdTable[2].UartCmdFunc=UartCmdls;
  UartCmdTable[3].UartCmdFunc=UartCmdrm;
  UartCmdTable[4].UartCmdFunc=UartCmdmv;
  UartCmdTable[5].UartCmdFunc=UartCmdpwd;
  UartCmdTable[6].UartCmdFunc=UartCmdcd;
  UartCmdTable[7].UartCmdFunc=UartCmdcat;
  UartCmdTable[8].UartCmdFunc=UartCmdmkdir;
  UartCmdTable[9].UartCmdFunc=UartCmdSDWrite;
  UartCmdTable[10].UartCmdFunc=UartCmdnew;
  UartCmdTable[11].UartCmdFunc=UartCmdtime;
  UartCmdTable[12].UartCmdFunc=UartCmdSetDebugLevel;
  UartCmdTable[13].UartCmdFunc=UartCmdReset;
  UartCmdTable[14].UartCmdFunc=UartCmdCp;
  UartCmdTable[15].UartCmdFunc=UartCmdTest;
  UartCmdTable[16].UartCmdFunc=UartSetChannel;
}

u8 UartParseCmdBuf(u8* UartCmdBuf,WordTable* CmdArg)
{ 
  u16 CmdBufIndex=0;
  u8 CharIndex=0;
  u8 WordIndex=0;
  u8 CmdEndFlag=0;
  u8 ch;

  ch=UartCmdBuf[0];
  for ( ; !CmdEndFlag && CmdBufIndex<UartCharMaxCount; CmdBufIndex++,ch=UartCmdBuf[CmdBufIndex] )
      {
	        if ( ch==0 ) 
		     { CmdEndFlag=1;
			   CmdArg->Argv[WordIndex][CharIndex]=0;
		     }
		else if ( ch==' ' ) 
		     { 
		                   CmdArg->Argv[WordIndex][CharIndex]=0;
				   WordIndex++; 
				   if ( WordIndex>=MaxArgv )return 1;
				   CharIndex=0;
		     }
		else
		     { 
			   CmdArg->Argv[WordIndex][CharIndex]=ch;
			   CharIndex++; 
			   if ( CharIndex>MaxChar )return 1;
		     }
      }
  if ( 	CmdBufIndex>=UartCharMaxCount )return 1;
  CmdArg->Argc=WordIndex+1;
  return 0;
}

void Task_UartCmd(void *pdata)
{
  u8 UartCharIn;	/*从串口获得的字符*/
  u8 UartCharCount; /*命令缓冲的当前索引*/
  u8* UartCmdStr;    /*命令字符串指针*/
  char UartCompResult=1;/*命令字符串比较的结果，如果在字符串表里找到。返回0.*/
  u8 UartCmdIndex;          /*命令字符串在表里的索引。*/
  void* argv[UartMaxPara];
  u8 j,argc;

  pdata=pdata;

  UartCharCount=0;
  UartCmdState=UartStateInput;
  //创建消息队列，用于串口命令。 
   UartMsgOSQ=OSQCreate(&UartMsg_Tbl[0],30);
  UartCmdTable_Init();   /*命令表的初始化。*/
  //mountSd();                    /* 挂载SD  卡*/
  Uart_Printf("\r\n********** 简单命令接口!*********\r\n");
  Uart_Printf("sh>");      /*输出命令提示符*/
  for(;;)
  {
  
  switch(UartCmdState)			   /*根据状态机进行散转*/
  {
    
    case UartStateInput:			//如果出于输入状态
	  { 
	  	//UartCharIn=Uart_GetChar();		//读取输入按键
	  	USART1Getch(&UartCharIn, 0);
		UartCharIn &= 0x7F;

	    if(UartCharIn=='\r')	    // 如果按下了Enter键,它是触发软件状态机从输入态
	      { 					// 进入命令解释执行态的关键。
		    UartCmdBuf[UartCharCount]='\0'; /*CharCount=输入字符的数字，命令缓冲区以0结束*/
			debug( "\r\n" );      //显示器上回显换行
			if ( UartCharCount==0 )	 	  /*之前没有输入字符，就是平白按了一下Enter*/
			   { Uart_Printf("sh>");   /*重新显示命令提示符，仍在输入态下循环*/
			   }
			else
			   { UartCmdState=UartStateExe;	  //如果输入过字符，进入命令解释态
			     UartCharCount=0;
			   }
	      }
		else if (UartCharIn=='\n')	   //按下回车时串口终端送过来为\r\n.
		  {
		  	//debug("'\n'");
		  }
		else if ( UartCharIn=='\b') 		  //如果输入退格键
		  {
			if ( UartCharCount==0 )		/*之前没有输入字符，按退格不能理他*/
			   { 
			   	//Speaker_Beep();  //警告，已经退回提示符了，再退就:>了。
			   }
			else
			   {
				 
				 UartCharCount -=1;		      /*有效字符数减去1.*/
				 debug( "%c",UartCharIn ); /*让字符界面自己处理退格键*/
				 debug( "%c",' '); 
				 debug( "%c",UartCharIn );
			   }
		  }
		else  		  //如果是其他键
		  {
		    UartCmdBuf[UartCharCount]=UartCharIn;  //将字符存入缓冲区当前位置
			UartCharCount +=1;			   //缓冲区位置向前移动
			if (UartCharCount>=UartCharMaxCount+1)		//如果超过了，这个字符现在占据最后一个位置
			   { 
			   	UartCharCount=UartCharMaxCount;  //使当前位置保持为最后一个，等待Enter。
			       //Speaker_Beep();			  //这个字符也不向字符界面输出,而是产生警告
			   }						  
			else
			   {
				 debug( "%c",UartCharIn );//将有效字符输出。
			   }
		  }

	   }
	   break;

    case UartStateExe:			//如果处于命令执行态
	  { 
	  	UartParseCmdBuf(UartCmdBuf,&CmdArg);
	        UartCmdStr=CmdArg.Argv[0];  
		//解析命令缓冲区，得到命令结构信息，我这里还只支持简单命令
	  	//所以处理很简单，直接获得命令字符串，以后可以扩展。					 		 
	    for(UartCmdIndex=0; UartCmdIndex<UartCmdMaxCount; UartCmdIndex++ )
	      {  // 在命令表里搜索对应命令字符串
		    UartCompResult=strcmp( (char*)UartCmdStr, (char*)(UartCmdStrTable[UartCmdIndex]) );
			//命令字符串比较的结果
			if (UartCompResult==0) //如果在字符串表里找到。返回0.
			    break; //此时CmdIndex对应的值为命令在命令表里的索引。
	      }
		if ( UartCmdIndex<UartCmdMaxCount)
		   {  /*找到对应命令，调用相应函数处理*/
		      argc=CmdArg.Argc;
			  for ( j=0; j<argc; j++ )
			  {
			  	argv[j]=(void*)CmdArg.Argv[j];
			  }
			  UartCmdTable[UartCmdIndex].UartCmdFunc(argc,argv);
		   } 
		else   /*没有找到对应命令，输出信息。*/
		   {  
		   	debug("Bad Command!\r\n"); 
		   }  
		UartCmdState=UartStateInput;
		Uart_Printf("Sh>");
		UartCharCount=0;
	  }
      break;

	default:
	  break;
  }
 }    
}

