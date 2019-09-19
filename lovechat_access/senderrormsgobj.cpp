#include "senderrormsgobj.h"
#include "sim_parser.h"
#include "lovechat_lemocard_proc.h"
#include "senderrormsgobj.h"
#include<json/json.h>

int CSendErrorMsgObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
{
	int ret = CCommObj::init(reqmsg, flow, ccdheader);
	if(ret != 0){ 
		DEBUG_P(LOG_ERROR, "(%s:%d)ret:%d", __FILE__, __LINE__, ret);
		_retcode = ERR_INIT;
		return _retcode;
	}   

//	_reqbody.ParseFromString(_body);
//	DEBUG_PL(LOG_DEBUG, "uin:%u reqbody:%s\n", _usrid.uin(), _reqbody.ShortDebugString().c_str());
	_svrseq = _proc->get_seq();

	return ret;
}

int CSendErrorMsgObj::do_work(Msg &msg, uint32_t &msg_seq)
{	
	int ret = 0;
	switch(_cur_step){
		case STEP_BASE:
			ret = END;
			_cur_step = STEP_END;
			break;
		default:
			ret = END;
			_cur_step = STEP_END;
			break;
	}   

	if(ret != 0){ 
		do_rsp();
		/*
		   if(_cur_step != STEP_END){
		   ret = 0;
		   }
		 */
	}   

	DEBUG_P(LOG_DEBUG, "(%s:%d)ret:%d\n", __FILE__, __LINE__, ret);
	return ret;
}


int CSendErrorMsgObj::do_rsp()
{
	if(has_rsp()){
		return 0;
	}

	unsigned flow = 0;
	unsigned bufsize = 0;
	char *buf = get_rsp(bufsize, flow);
	if(buf != NULL){
		_proc->enqueue_ccd(buf, bufsize, flow);
	}else{
		DEBUG_P(LOG_ERROR, "get_rsp failed\n"); 
	}

	return 0;
}

void CSendErrorMsgObj::generate_rspbody()
{

	//	_rspbodycontent = body;
	//	_rspbody.SerializeToString(&_rspbodycontent);
	//	DEBUG_PL(LOG_DEBUG, "rspbody:%s\n", _rspbody.ShortDebugString().c_str());
}

char* CSendErrorMsgObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	//generate_rspbody();
	// 添加 http包头信息， 通过readsome 函数 读出
	flow = _flow;
	Response *res = new Response(STATUS_BAD_REQUEST);
	string ver = "HTTP/1.1";
	Json::Value item;
	item["rsp"] = "400";
	res->set_body(item);
	res->gen_response(ver, 0);
	int readsize = 0;
	_ccdheader._type = ccd_req_data;
	memcpy(_objbuf, (char*)&_ccdheader, CCD_HEADER_LEN);
	//指针偏移 CCD_HEADER_LEN 长度
	res->readsome(_objbuf+CCD_HEADER_LEN, OBJ_BUF_LEN, readsize);
	bufsize = (unsigned)readsize +  CCD_HEADER_LEN; 
	delete res;
	return _objbuf;
}
