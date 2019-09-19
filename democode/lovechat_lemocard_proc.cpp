#include <stdio.h>
#include <string.h>
#include "heartbeat.h"
#include "commobj.h"
#include "lovechat_lemocard_proc.h"
#include "loginobj.h"
#include "verifycodeobj.h"
#include "decryptobj.h"
#include "lemoauthreqobj.h"
#include "lemocardmanage.h"
#include "senderrormsgobj.h"
#include "curl/curl.h"
#include "transtosvrobj.h"
#include "getleftsocre.h"
#include "miniapplogin.h"
#include "checkupdate.h"
#include "wxlogin.h"


using namespace tfc::cache;
using namespace tfc::diskcache;
using namespace tfc::net;
using namespace tfc::base;
using namespace lovechat;

CLoveChatLemoCardProc::CLoveChatLemoCardProc()
{
	_bid = 10000;
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

CLoveChatLemoCardProc::~CLoveChatLemoCardProc()
{
	curl_global_cleanup();
}

uint32_t get_uin_by_openid(const string &openid)
{
	string ipasswd = "account;;;xxxxxx";
	IAES iaes;
	string uin = CStr::str_2_bin(openid);
	string taceresult = iaes.DecodeAES_CBC(ipasswd,uin);
	return from_str<unsigned>(taceresult);

}


CCommObj* CLoveChatLemoCardProc::create_obj_factoryV2(Request &req, Msg &msg)
{
	CCommObj *obj = NULL;
	string path = req.get_request_uri();
	DEBUG_PL(LOG_DEBUG, "path:%s\n", path.c_str());

	string openid = req.get_param("openid");
	if(openid=="") openid = req.get_param("sessionkey");
	uint32_t uin = get_sessionkeyuin(openid);
	if(uin==0){
		openid = req.get_param("openid");
		if(openid!=""){
			uin = get_uin_by_openid(openid);
		}
	}
	msg.mutable_head()->mutable_usrid()->set_uin(uin);
	DEBUG_PL(LOG_ERROR, "uin:%u,openid:%s, path:%s\n", uin, openid.c_str(), path.c_str());

	if(path =="/heartbeat"){
		obj = new CHeartBeatObj((void*)this);
	}else if(path=="/account/auth"){
		obj = new CLemoAuthReqObj((void*)this);
	}else if(path=="/account/sendcode"){
		obj = new CVerifyCodeObj((void*)this);
	}else if (path=="/account/checkcode"){
		obj = new CVerifyCodeObj((void*)this);
	}else if (path=="/account/checkphone"){
		obj = new CMiniAppLoginObj((void*)this);
	}else if(path=="/account/wxlogin"){
		obj = new CWxLoginObj((void*)this);
	}else if(path=="/decrypt"){
		obj = new CDecryptObj((void*)this);
	}else if(path=="/account/checkupdate"){
		obj = new CCheckupdateObj((void*)this);
	}else if(path=="/profile/setprofile"){
		msg.mutable_head()->set_cmd(CMD_SETPROFILE_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoprofile");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/profile/getprofile"){
		msg.mutable_head()->set_cmd(CMD_GETPROFILE_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoprofile");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/profile/getprofilebyuin"){
		msg.mutable_head()->set_cmd(CMD_GETPROFILE_BYUIN_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoprofile");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/profile/updateprofile"){
		msg.mutable_head()->set_cmd(CMD_UPDATEPROFILE_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoprofile");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/account/login"){
		obj = new CLoginObj((void*)this);
	}else if(path =="/profile/getleftsocre"){
		obj = new CGetLeftSocre((void*)this);
	}else if(path=="/frilist/setfri"){
		msg.mutable_head()->set_cmd(CMD_SET_LEMO_FRI_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/frilist/getfri"){
		msg.mutable_head()->set_cmd(CMD_GET_LEMOFRILIST_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/frilist/setgroup"){
		msg.mutable_head()->set_cmd(CMD_SET_LEMO_GROUP_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/frilist/getgroup"){
		msg.mutable_head()->set_cmd(CMD_GET_LEMO_GROUP_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/frilist/setgroupmember"){
		msg.mutable_head()->set_cmd(CMD_SET_LEMO_GROUP_MEMBER_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/frilist/getgroupmember"){
		msg.mutable_head()->set_cmd(CMD_GET_LEMO_GROUP_MEMBER_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/frilist/search"){
		msg.mutable_head()->set_cmd(CMD_SEARCH_FOR_LEMOFRILIST_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/frilist/getrecommend"){
		msg.mutable_head()->set_cmd(CMD_GET_RECOMMEND_BYTYPE_REQ);
		msg.mutable_head()->set_instance("lovechat_lemofrilist");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/score/getleftscore"){
		msg.mutable_head()->set_cmd(CMD_GET_LEMO_SCORE_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoscore");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/score/history"){
		msg.mutable_head()->set_cmd(CMD_GET_LEMO_SCROEHISTORY_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoscore");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/score/detail"){
		msg.mutable_head()->set_cmd(CMD_GET_LEMO_INVITEDETAIL_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoscore");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/score/tasklist"){
		msg.mutable_head()->set_cmd(CMD_GET_LEMO_SCROE_TASK_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoscore");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/score/dig"){
		msg.mutable_head()->set_cmd(CMD_COINTASK_COMPLETE_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoscore");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/score/complete"){
		msg.mutable_head()->set_cmd(CMD_SET_TRADING_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoscore");
		obj = new CTransToSvrObj((void*)this);
	}else if(path=="/score/getcompletest"){
		msg.mutable_head()->set_cmd(CMD_GET_COMPLETE_ST_REQ);
		msg.mutable_head()->set_instance("lovechat_lemoscore");
		obj = new CTransToSvrObj((void*)this);
	}else{
		DEBUG_PL(LOG_ERROR, "error path%s\n", path.c_str());
	}
	return obj;
}

uint32_t  CLoveChatLemoCardProc::get_sessionkeyuin(string &key)
{
	CacheItem cacheitem;
	LemoCardSessionItem item;
	int ret = CGetCache::getcache(get_cache(), key, cacheitem);             
	if(ret == 0){ 
		if(!item.ParseFromString(string(cacheitem._buf, cacheitem._data_len))){          
			return 0;                                                          
		}                                                                                 
	}else{
		DEBUG_PL(LOG_DEBUG, "key:%s not in cache! ret:%d\n", key.c_str(), ret);                               
		return 0;
	}   
	return item.uin();
}


void CLoveChatLemoCardProc::send_err_msg(int code, unsigned flow)
{
	CCommObj *obj = NULL;
	obj = new CSendErrorMsgObj((void*)this);
	Msg msg;
	trigger_ccd_msg(obj, msg, flow);
	return;
}


int CLoveChatLemoCardProc::handle_ccd(char *msgbuf, const size_t bufsize, uint32_t flow)
{
    Request req; 
    int ret = req.parse_request(msgbuf, bufsize);
    if (ret != 0) {
        DEBUG_PL(LOG_ERROR, "PARSE request error which ret:%d\n", ret);
        send_err_msg(400, flow);
        return ret;
	}   

	Msg msg;
	CCommObj *obj = create_obj_factoryV2(req , msg);
	msg.set_body(msgbuf, bufsize);
	if(obj!= NULL){
		int ret = trigger_ccd_msg(obj, msg, flow);
		return ret; 
	}else{
		send_err_msg(400, flow);
		return -1;
	}    

	return 0;

}

void CLoveChatLemoCardProc::dispatch_ccd()
{
	unsigned		data_len;
	int				ret;
	char			*http_data = NULL;
	unsigned flow = 0;
	do {
		data_len = 0;
		ret = _mq_ccd_2_mcd->try_dequeue(_recv_buf, 1<<24, data_len, flow);
		if( !ret && data_len > 0 ) {
			TCCDHeader *ccd_header = (TCCDHeader *)_recv_buf;
			_ccd_header = ccd_header;
			if ( ccd_header->_type == ccd_rsp_data_shm ) {
				memhead *tmp_memhead = (memhead *)(_recv_buf + sizeof(TCCDHeader));
				http_data = (char*)myalloc_addr(tmp_memhead->mem);
				if ( http_data ) {
					if(handle_ccd(http_data, tmp_memhead->len, flow)){
						DEBUG_PL(LOG_ERROR, "Handle HTTP request fail!\n");
					}
				} else {
					DEBUG_PL(LOG_ERROR, "Get shm address fail!\n");
				}
				myalloc_free(tmp_memhead->mem);
			} else if ( ccd_header->_type == ccd_rsp_data ) {
				http_data = (char *)(_recv_buf + sizeof(TCCDHeader));
				int iret = handle_ccd(http_data, data_len - sizeof(TCCDHeader) , flow);
				if(iret == 0){ 
					DEBUG_PL(LOG_DEBUG, "msg request continue!\n");
				}else if(iret == 100){
					DEBUG_PL(LOG_DEBUG, "msg request end!\n");
				}else if(iret != 100 || iret != 0){ 
					DEBUG_PL(LOG_ERROR, "msg request fail! iret:%d\n", iret);
				}  
			} else if ( ccd_header->_type == ccd_rsp_connect ) {
				// cout<<"CCD request data type ccd_rsp_connect!"<<endl;
				continue;
			} else if ( ccd_header->_type == ccd_rsp_disconnect ) {
				// cout<<"CCD request data type ccd_rsp_disconnect!"<<endl;
				continue;
			} else if ( ccd_header->_type == ccd_rsp_overload ) {
				cout<<"CCD request data type ccd_rsp_overload!"<<endl;
				continue;
			} else if ( ccd_header->_type == ccd_rsp_overload_conn ) {
				cout<<"CCD request data type ccd_rsp_overload_conn!"<<endl;
				continue;
			} else if ( ccd_header->_type == ccd_rsp_overload_mem ) {
				cout<<"CCD request data type ccd_rsp_overload_mem!"<<endl;
				continue;
			} else {
				// Never occur.
				cout<<"Invalid CCD request data type!"<<endl;
				continue;
			}			
		}
	}while( !ret && data_len > 0 );
}



void CLoveChatLemoCardProc::init_child(CFileConfig &page)
{
	//TRY_GET_STRPARAM(page, _need_init_recommend_rtree, "root\\need_init_recommend_rtree", "false");
	//TRY_GET_INTPARAM(page, _size, "root\\mapfile_size", 1<<22);
	init_thread_dbv2(page);
	init_thread_redis_db(page);
	TRY_GET_INTPARAM(page, _bid, "root\\bid", 10000);
	TRY_GET_INTPARAM(page, _cache_dump_interval, "root\\core\\cachedumpinterval", 60);
	TRY_GET_INTPARAM(page, _corepoint, "root\\core\\corepoint", 60);
	TRY_GET_INTPARAM(page, _maxbinlogsize, "root\\core\\maxbinlogsize", 100000000); //每天凌晨一点dump一下
	TRY_GET_INTPARAM(page, _maxbinlognum, "root\\fricore\\maxbinlognum", 2); 
	TRY_GET_STRPARAM(page, _corefile, "root\\core\\corefile", "../data/cachecore_dump_file.data");
	TRY_GET_STRPARAM(page, _binlogfile, "root\\core\\binlogfile", "../data/binlog_file");
	if(0 == get_cache()->CoreInit(_cache_dump_interval, _corepoint, (char*)(_corefile.c_str()), (char*)(_binlogfile.c_str()), _maxbinlogsize, _maxbinlognum)){
		if(get_cache()->StartUp_mirror()){
			DEBUG_PL(LOG_ERROR, "cache not new. not startup mirror.\n");
		}else{
			DEBUG_PL(LOG_DEBUG, "cache is new. startup mirror.\n");
		}   
	}  
}

void CLoveChatLemoCardProc::runafter_init()
{

}

void CLoveChatLemoCardProc::dispatch_child()
{

}

extern "C"
{
	CacheProc* create_app()
	{
		return new CLoveChatLemoCardProc();
	}
}
