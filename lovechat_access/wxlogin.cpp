#include "wxlogin.h"
#include "lovechat_lemocard_proc.h"
#include "lemodbdbtask.h"

int CWxLoginObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
{
	// 客户端 调微信的sdk ，实现微信login
    int ret = CCommObj::init(reqmsg, flow, ccdheader);
    if(ret != 0){ 
        DEBUG_P(LOG_ERROR, "(%s:%d)ret:%d", __FILE__, __LINE__, ret);
		_retcode = ERR_INIT;
		return _retcode;
    }   
	_svrseq = _proc->get_seq();

	_reqbody.parse_request(reqmsg.body().c_str(), reqmsg.body().size());
	_unionid = _reqbody.get_param("unionid");	
	if(_unionid==""){
		_root["code"] = -1048;
		_root["debugmsg"] = "code is invalid";

		return -1043;
	}

    return ret;
}

int CWxLoginObj::do_work(Msg &msg, uint32_t &msg_seq)
{	
    int ret = 0;
    stat_rspstep();
    switch(_cur_step){
        case STEP_BASE:
			if(ret==0){
				ret = req_get_db(msg, msg_seq);
				if(ret==0){
					_cur_step = STEP_REQ_GET_DB;
					break;
				}
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_GET_DB:
			ret = rsp_get_db(msg);
			if(ret==NEED_ALLOC){
				//ret = req_set_to_db(msg, msg_seq);
				//_cur_step = STEP_REQ_SET_DB;
				_cur_step = STEP_END;
				ret = END;
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
			req_get_yunxintoken(msg, msg_seq, 1);
			_cur_step = STEP_REQ_GET_DATA;
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

int CWxLoginObj::do_rsp()
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

int CWxLoginObj::req_get_db(Msg &msg, unsigned &msg_seq)
{
    CLoveChatLemoCardProc* proc = (CLoveChatLemoCardProc*)_proc;
    CppMySQL3DB* db = (CppMySQL3DB*)proc->get_db(10000,"lemoaccount"); 
    if(db == NULL){
        return _retcode = ERR_BID_CONF;
    } 
    vector<string>columnlist;
    columnlist.push_back("*");
   

	string sql; 
	sql = db->getSelectSql("openid_2_uin", columnlist, " where unionid='" + _unionid +"'"); 
	Msg notifymsg;
	FILL_MSG(notifymsg, CMD_CLOUD_RDS_GET_REQ, 0, CloudMysqlGetReq,
            reqbody.set_mysqlquery(sql);
            reqbody.set_tablename("openid_2_uin");
            )    
    CLemoDbDBTask *taskobj = new CLemoDbDBTask(proc, proc->_mq_notify, &proc->_mutex, db, notifymsg);
    proc->puttask(taskobj);
    DEBUG_PL(LOG_DEBUG, "put to task.msg:%s\n", notifymsg.ShortDebugString().c_str());
    return 0;
}

int CWxLoginObj::rsp_get_db(Msg &msg)
{
	if(msg.head().retcode() != 0){
		DEBUG_PL(LOG_DEBUG, "no uin, need alloc. usrid:%s retcode:%d\n", _usrid.ShortDebugString().c_str(), msg.head().retcode());
		return  _retcode = ERR_ALLOC_UIN_FAILED;
	}

	CloudMysqlGetRsp rspbody;
	rspbody.ParseFromString(msg.body());
	DEBUG_PL(LOG_DEBUG, "usrid:%s rspbody:%s\n", _usrid.ShortDebugString().c_str(), rspbody.ShortDebugString().c_str());
	if(rspbody.itemlist_size() <= 0){
		_subroot["isnew"] = 1;
		_root["code"] = 0;
		_root["data"] = _subroot;
		return NEED_ALLOC;
	}


	MysqlRecordItem record;
	for(int j = 0; j < rspbody.itemlist_size(); ++j){
		//TODO may select 2 row
		const MysqlRecordItem &tmprecord = rspbody.itemlist(j);
		record = tmprecord;
		break;
	}

	for(int i = 0; i < record.itemlist_size(); ++i){
		const MysqlFieldItem &field = record.itemlist(i);
		if(field.fieldname() == "uin"){
			_uin_alloc = field.intfieldvalue();
			_head.mutable_usrid()->set_uin(_uin_alloc);
			_usrid.set_uin(_uin_alloc);
			//valid_alloc(_uin_alloc);
			DEBUG_PL(LOG_DEBUG, "get uin success.usrid:%s\n", _usrid.ShortDebugString().c_str());
			//更新到内存
		}else if(field.fieldname() == "phonenum"){
			_phonenum = field.fieldvalue();
		}
	}
	if(_phonenum==""){
		_subroot["isnew"] = 1;
		_root["code"] = 0;
		_root["data"] = _subroot;
		return NEED_ALLOC;
	}

	if(_uin_alloc!=0){
		//string key = get_openid(_uin_alloc);
		string key = aesEncode(_phonenum);
		LemoCardSessionItem item;
		item.set_expirestamp(time(NULL)+3*86400);
		item.set_sessionkey(key);
		item.set_uin(_uin_alloc);
		item.SerializeToString(&_data);
		_subroot["isnew"] = 0;
		_subroot["sessionkey"] = key;
		_subroot["uin"] = _uin_alloc;
		_root["code"] = 0;
		_root["data"] = _subroot;
		int ret =_proc->get_cache()->set(key.c_str(), _data.c_str(), _data.size());
		if(ret == 0){ 
			DEBUG_PL(LOG_ERROR, "set caceh cuccess cache! key:%s, keylen:%d\n", key.c_str(), key.size());
		}else{
			DEBUG_PL(LOG_ERROR, "uin:%u set caceh failed! ret:%d\n", _uin_alloc, ret);
		}
	}
	return 0; 
}

int CWxLoginObj::req_set_to_db(Msg &msg, unsigned &msg_seq)
{
    map<string, int> intparam;
    intparam.insert(make_pair("stamp", time(NULL)));
    intparam.insert(make_pair("flag", 0)); 
    intparam.insert(make_pair("lastauthstamp", time(NULL))); 
    map<string, string> strparam;
	strparam.insert(make_pair("unionid", _unionid));
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

int CWxLoginObj::req_update_db(Msg &msg, unsigned &msg_seq)
{
    map<string, int> intparam;
    intparam.insert(make_pair("lastauthstamp", time(NULL))); 
    map<string, string> strparam;
	if(_unionid!=""){
		strparam.insert(make_pair("unionid", _unionid));
	}
    CLoveChatLemoCardProc* proc = (CLoveChatLemoCardProc*)_proc;
    CppMySQL3DB *db = (CppMySQL3DB*)proc->get_db(10000, "lemoaccount");
    if(db == NULL){
        return _retcode = ERR_BID_CONF;
    }    
	string condi = "where unionid='"+ _unionid + "'";
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

int CWxLoginObj::rsp_set_to_db(Msg &msg)
{
    if(msg.head().retcode() != 0){
        DEBUG_PL(LOG_ERROR, "uin:%u alloc:%u failed. retcode:%d\n", _usrid.uin(), _uin_alloc, msg.head().retcode());
        return _retcode = msg.head().retcode();
    }    

    DEBUG_PL(LOG_ERROR, "uin:%u. allocl:%u retcode:%d usrid:%s\n", _usrid.uin(), _uin_alloc, msg.head().retcode(), _usrid.ShortDebugString().c_str());
    return msg.head().retcode();
}

void CWxLoginObj::generate_rspbody()
{
	Json::FastWriter writer;
	_rspbodycontent  = writer.write(_root);
	DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());
}


int CWxLoginObj::req_get_yunxintoken(Msg &msg, uint32_t &msg_seq, uint32_t type)
{
	COMMON_SEND_REQV2("lovechat_lemoprofile", CMD_GET_NETEASEIMTOKEN_REQ, _uin_alloc, GetNetEaseImTokenReq, 
			reqbody.set_type(type);
			//reqbody.set_token(get_openid(_uin_alloc));	
			reqbody.set_token(aesEncode(_phonenum));
			)   
		//指定云信的token 就是自己的sessionkey
    return 0;
}

int CWxLoginObj::rsp_get_yunxintoken(Msg &msg)
{
    int ret = msg.head().retcode();
    DEBUG_PL(LOG_DEBUG, "uin:%d rsp_get_profile ret:%d\n", _usrid.uin(), ret);
    if(ret != 0){
        return ret;
    }
    return 0;

}




char* CWxLoginObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
