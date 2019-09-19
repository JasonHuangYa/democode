#include"httptask.h"
/*
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
                ___/`---'\____
            .'  \\|      |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
     佛祖保佑    永无BUG
*/

void HttpTask::doit()
{
	int ret = 0;
	SendHttpReq body;
	body.ParseFromString(_msg.body());
	DEBUG_PL(LOG_DEBUG,"reqbody :%s\n", body.ShortDebugString().c_str());
	string url =  body.url();
	string buffer,errinfo;
	if(body.sendtype()==1){
		ret = libcurl_get(url.c_str(),buffer,errinfo);
	}else{
		ret = libcurl_post(url.c_str(), body.postpara().c_str(), buffer,errinfo);
	}
	SendHttpRsp rspbody;
	if(errinfo!="")
		DEBUG_PL(LOG_DEBUG,"error info :%s\n", errinfo.c_str());
	rspbody.set_rspbuf(buffer);
	string data;
	rspbody.SerializeToString(&data);
	_rspmsg.set_body(data);

	_retcode = ret;
	_rspmsg.mutable_head()->set_cmd(_msg.head().cmd() + CMD_REQ_RSP_SPAN);
	enqueue_notify();

}


bool HttpTask:: init(CURL*& conn, const char* url, std::string* p_buffer)
{
	CURLcode code;

	conn = curl_easy_init();
	if (NULL == conn)
	{
		DEBUG_PL(LOG_ERROR,"Failed to create CURL connection\n");
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, error_buffer);
	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR,"Failed to set error buffer :%d\n",code);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_URL, url);
	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR,"Failed to set URL :%s\n",error_buffer);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1);
	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR,"Failed to set redirect option : %s\n",error_buffer);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR,"Failed to set writer : %s\n",error_buffer);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, p_buffer);
	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR," Failed to set write data %s\n",error_buffer); 
		return false;
	}
	struct curl_slist *headers = NULL;

	//增加HTTP header
//	headers = curl_slist_append(headers, "Accept:application/json");
//	headers = curl_slist_append(headers, "Content-Type:application/json");
//	headers = curl_slist_append(headers, "charset:utf-8");
//	curl_easy_setopt(conn, CURLOPT_HTTPHEADER, headers);
	return true;

}

int HttpTask::writer(char* data, size_t size, size_t nmemb, std::string* writer_data)
{
	unsigned long sizes = size * nmemb;

	if (NULL == writer_data)
	{
		return 0;
	}

	writer_data->append(data, sizes);

	return sizes;
}
int HttpTask::libcurl_get(const char* url, std::string& buffer, std::string& errinfo)
{
	CURL *conn = NULL;
	CURLcode code;

	//curl_global_init(CURL_GLOBAL_DEFAULT);

	if (!init(conn, url, &buffer))
	{
		DEBUG_PL(LOG_ERROR," Connection initializion failed\n");
		errinfo = "Connection initializion failed\n";

		return -1;
	}

	code = curl_easy_perform(conn);

	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR,"Failed to get :%s, error:%s\n",url, error_buffer);
		errinfo.append("Failed to get ");
		errinfo.append(url);
		return -1;
	}

	curl_easy_cleanup(conn);

	return 0;
}

int HttpTask::libcurl_post(const char* url, const char* data, std::string& buffer, std::string& errinfo)
{
	CURL *conn = NULL;
	CURLcode code;
	if (!init(conn, url, &buffer))
	{
		DEBUG_PL(LOG_ERROR," Connection initializion failed\n");
		errinfo = "Connection initializion failed\n";

		return -1;
	}

	code = curl_easy_setopt(conn, CURLOPT_POST, true);

	if (code != CURLE_OK)
	{
		 DEBUG_PL(LOG_ERROR," Failed to set CURLOPT_POST:%s\n ",error_buffer); 
		return -1;
	}

	code = curl_easy_setopt(conn, CURLOPT_POSTFIELDS, data);
	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR," Failed to set CURLOPT_POSTFIELDS: %s \n",error_buffer);
		return -1;
	}

	code = curl_easy_perform(conn);

	if (code != CURLE_OK)
	{
		DEBUG_PL(LOG_ERROR,"Failed to posy :%s, error:%s\n",url, error_buffer);
		errinfo.append("Failed to post ");
		errinfo.append(url);

		return -2;
	}

	curl_easy_cleanup(conn);

	return 0;

}
