#ifndef				_TASKS_H
#define				_TASKS_H				

//====================

extern    	void CreateTasks_Tasks(void);//创建所有NCQ写的任务
extern    	void CreateMboxs_Tasks(void);//创建所有NCQ写的任务之间使用的消息邮箱和消息队列
extern		void CreateMutexs_Tasks(void);//创建NCQ内部使用的Mutex

#endif
