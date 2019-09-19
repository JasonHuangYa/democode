#pragma once
#include "itask.h"

namespace lovechat{

class CLemoDbDBTask : public ITask
{
	public:
		CLemoDbDBTask(PublicProc* proc, CFifoSyncMQ *mq_notify, CMutex* mutex, CppMySQL3DB* db, Msg &msg ):ITask(proc, mq_notify, mutex, msg){_db = db;}
		virtual ~CLemoDbDBTask(){}
		virtual void doit();
		int check_param();
	public:
		int do_set_mysql();
		int do_get_mysql();

	private:
		 CppMySQL3DB* _db;
};
}
//
//
//
