#include "loginobj.h"
#include "lemodbdbtask.h"

int CLoginObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
{
	// 客户端或者网页手机登录
    int ret = CCommObj::init(reqmsg, flow, ccdheader);
    if(ret != 0){ 
        DEBUG_P(LOG_ERROR, "(%s:%d)ret:%d", __FILE__, __LINE__, ret);
		_retcode = ERR_INIT;
		return _retcode;
    }   

    _svrseq = _proc->get_seq();

	std::map<std::string, std::string> paramap;
	 _reqbody.parse_request(reqmsg.body().c_str(), reqmsg.body().size());

	_reqbody.get_params(paramap);
	string captha;
	string phone;
	_is_new = false;
	_unionid = "";
	for(auto it = paramap.begin(); it!=paramap.end();++it){
		DEBUG_PL(LOG_DEBUG, "uin:%u key:%s, value:%s\n",_usrid.uin(), it->first.c_str(), it->second.c_str());
		if(it->first == "phonenum"){
			phone = it->second;
		}else if (it->first == "captcha"){
			captha = it->second;
		}else if (it->first == "unionid"){ 
			_unionid = it->second;
		}
	} 
	DEBUG_PL(LOG_DEBUG, "uin:%u phone:%s, captha:%s\n",_usrid.uin(), phone.c_str(), captha.c_str());
	if(phone=="" || captha == ""){
		_root["code"] = -1043;
		_root["debugmsg"] = "error param";
		return -1045;
	}
	_phonenum = phone;

	_checkcodeurl = "https://www.onluxy.com/lemocardsms/checkcode";
	_checkcodepostdata =  "secret=luxyisverygood&phone="+phone+"&code="+captha;
	if(CGetLocalIP::getlocalip() == "172.31.0.88"){
		_checkcodeurl = "https://test.onluxy.com/lemocardsms/checkcode";
	}
	DEBUG_PL(LOG_DEBUG, "locale ip :%s\n", CGetLocalIP::getlocalip().c_str());
    return ret;
}

int CLoginObj::do_work(Msg &msg, uint32_t &msg_seq)
{	
	int ret = 0;
	stat_rspstep();
	switch(_cur_step){
		case STEP_BASE:
			ret = req_get_httpapi(msg, msg_seq, _checkcodeurl, _checkcodepostdata);
			if(ret==0){
				_cur_step = STEP_REQ_SEND_HTTP;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_SEND_HTTP:
			ret = rsp_get_httpapi(msg, _http_rspbuf);
			if(_http_rspbuf=="0"){
				ret = req_get_db(msg, msg_seq);
				if(ret==0){
					_cur_step = STEP_REQ_GET_DB;
					break;
				}

			}else{
				_root["code"] = -1046;
				_root["debugmsg"] = "check faild!";
				_root["msg"] = "验证码错误请确认后重试";
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_GET_DB:
			ret = rsp_get_db(msg);
			if(ret==NEED_ALLOC){
				_is_new = true;
				ret = req_set_to_db(msg, msg_seq);
				_cur_step = STEP_REQ_SET_DB;
				break;
			}else if(ret==0){
				ret = req_update_db(msg, msg_seq);
				_cur_step = STEP_REQ_UPDATE_DB;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_UPDATE_DB:
			ret = rsp_set_to_db(msg);
			ret = req_get_yunxintoken(msg, msg_seq);
			if(ret==0){
				_cur_step = STEP_REQ_GET_DATA;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_SET_DB:
			ret = rsp_set_to_db(msg);
			if(ret==0){
				req_get_db(msg, msg_seq);
				_cur_step = STEP_REQ_GET_UINIDX_DB;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_GET_UINIDX_DB:
			ret = rsp_get_db(msg);
			ret = req_get_yunxintoken(msg, msg_seq);
			if(ret==0){
				_cur_step = STEP_REQ_GET_DATA;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;		
		case STEP_REQ_GET_DATA:
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

int CLoginObj::req_get_db(Msg &msg, unsigned &msg_seq)
{
	CppMySQL3DB *db = (CppMySQL3DB*)_proc->get_db(10000, "lemoaccount");
	vector<string> columnlist;
	columnlist.push_back("*");

	string sql = db->getSelectSql("openid_2_uin", columnlist, " where phonenum='" + _phonenum+"'");

	Msg notifymsg;
	FILL_MSG(notifymsg, CMD_CLOUD_RDS_GET_REQ, 0, CloudMysqlGetReq,
			reqbody.set_mysqlquery(sql);
			reqbody.set_tablename("openid_2_uin");
			)

	CLemoDbDBTask *taskobj = new CLemoDbDBTask(_proc, _proc->_mq_notify, &_proc->_mutex, db, notifymsg);
	_proc->puttask(taskobj);
	DEBUG_PL(LOG_DEBUG, "put to task.msg:%s\n", notifymsg.ShortDebugString().c_str());
	return 0;

}

int CLoginObj::rsp_get_db(Msg &msg)
{
	DEBUG_PL(LOG_DEBUG, "uin :%u get db succcess\n", _usrid.uin());
	if(msg.head().retcode() != 0){
		DEBUG_PL(LOG_ERROR, "uin:%u get db failed.ret:%d\n", _usrid.uin(), msg.head().retcode());
		return _retcode = msg.head().retcode();
	}    
	uint32_t uin = 0;
	CloudMysqlGetRsp rspbody;
	Json::FastWriter writer;
	rspbody.ParseFromString(msg.body());
	if(rspbody.itemlist_size() == 0){
		return NEED_ALLOC;
	}else{
		const MysqlRecordItem &record = rspbody.itemlist(0);
		for(int i = 0; i < record.itemlist_size(); ++i){
			const MysqlFieldItem &field = record.itemlist(i);
			if(field.fieldname()=="uin"){
				uin  = field.intfieldvalue();
			}
		}
	}
	if(uin!=0){
		_usrid.set_uin(uin);
		_head.mutable_usrid()->set_uin(uin);
		set_login();
		string key = aesEncode(_phonenum); 
		LemoCardSessionItem item;
		item.set_expirestamp(time(NULL)+3*86400);
		//item.set_openid(_openid);
		//item.set_sessionkey(_newsessionkey);
		item.set_uin(uin);
		string _data;
		item.SerializeToString(&_data);
		int ret =_proc->get_cache()->set(key.c_str(), _data.c_str(), _data.size());
		if(ret == 0){ 
			DEBUG_PL(LOG_DEBUG, "uin:%u, set caceh cuccess cache! key:%s, keylen:%d\n",_usrid.uin(), key.c_str(), key.size());
		}else{
			DEBUG_PL(LOG_DEBUG, "uin:%u, set caceh failed! ret:%d\n", _usrid.uin(), ret);
		}  
	}
	return 0;
}

int CLoginObj::req_set_to_db(Msg &msg, unsigned &msg_seq)
{
    map<string, int> intparam;
    intparam.insert(make_pair("stamp", time(NULL)));
    intparam.insert(make_pair("flag", 0)); 
    intparam.insert(make_pair("lastauthstamp", time(NULL))); 
    map<string, string> strparam;
    strparam.insert(make_pair("phonenum", _phonenum));

	if(_unionid!="") strparam.insert(make_pair("unionid", _unionid)); 
    CLoveChatLemoCardProc* proc = (CLoveChatLemoCardProc*)_proc;
    CppMySQL3DB *db = (CppMySQL3DB*)proc->get_db(10000, "lemoaccount");
    if(db == NULL){
        return _retcode = ERR_BID_CONF;
    }    
    string sql = db->getInsertSql("openid_2_uin", intparam, strparam, true);
    DEBUG_PL(LOG_DEBUG, "sql:%s\n", sql.c_str());
    Msg notifymsg;
    FILL_MSG(notifymsg, CMD_CLOUD_RDS_SET_REQ, 0, CloudMysqlSetReq,
            reqbody.set_mysqlquery(sql);
            );   

    CLemoDbDBTask *taskobj = new CLemoDbDBTask(proc, proc->_mq_notify, &proc->_mutex, db, notifymsg);
    proc->puttask(taskobj);
    DEBUG_PL(LOG_DEBUG, "put to task.msg:%s\n", notifymsg.ShortDebugString().c_str());
    return 0;
}

int CLoginObj::req_update_db(Msg &msg, unsigned &msg_seq)
{
    map<string, int> intparam;
    intparam.insert(make_pair("lastauthstamp", time(NULL))); 
    map<string, string> strparam;
	if(_unionid!="") strparam.insert(make_pair("unionid", _unionid)); 
    CLoveChatLemoCardProc* proc = (CLoveChatLemoCardProc*)_proc;
    CppMySQL3DB *db = (CppMySQL3DB*)proc->get_db(10000, "lemoaccount");
    if(db == NULL){
        return _retcode = ERR_BID_CONF;
    }    
	string condi = " where phonenum='"+_phonenum + "'";
    string sql = db->getUpdateSql("openid_2_uin", intparam, strparam, condi);
    DEBUG_PL(LOG_DEBUG, "sql:%s\n", sql.c_str());
    Msg notifymsg;
    FILL_MSG(notifymsg, CMD_CLOUD_RDS_SET_REQ, 0, CloudMysqlSetReq,
            reqbody.set_mysqlquery(sql);
            );   

    CLemoDbDBTask *taskobj = new CLemoDbDBTask(proc, proc->_mq_notify, &proc->_mutex, db, notifymsg);
    proc->puttask(taskobj);
    DEBUG_PL(LOG_DEBUG, "put to task.msg:%s\n", notifymsg.ShortDebugString().c_str());
    return 0;
}

int CLoginObj::rsp_set_to_db(Msg &msg)
{
    if(msg.head().retcode() != 0){
        DEBUG_PL(LOG_ERROR, "uin:%u failed. retcode:%d\n", _usrid.uin(), msg.head().retcode());
        return _retcode = msg.head().retcode();
    }    

    DEBUG_PL(LOG_ERROR, "uin:%u.  retcode:%d usrid:%s\n", _usrid.uin(), msg.head().retcode(), _usrid.ShortDebugString().c_str());
    return msg.head().retcode();
}

int CLoginObj::req_get_yunxintoken(Msg &msg, uint32_t &msg_seq)
{
	COMMON_SEND_REQV2("lovechat_lemoprofile", CMD_GET_NETEASEIMTOKEN_REQ, _usrid.uin(), GetNetEaseImTokenReq, 
			if(_is_new){
				reqbody.set_type(0);
			}else{
				reqbody.set_type(1);
				//reqbody.set_type(0);
			}
			reqbody.set_token(_socre["sessionkey"].asString());
			)
		//指定云信的token 就是自己的sessionkey
	return 0;
}

int CLoginObj::rsp_get_yunxintoken(Msg &msg)
{
	int ret = msg.head().retcode();
	DEBUG_PL(LOG_DEBUG, "uin:%d rsp_get_yunxintoken ret:%d\n", _usrid.uin(), ret);
	if(ret != 0){  
		return ret; 
	}    
	return 0;

}


void CLoginObj::set_login()
{
	string headrk("Set-Cookie");
	string headrv = aesEncode(_phonenum);
	string v = "sessionkey="+headrv;
	v += ";domain=cardapi.meetin.co;path=/;max-age=604800;SameSite=Lax;HttpOnly;secure";
	_rspbody.set_head(headrk, v);
	_socre["sessionkey"] = headrv;
	_socre["uin"] = _usrid.uin();
	_root["data"] = _socre;

	_root["code"] = 0;
}
int CLoginObj::do_rsp()
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

void CLoginObj::generate_rspbody()
{

	Json::FastWriter writer;
	_rspbodycontent  = writer.write(_root);
	DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());
	// http://blog.csdn.net/qiqingjin/article/details/51760343 服务器cookie 详解
}


char* CLoginObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
