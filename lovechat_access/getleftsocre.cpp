#include "getleftsocre.h"
#include "pb_2_json.h"
#include "json/json.h"
#include "lemodbdbtask.h"

int CGetLeftSocre::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
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

	string cookie = _reqbody.get_cookie("sessionkey");
	if(cookie == ""){
		cookie = _reqbody.get_param("sessionkey");
	}
	if(cookie == ""|| cookie=="123" || cookie =="noAppSessionKey"){
		_root["code"] = -1043;
		_root["debugmsg"] = "need login!";
		_root["msg"] = "需要登录";

		return -1043;
	}

	_phonenum = aesDecode(cookie);
	if(_phonenum == ""){
		_root["code"] = -1045;
		_root["debugmsg"] = "need login!";
		_root["msg"] = "需要登录";

		return -1045;
	}
	DEBUG_PL(LOG_DEBUG, "cookie:%s this, phone:%s \n", cookie.c_str(), _phonenum.c_str());






    return ret;
}

int CGetLeftSocre::do_work(Msg &msg, uint32_t &msg_seq)
{	
	int ret = 0;
	stat_rspstep();
	switch(_cur_step){
		case STEP_BASE:
			ret = req_get_db(msg, msg_seq);
			if(ret==0){
				_cur_step = STEP_REQ_GET_DB;
				break;
			}
			ret = END;
			_cur_step = STEP_END;
			break;
		case STEP_REQ_GET_DB:
			ret = rsp_get_db(msg);
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
int CGetLeftSocre::req_get_redis(Msg &msg, uint32_t &msg_seq)
{

    DEBUG_PL(LOG_DEBUG, "xxx:%s\n", _phonenum.c_str()); 

    string redisdb = "8";
    CppRedis *db = (CppRedis*)_proc->get_redis_db(10000, redisdb);
    if(db == NULL){
        DEBUG_PL(LOG_ERROR, "db is null!\n");
        return -1; 
    }   

	string key = "fen_" + _phonenum;
    Msg notifymsg;
    FILL_MSG(notifymsg, CMD_CLOUD_REDIS_GET_REQ, 100000, CloudRedisGetReq,
			reqbody.set_type(REDIS_STRING);
			reqbody.set_key(key);
            )   
    RedisTask *taskobj = new RedisTask(_proc, _proc->_mq_notify, &_proc->_mutex, db, notifymsg);
    _proc->puttask(taskobj);
    DEBUG_PL(LOG_DEBUG, "notifymsg:%s\n", notifymsg.ShortDebugString().c_str());
    return 0;

}

int CGetLeftSocre::rsp_get_redis(Msg &msg)
{
	DEBUG_PL(LOG_DEBUG, "uin:%u retcode:%d\n", _usrid.uin(), msg.head().retcode());
	int ret = msg.head().retcode();
	if((ret != 0) && (ret != ERR_NO_RECORD)){
		Json::Value socre;
		socre["leftscore"] = 0;
		_root["data"] = socre;
		_root["code"] = 0;
		return _retcode = ret;
	}   
	if(ret == ERR_NO_RECORD){
		Json::Value socre;
		socre["leftscore"] = 0;
		_root["data"] = socre;
		_root["code"] = 0;
		return 0;  
	}   

	CloudRedisGetRsp rspbody;
	rspbody.ParseFromString(msg.body());

	_fen = rspbody.value();
	Json::Value socre;
	socre["leftscore"] = _fen;
	_root["data"] = socre;
	_root["code"] =0;
	DEBUG_PL(LOG_DEBUG, "phone:%s, fen:%s", _phonenum.c_str(), _fen.c_str());
	return 0;
}

int CGetLeftSocre::req_get_db(Msg &msg, uint32_t &msg_seq)
{
    CppMySQL3DB *db = (CppMySQL3DB*)_proc->get_db(10000, "lemoaccount");
    vector<string> columnlist;
    columnlist.push_back("*"); 
    string sql ;//= db->getSelectSql("score", columnlist, " where uin=" + to_str<unsigned>(_usrid.uin()));
	DEBUG_PL(LOG_DEBUG, "_phonenum:%s\n", _phonenum.c_str());
	sql = "select score from score where uin =(select uin from  profile where phonenum="+ string(_phonenum.c_str());
   	sql += "   );   ";

    Msg notifymsg;
    FILL_MSG(notifymsg, CMD_CLOUD_RDS_GET_REQ, 0, CloudMysqlGetReq,
            reqbody.set_mysqlquery(sql);
            reqbody.set_tablename("score");
            )   

	CLemoDbDBTask *taskobj = new CLemoDbDBTask(_proc, _proc->_mq_notify, &_proc->_mutex, db, notifymsg);
	_proc->puttask(taskobj);
	DEBUG_PL(LOG_DEBUG, "put to task.msg:%s\n", notifymsg.ShortDebugString().c_str());
	return 0;
}

int CGetLeftSocre::rsp_get_db(Msg &msg)
{
    DEBUG_PL(LOG_DEBUG, "uin :%u get db succcess\n", _usrid.uin());
    if(msg.head().retcode() != 0){
		Json::Value socre;
		socre["leftscore"] = 0;
		_root["data"] = socre;
		_root["code"] = 0;
		return _retcode = msg.head().retcode();
    }

    CloudMysqlGetRsp rspbody;
	Json::FastWriter writer;
	Json::Value info;
	rspbody.ParseFromString(msg.body());
	if(rspbody.itemlist_size() == 0){
		Json::Value socre;
		socre["leftscore"] = 0; 
		_root["data"] = socre;
		_root["code"] =0;
		return 0;
	}else{
		const MysqlRecordItem &record = rspbody.itemlist(0);
		for(int i = 0; i < record.itemlist_size(); ++i){
			const MysqlFieldItem &field = record.itemlist(i);
			if(field.fieldname()=="score"){
				Json::Value socre;
				socre["leftscore"] = (field.intfieldvalue()/100);
				_root["data"] = socre;
				_root["code"] =0;
			}else if (field.fieldname()=="reckon"){
			}
		}
	}
	return 0;

}



int CGetLeftSocre::do_rsp()
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

void CGetLeftSocre::generate_rspbody()
{
	Json::FastWriter writer;
	_rspbodycontent  = writer.write(_root);
	DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());
// http://blog.csdn.net/qiqingjin/article/details/51760343 服务器cookie 详解
}

char* CGetLeftSocre::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
