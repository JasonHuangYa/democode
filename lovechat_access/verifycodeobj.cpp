#include "verifycodeobj.h"
#include "httptask.h"

int CVerifyCodeObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
{
    int ret = CCommObj::init(reqmsg, flow, ccdheader);
    if(ret != 0){ 
        DEBUG_P(LOG_ERROR, "(%s:%d)ret:%d", __FILE__, __LINE__, ret);
		_retcode = ERR_INIT;
		return _retcode;
    }   

    _svrseq = _proc->get_seq();
	_reqbody.parse_request(reqmsg.body().c_str(), reqmsg.body().size());
	string phone = _reqbody.get_param("phone");
	string codebit = _reqbody.get_param("codebit");
	if(phone==""){
		 _root["errcode"] = -1043;
		 _root["errmsg"] = "phone is invalid";
		 _root["code"] = -1043;
		 _root["debugmsg"] = "phone is invalid";
		 _root["msg"] = "手机号无效";
		 return -1043;
	}
	string path = _reqbody.get_request_uri();
	if(path=="/account/sendcode"){
		_posturl = "https://www.tuomin.com/lemocardsms/sendcode";
		_postdata = "secret=luxyisverygood&phone="+phone;
	//	if(codebit=="4"){
			_postdata+="&codebit=4";
	//	}
	}else{
		_posturl = "https://www.tuomin.com/lemocardsms/checkcode";
		_postdata = "secret=luxyisverygood&phone="+phone+"&code="+_reqbody.get_param("code");
	}
	string usrip = _reqbody.get_header("X-Real-IP");
	string ticket = _reqbody.get_param("ticket");
	string randstr = _reqbody.get_param("randstr");
	/*if(ticket=="" or randstr==""){
		_root["code"] = -1043; 
		DEBUG_PL(LOG_ERROR, "phone:%s has not ticket\n", phone.c_str());
		return -1043;
	}
	*/
	_tencnetcodebuf = "https://ssl.captcha.qq.com/ticket/verify?aid=2054237779&AppSecretKey=01vTsj4cvGEMgi4oBslN_uA**&Ticket="+ticket+"&Randstr="+randstr+"&UserIP="+usrip;
	DEBUG_PL(LOG_ERROR, "phone:%s sned ticket tencent check! %s\n", phone.c_str(), _tencnetcodebuf.c_str());
    return ret;
}

int CVerifyCodeObj::do_work(Msg &msg, uint32_t &msg_seq)
{	
    int ret = 0;
    stat_rspstep();
    switch(_cur_step){
        case STEP_BASE:
		goto NOCHECK;
			ret = req_send_tencent(msg, msg_seq);
			if(ret==0){
				_cur_step = STEP_REQ_GET_DATA;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_GET_DATA:
			ret = rsp_send_tencent(msg);
			if(ret==0){
NOCHECK:		ret = req_get_httpapi(msg, msg_seq, _posturl, _postdata);
				if(ret==0){
					_cur_step = STEP_REQ_SEND_HTTP;
					break;
				}   
			}
            ret = END;
            _cur_step = STEP_END;
            break;
		case STEP_REQ_SEND_HTTP:
			ret = rsp_get_httpapi(msg, _http_rspbuf);
			if(_http_rspbuf=="0"){
				_root["errcode"]=0;
				_root["code"] = 0;
			}else{
				_root["errcode"]=-1;
				_root["code"] = -1047;
				_root["debugmsg"] = "invalid phone";
				if(_posturl =="https://www.tuomin.com/lemocardsms/sendcode"){
					_root["msg"] = "短信发送失败";
				}else{
					_root["msg"] = "验证码无效";
				}
			}
			DEBUG_PL(LOG_ERROR, "verify code phone %s, %s\n", _postdata.c_str(), _http_rspbuf.c_str());
			ret = END;
            _cur_step = STEP_END;
            break;
        default:
            ret = END;
            _cur_step = STEP_END;
            break;
    }   

    stat_step();
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

int CVerifyCodeObj::req_send_tencent(Msg &msg, uint32_t &msg_seq)
{
	Msg notifymsg;
	FILL_MSG(notifymsg, CMD_SEND_OPERATION_MAIL_REQ, _usrid.uin(), SendHttpReq,
			reqbody.set_url(_tencnetcodebuf);
			reqbody.set_sendtype(1);
			);  

    HttpTask *taskobj= new HttpTask(_proc, _proc->_mq_notify, &_proc->_mutex, notifymsg);
    _proc->puttask(taskobj);
    return 0;
}
int CVerifyCodeObj::rsp_send_tencent(Msg &msg)
{
    SendHttpRsp rspbody;
    if(!rspbody.ParseFromString(msg.body())){
        return ERR_PKG_PARSE;
    }
	string rspbuf = rspbody.rspbuf();
	Json::Reader reader;
	Json::Value root;
    DEBUG_PL(LOG_DEBUG, "tencent:%s\n", rspbuf.c_str());
    if (reader.parse(rspbuf, root)) {
        if(!root["response"].isNull()){
            if(root["response"].asString()!="1"){
				_root["code"] = -1043;
				_root["debugmsg"] = "parase tencent response!=1";
				return -1; 
			}
		}else{
			_root["code"] = -1043;
			_root["debugmsg"] = "parase tencent  has no key like 'response'";
			return -1; 
		}   
	}else{
        DEBUG_PL(LOG_ERROR,"uin:%u, tencent json failed\n", _usrid.uin());
        _root["code"] = -1043;
        _root["debugmsg"] = "parase tencent json failed";
        return -1; 
    }  
	return 0;

}



int CVerifyCodeObj::do_rsp()
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

void CVerifyCodeObj::generate_rspbody()
{
	Json::FastWriter writer;
	_rspbodycontent  = writer.write(_root);
	DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());
}

char* CVerifyCodeObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
