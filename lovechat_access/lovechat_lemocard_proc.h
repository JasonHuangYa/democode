#pragma once
#include <stdio.h>
#include <string.h>
#include "lovechat.pb.h"
#include "comm_inc.h"
#include "publicproc.h"
#include "sim_parser.h"

using namespace tfc::cache;
using namespace tfc::net;

namespace lovechat{
class CLoveChatLemoCardProc : public PublicProc
{
public:
	CLoveChatLemoCardProc();
	virtual ~CLoveChatLemoCardProc();
	virtual tfc::cache::CacheAccess* get_cache(){string cachename="flow_cache"; return getcache(cachename);}
	virtual tfc::cache::CacheAccessUin* get_uincache(){return getuincache("uin_cache");}
	
//	virtual CCommObj* create_obj_factory(Msg &msg);
	virtual void init_child(CFileConfig &page);
	virtual void runafter_init();
	virtual void dispatch_child();
	virtual void dispatch_ccd();

	virtual int handle_ccd(char *msgbuf, const size_t bufsize, uint32_t flow);
	void send_err_msg(int code, unsigned flow);

	CCommObj* create_obj_factoryV2(Request &req, Msg &msg);



public:
	string getwording(const unsigned bid, const string id) { return get_fieldconf(bid, id); }
	unsigned get_bid(){return _bid;}
	uint32_t get_sessionkeyuin(string &key);

private:
	unsigned _bid;
	int _cache_dump_interval; 
	int _corepoint;
	int _maxbinlogsize;
	int _maxbinlognum;
	string _corefile;
	string _binlogfile;

};

#define ProcPoint ((CLoveChatLemoCardProc*)_proc)
}
