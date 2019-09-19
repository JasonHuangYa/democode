#pragma once
#include "comm_inc.h"
#include"lovechat_lemocard_proc.h"
#include <string>
#include <iostream>
#include "curl/curl.h"
using namespace std;
namespace lovechat{
	class ITask;
	class HttpTask : public ITask
	{   
		public:
			HttpTask(PublicProc* proc, CFifoSyncMQ* mq_notify, CMutex *mutex, Msg &msg):ITask(proc, mq_notify, mutex, msg)
			{
				_proc = proc;
			}
			virtual ~HttpTask()
			{

			}
			virtual void doit();

		public:

			PublicProc* _proc;
		public:
			 char error_buffer[CURL_ERROR_SIZE];
			 static int writer(char*, size_t, size_t, std::string*);
			 bool init(CURL*&, const char*, std::string*);
			 int libcurl_get(const char* url, std::string& buffer, std::string& errinfo);
			 int libcurl_post(const char* url, const char* data, std::string& buffer, std::string& errinfo);

	};  
}

