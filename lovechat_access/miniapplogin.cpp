#include "miniapplogin.h"
#include "lemodbdbtask.h"

int CMiniAppLoginObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
{
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
	for(auto it = paramap.begin(); it!=paramap.end();++it){
		DEBUG_PL(LOG_DEBUG, "uin:%u key:%s, value:%s\n",_usrid.uin(), it->first.c_str(), it->second.c_str());
		if(it->first == "phonenum"){
			phone = it->second;
		}else if (it->first == "captcha"){
			captha = it->second;
		}
	} 
	_need_verifycode = true;
	DEBUG_PL(LOG_DEBUG, "uin:%u phone:%s, captha:%s\n",_usrid.uin(), phone.c_str(), captha.c_str());
	if(phone==""){
		_root["code"] = -1043;
		_root["debugmsg"] = "error param";
		return -1045;
	}
	_uin_alloc = 0;
	_phonenum = phone;
	_checkcodeurl = "https://www.onluxy.com/lemocardsms/checkcode";
	if(CGetLocalIP::getlocalip() == "172.31.0.88"){
		_checkcodeurl = "https://test.onluxy.com/lemocardsms/checkcode";
	} 
	_checkcodepostdata =  "secret=luxyisverygood&phone="+phone+"&code="+captha;
	if(captha==""){
		_need_verifycode =false;
	}
	//在小程序 绑定手机号的
    return ret;
}

int CMiniAppLoginObj::do_work(Msg &msg, uint32_t &msg_seq)
{	
	int ret = 0;
	stat_rspstep();
	switch(_cur_step){
		case STEP_BASE:
			if(_need_verifycode == false){
				goto GETDB;
			}
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
GETDB:			ret = req_get_db(msg, msg_seq);
				if(ret==0){
					_cur_step = STEP_REQ_GET_DB;
					break;
				}

			}else{
				_root["code"] = -1046;
				_root["debugmsg"] = "check faild!";
				_root["msg"] = "无效的验证码";
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_GET_DB:
			ret = rsp_get_db(msg);
			if(ret==NEED_ALLOC){
				_root["code"]=0;
				_root["data"]["isnew"]=1;
				_cur_step = STEP_END;
				ret = END;
				break;
			}else if(ret==0){
				ret = req_set_to_db(msg, msg_seq);
				ret = req_set_to_db_delete(msg, msg_seq);
				_root["code"]=0;
				_root["data"]["isnew"] = 0;
				_root["data"]["openid"] = get_openid(_uin_alloc);
				_cur_step = STEP_REQ_UPDATE_DB;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_UPDATE_DB:
			ret = rsp_set_to_db(msg);
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

int CMiniAppLoginObj::req_get_db(Msg &msg, unsigned &msg_seq)
{
	CppMySQL3DB *db = (CppMySQL3DB*)_proc->get_db(10000, "lemoaccount");
	vector<string> columnlist;
	columnlist.push_back("*");
	//select * from openid_2_uin where uin=10072 or phonenum  = '18482105879';
	string sql = db->getSelectSql("openid_2_uin", columnlist, " where phonenum='" + _phonenum+"'"  + "or uin = "+to_str<uint32_t>(_usrid.uin()));

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

int CMiniAppLoginObj::rsp_get_db(Msg &msg)
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
		//理论上这种情况不会出现
		return NEED_ALLOC;
	}else if (rspbody.itemlist_size() == 1){
		DEBUG_PL(LOG_ERROR, "uin:%u has no regist app\n", _usrid.uin());
		return NEED_ALLOC;
		//第一次注册小程序，没有注册客户端
		//仅仅拉到了uin的数据，没有拉到phonenum 数据
	}else {
		const MysqlRecordItem &record = rspbody.itemlist(0);
		for(int i = 0; i < record.itemlist_size(); ++i){
			const MysqlFieldItem &field = record.itemlist(i);
			if(field.fieldname()=="uin"){
				uin = field.intfieldvalue();
				if(uin == _usrid.uin()){
					_miniapp_record = rspbody.itemlist(0);
					_androidios_record = rspbody.itemlist(1);
				}else{
					_miniapp_record = rspbody.itemlist(1);
					_androidios_record = rspbody.itemlist(0);
				}
			}
		}
	}
	return 0;
}

int CMiniAppLoginObj::req_set_to_db(Msg &msg, unsigned &msg_seq)
{

	uint32_t androidiosuin = 0;
	string openid, sessionkey;
	
	for(int i = 0; i <_androidios_record.itemlist_size(); ++i){
		 const MysqlFieldItem &field = _androidios_record.itemlist(i);
		 if(field.fieldname()=="uin"){
			androidiosuin = field.intfieldvalue();
			_uin_alloc = androidiosuin;
		 }
	}

	 for(int i = 0; i <_miniapp_record.itemlist_size(); ++i){
		 const MysqlFieldItem &field = _miniapp_record.itemlist(i);
		 if(field.fieldname()=="openid"){
			 openid =  field.fieldvalue();
		 }else if(field.fieldname()=="sessionkey"){
			 sessionkey = field.fieldvalue();
		 }
	 }
	 string sql = "update openid_2_uin set openid = '" + openid +"', sessionkey='" +sessionkey+"' where uin="+to_str<uint32_t>(androidiosuin) + " and phonenum='"+ _phonenum+ "'";

	 DEBUG_PL(LOG_ERROR, "uin:%u, sqlupdate:%s \n", _usrid.uin(), sql.c_str());



    CLoveChatLemoCardProc* proc = (CLoveChatLemoCardProc*)_proc;
    CppMySQL3DB *db = (CppMySQL3DB*)proc->get_db(10000, "lemoaccount");
    if(db == NULL){
        return _retcode = ERR_BID_CONF;
    }    
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


int CMiniAppLoginObj::req_set_to_db_delete(Msg &msg, unsigned &msg_seq)
{

	string openid, sessionkey;
	
	 for(int i = 0; i <_miniapp_record.itemlist_size(); ++i){
		 const MysqlFieldItem &field = _miniapp_record.itemlist(i);
		 if(field.fieldname()=="openid"){
			 openid =  field.fieldvalue();
		 }else if(field.fieldname()=="sessionkey"){
			 sessionkey = field.fieldvalue();
		 }
	 }
	 string sql = "delete from openid_2_uin where uin=" + to_str<uint32_t>(_usrid.uin()) + " and openid='"+openid+"' and openid!=''" ;
	 // delete 语句条件严格限制

	 DEBUG_PL(LOG_ERROR, "uin:%u, sqldelete:%s \n", _usrid.uin(), sql.c_str());



    CLoveChatLemoCardProc* proc = (CLoveChatLemoCardProc*)_proc;
    CppMySQL3DB *db = (CppMySQL3DB*)proc->get_db(10000, "lemoaccount");
    if(db == NULL){
        return _retcode = ERR_BID_CONF;
    }    
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


int CMiniAppLoginObj::rsp_set_to_db(Msg &msg)
{
    if(msg.head().retcode() != 0){
        DEBUG_PL(LOG_ERROR, "uin:%u failed. retcode:%d\n", _usrid.uin(), msg.head().retcode());
        return _retcode = msg.head().retcode();
    }    

    DEBUG_PL(LOG_ERROR, "uin:%u.  retcode:%d usrid:%s\n", _usrid.uin(), msg.head().retcode(), _usrid.ShortDebugString().c_str());
    return msg.head().retcode();
}


int CMiniAppLoginObj::do_rsp()
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

void CMiniAppLoginObj::generate_rspbody()
{

	Json::FastWriter writer;
	_rspbodycontent  = writer.write(_root);
	DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());
	// http://blog.csdn.net/qiqingjin/article/details/51760343 服务器cookie 详解
}


char* CMiniAppLoginObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
