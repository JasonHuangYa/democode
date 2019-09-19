#pragma once
#include "comm_inc.h"
#include "lovechat_lemocard_proc.h"
#include "json/json.h"
#include "sim_parser.h"
namespace lovechat{


class CLemoCardCommObj : public CCommObj
{
public:
	CLemoCardCommObj(void* p):CCommObj(p){
	}
	virtual ~CLemoCardCommObj(){}
	virtual char* generate_rsp_buf(unsigned &bufsize);
	int req_get_httpapi(Msg &msg, uint32_t &msg_seq, string &url, string &postdata);
	int rsp_get_httpapi(Msg &msg, string &rspbuf);
public:
	int get_incache(string &key, LemoCardSessionItem &item);
	int set_incache(string &key, LemoCardSessionItem &item);
	string aesEncode(const string tdata);
	string aesDecode(const string &openid);
	virtual string get_statname();
	virtual void stat_step(); 
	virtual void stat_rspstep(); 
	virtual void on_expire();

public:
	Json::Value _root;
	Request _reqbody;
	Response _rspbody;

};
}
//
//
