#pragma once
#include "comm_inc.h"
#include "commutil.h"
#include "lemocardcommobj.h"
#include "aes.h"
#include "base64.h"
#include "checkupdate.h"


namespace lovechat{
	class CCheckupdateObj: public CLemoCardCommObj 
	{
		public:
			CCheckupdateObj(void *p):CLemoCardCommObj(p){}	
			virtual ~CCheckupdateObj(){}

			virtual int init(const Msg &reqmsg, const uint32_t flow, TCCDHeader* ccdheader);
			virtual int do_work(Msg &msg, uint32_t &msg_seq);

			virtual int do_rsp();
			virtual void generate_rspbody();
			virtual char* get_rsp(unsigned &bufsize, unsigned &flow);

			virtual ENQueueType_t queuetype(){
				/*
				typedef enum QueueType{
					EN_QUEUE_NONE = 0, //不排队
					EN_QUEUE_UIN = 1, //按uin排队
					EN_QUEUE_STR = 2, //生成strkey排队
				}ENQueueType_t;
				*/
				return EN_QUEUE_NONE;
			}

			/*
			virtual string getstrkey(){
				//return _key;
			}
			*/
        public:
			int create_rsp();

		public:
			string _product;
			string _buildver;
			
	};
}
