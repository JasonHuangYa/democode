#pragma once
#include "comm_inc.h"
#include "commutil.h"
#include "lemocardcommobj.h"

namespace lovechat{

class CTransToSvrObj : public CLemoCardCommObj 
{
	public:
		CTransToSvrObj(void *p):CLemoCardCommObj(p){}
		virtual ~CTransToSvrObj(){}
		virtual int init(const Msg &reqmsg, const uint32_t flow, TCCDHeader* ccdheader);
		virtual int do_work(Msg &msg, uint32_t &msg_seq);
		virtual int do_rsp();
	private:
		int req_trans_to_svr(Msg &ccdmsg, uint32_t &msg_seq);
		int rsp_trans_to_svr(Msg &dccmsg);
	private:	
		virtual void generate_rspbody();
		virtual char* get_rsp(unsigned &bufsize, unsigned &flow);
	private:
		Json::Value _root;
		string _commonrspbody;
		string _jsonbody;
		bool _is_initfaild;
	public:
		void trim(string &s);
};
		inline void CTransToSvrObj:: trim(string &s) 
		{
			if( !s.empty() )
			{   
				s.erase(0,s.find_first_not_of(" "));
				s.erase(s.find_last_not_of(" ") + 1); 
			}   

		}


}


