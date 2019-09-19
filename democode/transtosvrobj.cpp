#include "transtosvrobj.h"
#include "lovechat_lemocard_proc.h"
#include "time_stat.h"

namespace lovechat{
int CTransToSvrObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader* ccdheader)
{
	int ret = CCommObj::init(reqmsg, flow, ccdheader);
	if(ret != 0){
		DEBUG_PL(LOG_ERROR, "ret:%d",  ret);
	}
	_svrseq = _proc->get_seq();
	_jsonbody =  reqmsg.body();
	_reqbody.parse_request(reqmsg.body().c_str(), reqmsg.body().size());
/*
	string openid = _reqbody.get_param("openid");
	if(openid==""){
		_is_initfaild = false;
		_root["errcode"] = ERR_PARAM;
		return ERR_PARAM;	
	}

	uint32_t uin = get_uin_by_openid(openid);
	if(uin==0){
		_root["errcode"] = ERR_PARAM;
		_is_initfaild = false;
		_root["errmsg"] = "openid is invalid";
		return ERR_PARAM;
	}
	DEBUG_PL(LOG_DEBUG, "uin:%u\n", uin);
	_usrid.set_uin(uin);
	_head.mutable_usrid()->set_uin(uin);
	if(ret!=0){
		_is_initfaild = false;
		_root["errcode"] = ret;
	}else

		*/
	if(_usrid.uin()==0){
		_root["code"] = -994;
		_is_initfaild = false;
		_root["debugmsg"] = "openid is invalid";
		_root["msg"] = "登录超时，请重新登录";
		return ERR_PARAM;
	}
	_is_initfaild = true;
	return ret;
}

int CTransToSvrObj::do_work(Msg &msg, uint32_t &msg_seq)
{
	int ret = 0;
	stat_rspstep();
	switch(_cur_step){
		case STEP_BASE:
			ret =  req_trans_to_svr(msg, msg_seq);
			if(ret == 0){
				_cur_step = STEP_REQ_TRANS_TO_SVR;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_TRANS_TO_SVR:
			ret = rsp_trans_to_svr(msg);
			ret = END;
			_cur_step = STEP_END;
			break;
		default:
			ret = END;
			_cur_step = STEP_END;
			break;
	}


	stat_step();
	DEBUG_PL(LOG_DEBUG, "ret:%d\n",  ret);
	if(ret != 0){
		do_rsp();
	}

	return ret;
}

int CTransToSvrObj::req_trans_to_svr(Msg &ccdmsg, uint32_t &msg_seq)
{
	short port = 0;
	unsigned  ip = 0;
	//string _svrname = "lovechat_lemoprofile";
	string _svrname  = _head.instance();
	int ret = _proc->get_dcc_svrip(_head.bizid(), _svrname, _usrid.uin(), "", ip, port);
	if(ret != 0){
		DEBUG_PL(LOG_DEBUG, "ret:%d\n",  ret);
		_retcode = ret;
		return ret;
	}

	msg_seq = _svrseq;
	ccdmsg.mutable_head()->set_svrseq(msg_seq);
	ccdmsg.set_body(_jsonbody);
	string data;
	ccdmsg.SerializeToString(&data);
	string senddata = get_send_buf(data);

	ret = _proc->enqueue_dcc(const_cast<char*>(senddata.c_str()), senddata.size(), ip, port);
	DEBUG_PL(LOG_DEBUG, "ret:%d ip:%s port:%d\n", ret, ip2str(ip), port);
	return ret;
}

int CTransToSvrObj::rsp_trans_to_svr(Msg &dccmsg)
{

	_commonrspbody = dccmsg.body();
	trim(_commonrspbody);
	if(dccmsg.head().retcode() != 0){
		DEBUG_PL(LOG_ERROR, "uin:%u sendmsg failed.ret:%d\n", _usrid.uin(), dccmsg.head().retcode());
		_retcode = dccmsg.head().retcode();
		return _retcode;
	}

	DEBUG_PL(LOG_ERROR, "uin:%u sendmsg. ret:%d\n", _usrid.uin(), dccmsg.head().retcode());
	return 0;
}

int CTransToSvrObj::do_rsp()
{
	if(has_rsp()){
		return 0;
	}

	unsigned bufsize = 0;
	unsigned flow = 0;
	char *buf = get_rsp(bufsize, flow);
	if(buf != NULL){
		int ret = _proc->enqueue_ccd(buf, bufsize, flow);
		DEBUG_PL(LOG_DEBUG, "ret:%d\n",  ret);
	}

	return 0;
}

void CTransToSvrObj::generate_rspbody()
{
	if(_is_initfaild){	
		_rspbodycontent  = _commonrspbody;
	}else{
		Json::FastWriter writer;
		_rspbodycontent = writer.write(_root);
	}
	//DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());

}

char* CTransToSvrObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
}
//
