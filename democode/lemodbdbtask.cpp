#include "lemodbdbtask.h"

void CLemoDbDBTask::doit()
{
	int ret = 0;
	switch(_msg.head().cmd()){
		case CMD_CLOUD_RDS_SET_REQ:
			ret = do_set_mysql();
			break;
		case CMD_CLOUD_RDS_GET_REQ:
			ret = do_get_mysql();
			break;
		default:
			break;
	}

	_retcode = ret;
	_rspmsg.mutable_head()->set_cmd(_msg.head().cmd() + CMD_REQ_RSP_SPAN);
	DEBUG_PL(LOG_DEBUG, "sql:_retcode:%d\n", _retcode);
	enqueue_notify();
}


int CLemoDbDBTask::do_set_mysql()
{
	int ret = check_param();
	if(ret != 0){
		return ret;
	}

	CloudMysqlSetReq body;
	body.ParseFromString(_msg.body());
	string errmsg;
	string sql = body.mysqlquery();
	MutexGuard g(_db->_dbmutex);
	ret =_db->execSQL(sql, errmsg);
	if(ret < 0){
		DEBUG_PL(LOG_ERROR, "ret:%d errmsg:%s\n", ret, errmsg.c_str());
	}else{
		ret = 0;
	}

	return ret;
}

/*
 * message MysqlFieldItem                                                               
 * */
#define AddFieldToRecord(QUERY, RECORD, COLNAME, TYPE) \
{\
	MysqlFieldItem *field = RECORD->add_itemlist();\
	field->set_fieldtype(TYPE);\
	field->set_fieldname(COLNAME);\
	if(TYPE == DB_INT){\
		field->set_intfieldvalue(QUERY.getIntField(COLNAME));\
	}else{\
		field->set_fieldvalue(QUERY.getStringField(COLNAME));\
	}\
}\

int CLemoDbDBTask::do_get_mysql()
{
	int ret = check_param();
	if(ret != 0){
		return ret;
	}

	CloudMysqlGetReq body;
	body.ParseFromString(_msg.body());
	string errmsg;
	int retcode = 0;
	string sql = body.mysqlquery();
	MutexGuard g(_db->_dbmutex);
	CppMySQLQuery query =_db->querySQL(sql, retcode, errmsg);
	DEBUG_PL(LOG_DEBUG, "result size:%d sql:%s retcode:%d errmsg:%s\n", query.numRow(), sql.c_str(), retcode, errmsg.c_str());
	CloudMysqlGetRsp rspbody;
	if(query.numRow() > 0){ 
		if(body.tablename() == "openid_2_uin"){
			for(query.seekRow(0); !query.eof(); query.nextRow()){
				MysqlRecordItem *record = rspbody.add_itemlist();
				AddFieldToRecord(query, record, "uin", DB_INT);
				AddFieldToRecord(query, record, "openid", DB_STR);
				AddFieldToRecord(query, record, "stamp", DB_INT);
				AddFieldToRecord(query, record, "sessionkey", DB_STR);
				AddFieldToRecord(query, record, "lastauthstamp", DB_INT);
				AddFieldToRecord(query, record, "phonenum", DB_STR);
				AddFieldToRecord(query, record, "flag", DB_INT);
			}
		}else if(body.tablename() == "profile"){
			for(query.seekRow(0); !query.eof(); query.nextRow()){
				MysqlRecordItem *record = rspbody.add_itemlist();
				AddFieldToRecord(query, record, "uin", DB_INT);
				AddFieldToRecord(query, record, "headimg", DB_STR);
				AddFieldToRecord(query, record, "name", DB_STR);
				AddFieldToRecord(query, record, "phonenum", DB_STR);
				AddFieldToRecord(query, record, "company", DB_STR);
				AddFieldToRecord(query, record, "job", DB_STR);
				AddFieldToRecord(query, record, "content", DB_STR);
				AddFieldToRecord(query, record, "stamp", DB_INT);
				AddFieldToRecord(query, record, "mask", DB_INT);
				AddFieldToRecord(query, record, "openid", DB_STR);
				AddFieldToRecord(query, record, "gender", DB_STR);
			}

		}else if(body.tablename() == "score"){
			for(query.seekRow(0); !query.eof(); query.nextRow()){
				MysqlRecordItem *record = rspbody.add_itemlist();
				AddFieldToRecord(query, record, "score", DB_INT);
			}
		}
	}

	DEBUG_PL(LOG_DEBUG, "rspbody:%s\n", rspbody.ShortDebugString().c_str());
	string data;
	rspbody.SerializeToString(&data);
	_rspmsg.set_body(data);
	return retcode;
	
}

int CLemoDbDBTask::check_param()
{
	if(_db == NULL){
		return ERR_INITED;
	}

	return 0;
}
//
//
//
