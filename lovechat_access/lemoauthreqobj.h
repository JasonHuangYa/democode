#pragma once
#include "comm_inc.h"
#include "commutil.h"
#include "lemocardcommobj.h"


namespace lovechat{
	class CLemoAuthReqObj: public CLemoCardCommObj 
	{
		public:
			CLemoAuthReqObj(void *p):CLemoCardCommObj(p){}	
			virtual ~CLemoAuthReqObj(){}

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
			int req_get_db(Msg &msg, unsigned &msg_seq);
			int rsp_get_db(Msg &msg);
			int req_set_to_db(Msg &msg, unsigned &msg_seq);
			int rsp_set_to_db(Msg &msg);	

			int req_update_db(Msg &msg, unsigned &msg_seq);
			//int rsp_update_db(Msg &msg);
			int parase_json();
			int req_get_yunxintoken(Msg &msg, uint32_t &msg_seq);
			int rsp_get_yunxintoken(Msg &msg);
		public:
			string _code_to_sessionkey_url;
			string _postdata;
			string _http_rspbuf;
			string _data;
			uint32_t _uin_alloc;
			string _openid;
			string _cliopenid;
			string _newsessionkey;
			string _unionid;
			uint32_t _uin;
			Json::Value _subroot;
			LemoCardSessionItem _sessionkeyitem;




	};
}
