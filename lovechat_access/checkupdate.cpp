#include "checkupdate.h"
#include "aes.h"
#include "redistask.h"

int CCheckupdateObj::init(const Msg &reqmsg, const uint32_t flow, TCCDHeader *ccdheader)
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

	_buildver = _reqbody.get_param("buildver");
	_product = _reqbody.get_param("product");
	if(_buildver==""){
		_buildver = _reqbody.get_header("Builderver");
	}
	if(_product==""){
		_product = _reqbody.get_header("Product");
	}
	if(_buildver=="" || _product=="" ||  from_str<uint32_t>(_buildver)==0){
		_root["code"]=-1043;
		_root["debugmsg"] = "error param";
	}


	return ret;
}

int CCheckupdateObj::do_work(Msg &msg, uint32_t &msg_seq)
{	
	int ret = 0;
	stat_rspstep();
	switch(_cur_step){
		case STEP_BASE:
			create_rsp();
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

int CCheckupdateObj::create_rsp()
{

	string androidversion  = ProcPoint->get_fieldconf(10000, "androidversion");
	string iosversion = ProcPoint->get_fieldconf(10000, "iosversion");
	string android_downloadurl = ProcPoint->get_fieldconf(10000, "android_downloadurl");
	string ios_downloadurl = ProcPoint->get_fieldconf(10000, "ios_downloadurl");
	string alertwording = ProcPoint->get_fieldconf(10000, "update_alert");
	uint32_t ios_onlinever = from_str<uint32_t>(ProcPoint->get_fieldconf(10000, "ios_onlinever"));
	uint32_t android_onlinever = from_str<uint32_t>(ProcPoint->get_fieldconf(10000, "android_onlinever"));
	_root["code"]=0;
	if(_product=="3"){
		Json::Value confinfo;
		if(android_onlinever < from_str<uint32_t>(_buildver)){
			confinfo["oncheck"] = 1;
		}else{
			confinfo["oncheck"] = 0;
		}
		if(from_str<uint32_t>(androidversion) > from_str<uint32_t>(_buildver)){
			_root["data"]["hasnew"]=1;
			_root["data"]["url"] = android_downloadurl;
			_root["data"]["alert"] = alertwording;
		}else{
			_root["data"]["hasnew"]=0;
			_root["data"]["url"] = "";
			_root["data"]["alert"] = "";
		}
		_root["data"]["confinfo"] = confinfo;


	}else if(_product=="2") {
		Json::Value confinfo;
		if(ios_onlinever < from_str<uint32_t>(_buildver)){
			confinfo["oncheck"] = 1;
		}else{
			confinfo["oncheck"] = 0;
		}
		if(from_str<uint32_t>(iosversion) > from_str<uint32_t>(_buildver)){
			_root["data"]["hasnew"]=1;
			_root["data"]["url"] = ios_downloadurl;
			_root["data"]["alert"] = alertwording;
		}else{
			_root["data"]["hasnew"]=0;
			_root["data"]["url"] = "";
			_root["data"]["alert"] = "";
		}	
		_root["data"]["confinfo"] = confinfo;
	}else{
		_root["data"]["hasnew"]=0;
		_root["data"]["url"] = "";
		_root["data"]["alert"] = "";

	}

	return 0;
}



int CCheckupdateObj::do_rsp()
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

void CCheckupdateObj::generate_rspbody()
{
	//_root["rsp"] = _decryptedData;
	Json::FastWriter writer;
	_rspbodycontent  = writer.write(_root);
	DEBUG_PL(LOG_DEBUG, "_rspbodycontent:%s, size:%d\n",  _root.toStyledString().c_str(), _rspbodycontent.size());
	//_rspbody.SerializeToString(&_rspbodycontent);
	//DEBUG_PL(LOG_DEBUG, "rspbody:%s\n", _rspbody.ShortDebugString().c_str());
}

char* CCheckupdateObj::get_rsp(unsigned &bufsize, unsigned &flow)
{
	//standard methond
	generate_rspbody();
	flow = _flow;
	return generate_rsp_buf(bufsize);
}
