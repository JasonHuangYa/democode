#include "lemocardcommobj.h"
#include "httptask.h"
#include "time_stat.h"

char* CLemoCardCommObj::generate_rsp_buf(unsigned &bufsize)
{
	string ver = "HTTP/1.1";
	_rspbody.set_body(_rspbodycontent);
	_rspbody.gen_response(ver, 0); 
	int readsize = 0;
	_ccdheader._type = ccd_req_data;
	memcpy(_objbuf, (char*)&_ccdheader, CCD_HEADER_LEN);
	_rspbody.readsome(_objbuf+CCD_HEADER_LEN, OBJ_BUF_LEN, readsize);
	bufsize = CCD_HEADER_LEN + readsize;
	if(_retcode == 0){  
		DEBUG_PL(LOG_DEBUG, "rspmsg.uin:%u cmd:%d step:%d flow:%u svrseq:%u cliseq:%u msgseq:%u retcode:%d\n", _usrid.uin(), _cmd, _cur_step, _flow, _clisvrseq, _cliseq, _msg_seq, _retcode);  
	}else{
		DEBUG_PL(LOG_ERROR, "rspmsg.uin:%u cmd:%d step:%d flow:%u svrseq:%u cliseq:%u msgseq:%u retcode:%d\n", _usrid.uin(), _cmd, _cur_step, _flow, _clisvrseq, _cliseq, _msg_seq, _retcode);    
	}

	return _objbuf;
}

int CLemoCardCommObj:: req_get_httpapi(Msg &msg, uint32_t &msg_seq, string &url, string &postdata)
{

    Msg notifymsg;
	DEBUG_PL(LOG_DEBUG, "url:%s\n", url.c_str());
    FILL_MSG(notifymsg, CMD_SEND_OPERATION_MAIL_REQ, _usrid.uin(), SendHttpReq,
            reqbody.set_url(url);
            reqbody.set_postpara(postdata);
            );  

    HttpTask *taskobj= new HttpTask(_proc, _proc->_mq_notify, &_proc->_mutex, notifymsg);
    _proc->puttask(taskobj);
    return 0;
}

int CLemoCardCommObj::rsp_get_httpapi(Msg &msg, string &rspbuf)
{
    SendHttpRsp rspbody;
    if(!rspbody.ParseFromString(msg.body())){
        return ERR_PKG_PARSE;
    }
	rspbuf = rspbody.rspbuf();
	return 0;

}

int CLemoCardCommObj::get_incache(string &key, LemoCardSessionItem &item)
{
	CacheItem cacheitem;
	int ret = CGetCache::getcache((void*)_proc->get_cache(), key, cacheitem);         
	if(ret == 0){ 
		if(!item.ParseFromString(string(cacheitem._buf, cacheitem._data_len))){      
			return NOT_IN_CACHE;                                                      
		}                                                                             
	}else{
		DEBUG_PL(LOG_DEBUG, "not in cache! ret:%d\n", ret);                           
		return NOT_IN_CACHE;
	}  
	return 0;
}

int CLemoCardCommObj::set_incache(string &key, LemoCardSessionItem &item)
{
	string _data;
	item.SerializeToString(&_data);
	int ret =_proc->get_cache()->set(key.c_str(), _data.c_str(), _data.size());
	if(ret == 0){ 
		DEBUG_PL(LOG_DEBUG, "set caceh cuccess cache! key:%s, keylen:%d\n", key.c_str(), key.size());
	}else{
		DEBUG_PL(LOG_DEBUG, "set caceh failed! ret:%d\n", ret);
		return -1043;
	}   
	return 0;
}

string  CLemoCardCommObj::aesEncode(const string tdata) 
{
    string openid = "";
    string ipasswd = "account;;;xxxxxx";
    IAES iaes;
    string taceresult = iaes.EncodeAES_CBC(ipasswd, tdata);
    openid = CStr::bin_2_str(taceresult.c_str(), taceresult.size());
    return openid;
}

string CLemoCardCommObj::aesDecode(const string &openid)
{
    string ipasswd = "account;;;xxxxxx";
    IAES iaes;
    string uin = CStr::str_2_bin(openid);
    string taceresult = iaes.DecodeAES_CBC(ipasswd,uin);
	return taceresult;
}

string CLemoCardCommObj::get_statname()
{
    map<unsigned, string>::iterator it;
    it = _proc->_cmd_2_str.find(_head.cmd());
    string cmd; 
    if(it == _proc->_cmd_2_str.end()){
		 cmd = _reqbody.get_request_uri();
    }else{
        cmd = it->second;
    }    
/*
    map<unsigned, string>::iterator it1; 
    it1 = _proc->_step_2_str.find(_cur_step);
    string step;
    if(it1 == _proc->_step_2_str.end()){
        step = to_str<unsigned>(_cur_step);
    }else{
        step = it1->second;
    }    
*/
    return " "+cmd; 
}

void CLemoCardCommObj::stat_step()
{
    string name = get_statname();
    struct timeval t1;
    CTimeStat::time_begin(t1);
    _statmap[name] = t1;

}

void CLemoCardCommObj::stat_rspstep()
{
    string name = get_statname();
    if(_statmap.find(name) != _statmap.end()){
        //stat rsp
        struct timeval t2;
        CTimeStat::time_end(t2);
        struct timeval &t1 = _statmap[name];
        _statmap.erase(name);
        CTimeStat::stat(name, _head.bizid(), _retcode, &t1, &t2);
    }   
}

void CLemoCardCommObj::on_expire()
{
	_retcode = ERR_TIME_OUT; 
	DEBUG_PL(LOG_ERROR, "timeout.uin:%u uri:%s cmd:%d step:%d\n", _usrid.uin(), _reqbody.get_request_uri().c_str() ,_cmd, _cur_step); do_rsp(); 
	_root["code"] = _retcode;
	_root["debugmsg"] = "timeout";
	_root["msg"] = "服务器挖矿去了，请稍后重试";
	do_rsp();
}
