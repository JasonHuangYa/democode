#include "decryptobj.h"
#include "aes.h"
#include "redistask.h"

int CDecryptObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
{
	int ret = CCommObj::init(reqmsg, flow, ccdheader);
	if(ret != 0){ 
		DEBUG_P(LOG_ERROR, "(%s:%d)ret:%d", __FILE__, __LINE__, ret);
		_retcode = ERR_INIT;
		return _retcode;
	}   

	_svrseq = _proc->get_seq();
	_reqbody.parse_request(reqmsg.body().c_str(), reqmsg.body().size());
	std::map<std::string, std::string> paramap;
	_reqbody.get_params(paramap);
	for( std::map<std::string, std::string>::iterator it = paramap.begin(); it!=paramap.end();++it){
		DEBUG_PL(LOG_DEBUG, "map key:%s, value:%s\n", it->first.c_str(), it->second.c_str());
	}
	_iv = _reqbody.get_param("iv");
	_encryptedData = _reqbody.get_param("encryptedData");
	_openid  = _reqbody.get_param("openid");
	_type = _reqbody.get_param("type"); //type 为空或者为1， 就是解密手机号   为2 就是解密群id


	return ret;
}

int CDecryptObj::do_work(Msg &msg, uint32_t &msg_seq)
{	
	int ret = 0;
	stat_rspstep();
	switch(_cur_step){
		case STEP_BASE:
			ret = decrypt();
			if(ret==0){
				req_set_redis(msg, msg_seq);
			}
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



int CDecryptObj::decrypt()
{

	if(_openid == ""){
		_root["code"] = -1043;
		_root["debugmsg"] = "Invalid openid";
		_root["msg"] = "手机号获取失败请手动输入";
		return -1;
	}
	CacheItem cacheitem;
	string key = _openid;
	LemoCardSessionItem item;
	int ret = get_incache(key, item);
	if(ret!=0){
		_root["code"] = -1045;
		_root["debugmsg"] = "No Such openid";
		_root["msg"] = "手机号获取失败请手动输入";
		return -1;
	}
	DEBUG_PL(LOG_DEBUG, "cache :%s\n", item.ShortDebugString().c_str());
	_sessionkey = item.sessionkey();

	IAES iaes;
	string oiv = base64_decode(_iv);
	_decryptedData = iaes.DecodeAES_CBC(base64_decode(_sessionkey), base64_decode(_encryptedData), oiv);
	if(_decryptedData==""){
		_root["code"] = -1044;
		_root["debugmsg"] = "Decode error";
		_root["msg"] = "手机号获取失败请手动输入";
		return -1;
	}
	_root["code"] = 0;
	Json::Reader reader;
	Json::Value rsp;
	DEBUG_PL(LOG_DEBUG, "decrypt data :%s\n", _decryptedData.c_str());
	if (reader.parse(_decryptedData, rsp)) {
		DEBUG_PL(LOG_DEBUG, "decrypt weixin data success:\n%s", rsp.toStyledString().c_str());
		if(_type==""|| _type=="1"){
			if(rsp["phoneNumber"].isNull()){
				_root["code"] = -1045;
				_root["debugmsg"] = "Decode error";
				_root["msg"] = "手机号获取失败请手动输入";
				return -1;
			}
			_phonenum = rsp.get("phoneNumber", "").asString();

		}else if (_type=="2") {
			if(rsp["openGId"].isNull()){
				_root["code"] = -1045;
				_root["debugmsg"] = "Decode group id error";
				return -1;
			}
		}else{

		}
		_root["_decryptedData"] = rsp;
		_root["data"] = rsp;
	}else{
		_root["code"] = -1045;
		_root["debugmsg"] = "Decode error";
		_root["msg"] = "手机号获取失败请手动输入";

		DEBUG_PL(LOG_ERROR,"openid:%s decrypt weixin json faild!\n", _openid.c_str());
		return -1;
	}
	return 0;
}

int CDecryptObj::req_set_redis(Msg &msg, uint32_t &msg_seq)
{
	if(_phonenum == ""){
		return -1;
	}
	string redisdb = "8";
	CppRedis *db = (CppRedis*)_proc->get_redis_db(10000, redisdb);
	if(db == NULL){
		DEBUG_PL(LOG_ERROR, "db is null!\n");
		return -1; 
	}   

	//weixin 的手机号在这里模拟发送过手机验证码, 七位数的验证码，不会和6位的发生碰撞
	string cmd = "set "+ _phonenum +" 8888888";
	string cmd2 = "expire "+ _phonenum +"3600";
	Msg notifymsg;
	FILL_MSG(notifymsg, CMD_CLOUD_REDIS_TEST_REQ, 100000, CloudRedisDoCmdReq,
			reqbody.add_cmdlist(cmd);
			reqbody.add_cmdlist(cmd2);
			)   
	RedisTask *taskobj = new RedisTask(_proc, _proc->_mq_notify, &_proc->_mutex, db, notifymsg);
	_proc->puttask(taskobj);
	DEBUG_PL(LOG_DEBUG, "notifymsg:%s\n", notifymsg.ShortDebugString().c_str());
	return 0;

}

int CDecryptObj::rsp_set_redis(Msg &msg)
{
	return 0;
}

int CDecryptObj::do_rsp()
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

void CDecryptObj::generate_rspbody()
{
	//_root["rsp"] = _decryptedData;
	Json::FastWriter writer;
	_rspbodycontent  = writer.write(_root);
	DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());
	//_rspbody.SerializeToString(&_rspbodycontent);
	//DEBUG_PL(LOG_DEBUG, "rspbody:%s\n", _rspbody.ShortDebugString().c_str());
}

char* CDecryptObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
