#include "base/cachepool/CachePool.h"

CacheConn::CacheConn(const char *server_ip, int server_port, int db_index, const char *password,
					 const char *pool_name)
{
	m_server_ip = server_ip;
	m_server_port = server_port;

	m_db_index = db_index;
	m_password = password;
	m_pool_name = pool_name;
	m_pContext = NULL;
	m_last_connect_time = 0;
}

CacheConn::CacheConn(CachePool *pCachePool)
{
	m_pCachePool = pCachePool;
	if (pCachePool)
	{
		m_server_ip = pCachePool->GetServerIP();
		m_server_port = pCachePool->GetServerPort();
		m_db_index = pCachePool->GetDBIndex();
		m_password = pCachePool->GetPassword();
		m_pool_name = pCachePool->GetPoolName();
	}
	else
	{
		spdlog::error("File:{} Line:{} pCachePool is NULL",__FILE__,__LINE__);
	}

	m_pContext = NULL;
	m_last_connect_time = 0;
}

CacheConn::~CacheConn()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

/*
 * redis初始化连接和重连操作，类似mysql_ping()
 */
int CacheConn::Init()
{
	if (m_pContext) // 非空，连接是正常的
	{
		return 0;
	}

	// 1s 尝试重连一次
	uint64_t cur_time = (uint64_t)time(NULL);
	if (cur_time < m_last_connect_time + 1) // 重连尝试 间隔1秒
	{
		spdlog::info("File:{} Line:{} cur_time:{}, m_last_connect_time:{}",__FILE__,__LINE__, cur_time, m_last_connect_time);
		return 1;
	}
	// printf("m_last_connect_time = cur_time\n");
	m_last_connect_time = cur_time;

	// 1000ms超时
	struct timeval timeout = {0, 1000000};
	// 建立连接后使用 redisContext 来保存连接状态。
	// redisContext 在每次操作后会修改其中的 err 和  errstr 字段来表示发生的错误码（大于0）和对应的描述。
	m_pContext = redisConnectWithTimeout(m_server_ip.c_str(), m_server_port, timeout);

	if (!m_pContext || m_pContext->err)
	{
		if (m_pContext)
		{
			spdlog::error("File:{} Line:{} redisConnect failed: {}",__FILE__,__LINE__, m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
		}
		else
		{
			spdlog::error("File:{} Line:{} redisConnect failed",__FILE__,__LINE__);
		}

		return 1;
	}

	redisReply *reply;
	// 验证
	if (!m_password.empty())
	{
		reply = (redisReply *)redisCommand(m_pContext, "AUTH %s", m_password.c_str());

		if (!reply || reply->type == REDIS_REPLY_ERROR)
		{
			spdlog::error("File:{} Line:{} Authentication failure",__FILE__,__LINE__);
			if (reply)
				freeReplyObject(reply);
			return -1;
		}
		else
		{
			spdlog::info("Authentication success");
		}

		freeReplyObject(reply);
	}

	reply = (redisReply *)redisCommand(m_pContext, "SELECT %d", this->m_db_index);

	if (reply && (reply->type == REDIS_REPLY_STATUS) && (strncmp(reply->str, "OK", 2) == 0))
	{
		freeReplyObject(reply);
		return 0;
	}
	else
	{
		if (reply)
			spdlog::error("File:{} Line:{} select cache db failed:{}",__FILE__,__LINE__, reply->str);
		return 2;
	}
}

void CacheConn::DeInit()
{
	if (m_pContext)
	{
		redisFree(m_pContext);
		m_pContext = NULL;
	}
}

const char *CacheConn::GetPoolName()
{
	return m_pool_name.c_str();
}

string CacheConn::get(string key)
{
	string value;

	if (Init())
	{
		return value;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "GET %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return value;
	}

	if (reply->type == REDIS_REPLY_STRING)
	{
		value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return value;
}
// 成功返回ok
string CacheConn::set(string key, string &value)
{
	string ret_value;

	if (Init())
	{
		return ret_value;
	}
	// 返回的结果存放在redisReply
	redisReply *reply = (redisReply *)redisCommand(m_pContext, "SET %s %s", key.c_str(), value.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply); // 释放资源
	return ret_value;
}

string CacheConn::setex(string key, int64_t timeout, string value)
{
	string ret_value;

	if (Init())
	{
		return ret_value;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "SETEX %s %ld %s", key.c_str(), timeout, value.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::mget(const vector<string> &keys, map<string, string> &ret_value)
{
	if (Init())
	{
		return false;
	}
	if (keys.empty())
	{
		return false;
	}

	string strKey;
	bool bFirst = true;
	for (vector<string>::const_iterator it = keys.begin(); it != keys.end(); ++it)
	{
		if (bFirst)
		{
			bFirst = false;
			strKey = *it;
		}
		else
		{
			strKey += " " + *it;
		}
	}

	if (strKey.empty())
	{
		return false;
	}
	strKey = "MGET " + strKey;
	redisReply *reply = (redisReply *)redisCommand(m_pContext, strKey.c_str());
	if (!reply)
	{
		spdlog::info("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}
	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t i = 0; i < reply->elements; ++i)
		{
			redisReply *child_reply = reply->element[i];
			if (child_reply->type == REDIS_REPLY_STRING)
			{
				ret_value[keys[i]] = child_reply->str;
			}
		}
	}
	freeReplyObject(reply);
	return true;
}

bool CacheConn::isExists(string &key)
{
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "EXISTS %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	if (0 == ret_value)
	{
		return false;
	}
	else
	{
		return true;
	}
}

long CacheConn::del(string &key)
{
	if (Init())
	{
		return 0;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "DEL %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return 0;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::hget(string key, string field)
{
	string ret_value;
	if (Init())
	{
		return ret_value;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HGET %s %s", key.c_str(), field.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	if (reply->type == REDIS_REPLY_STRING)
	{
		ret_value.append(reply->str, reply->len);
	}

	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hgetAll(string key, map<string, string> &ret_value)
{
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HGETALL %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if ((reply->type == REDIS_REPLY_ARRAY) && (reply->elements % 2 == 0))
	{
		for (size_t i = 0; i < reply->elements; i += 2)
		{
			redisReply *field_reply = reply->element[i];
			redisReply *value_reply = reply->element[i + 1];

			string field(field_reply->str, field_reply->len);
			string value(value_reply->str, value_reply->len);
			ret_value.insert(make_pair(field, value));
		}
	}

	freeReplyObject(reply);
	return true;
}

long CacheConn::hset(string key, string field, string value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::hincrBy(string key, string field, long value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "HINCRBY %s %s %ld", key.c_str(), field.c_str(), value);
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::incrBy(string key, long value)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "INCRBY %s %ld", key.c_str(), value);
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redis Command failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

string CacheConn::hmset(string key, map<string, string> &hash)
{
	string ret_value;

	if (Init())
	{
		return ret_value;
	}

	int argc = hash.size() * 2 + 2;
	const char **argv = new const char *[argc];
	if (!argv)
	{
		return ret_value;
	}

	argv[0] = "HMSET";
	argv[1] = key.c_str();
	int i = 2;
	for (map<string, string>::iterator it = hash.begin(); it != hash.end(); it++)
	{
		argv[i++] = it->first.c_str();
		argv[i++] = it->second.c_str();
	}

	redisReply *reply = (redisReply *)redisCommandArgv(m_pContext, argc, argv, NULL);
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		delete[] argv;

		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}

	ret_value.append(reply->str, reply->len);

	delete[] argv;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::hmget(string key, list<string> &fields, list<string> &ret_value)
{
	if (Init())
	{
		return false;
	}

	int argc = fields.size() + 2;
	const char **argv = new const char *[argc];
	if (!argv)
	{
		return false;
	}

	argv[0] = "HMGET";
	argv[1] = key.c_str();
	int i = 2;
	for (list<string>::iterator it = fields.begin(); it != fields.end(); it++)
	{
		argv[i++] = it->c_str();
	}

	redisReply *reply = (redisReply *)redisCommandArgv(m_pContext, argc, (const char **)argv, NULL);
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		delete[] argv;

		redisFree(m_pContext);
		m_pContext = NULL;

		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t i = 0; i < reply->elements; i++)
		{
			redisReply *value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	delete[] argv;
	freeReplyObject(reply);
	return true;
}

long CacheConn::incr(string key)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "INCR %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redis Command failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::decr(string key)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "DECR %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redis Command failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::llen(string key)
{
	if (Init())
	{
		return -1;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "LLEN %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}

	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::lrange(string key, long start, long end, list<string> &ret_value)
{
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "LRANGE %s %d %d", key.c_str(), start, end);
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_ARRAY)
	{
		for (size_t i = 0; i < reply->elements; i++)
		{
			redisReply *value_reply = reply->element[i];
			string value(value_reply->str, value_reply->len);
			ret_value.push_back(value);
		}
	}

	freeReplyObject(reply);
	return true;
}

long CacheConn::lrem(string key, int count, string del_value)
{
	if (Init())
	{
		return -1;
	}
	redisReply *reply = (redisReply *)redisCommand(m_pContext, "LREM %s %d %s", key.c_str(), count, del_value.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::sadd(string key, string member)
{
	if (Init())
	{
		return -1;
	}
	redisReply *reply = (redisReply *)redisCommand(m_pContext, "SADD %s %s", key.c_str(), member.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::smembers(vector<string>& ret_value, string key)
{
	if (Init())
	{
		return false;
	}
	redisReply *reply = (redisReply *)redisCommand(m_pContext, "SMEMBERS %s", key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}
	for (size_t i = 0; i < reply->elements; i++)
	{
		redisReply *reply_value = reply->element[i];
		string value(reply_value->str, reply_value->len);
		ret_value.push_back(value);
	}
	freeReplyObject(reply);
	return true;
}

long CacheConn::srem(string key, string member)
{
	if (Init())
	{
		return -1;
	}
	redisReply *reply = (redisReply *)redisCommand(m_pContext, "SREM %s %s", key.c_str(), member.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

long CacheConn::zadd(string key, map<string, float> &values)
{
	if (Init())
	{
		return -1;
	}
	string command = "ZADD %s";
	for (map<string, float>::iterator it = values.begin(); it != values.end(); it++)
	{
		command += " ";
		command += std::to_string((*it).second);
		command += " ";
		command += (*it).first;
	}
	redisReply *reply = (redisReply *)redisCommand(m_pContext, command.c_str(), key.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return -1;
	}
	long ret_value = reply->integer;
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::zrange(string key, uint32_t beg, uint32_t end, vector<string> &members)
{
	if (Init())
	{
		return false;
	}
	redisReply *reply=(redisReply*)redisCommand(m_pContext,"ZRANGE %s %d %d",key.c_str(),beg,end);
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}
	for(int i=0;i<reply->elements;i++)
	{
		redisReply* reply_value=reply->element[i];
		string value(reply_value->str,reply_value->len);
		members.push_back(value);
	}
	freeReplyObject(reply);
    return true;
}

string CacheConn::zscore(string key, string member)
{
	string ret_value;
	if (Init())
	{
		return ret_value;
	}
	redisReply *reply = (redisReply *)redisCommand(m_pContext, "ZSCORE %s %s", key.c_str(), member.c_str());
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return ret_value;
	}
	if(reply->type == REDIS_REPLY_NIL){
		return "";
	}else{
		ret_value=reply->str;
	}
	freeReplyObject(reply);
	return ret_value;
}

bool CacheConn::flushdb()
{
	bool ret = false;
	if (Init())
	{
		return false;
	}

	redisReply *reply = (redisReply *)redisCommand(m_pContext, "FLUSHDB");
	if (!reply)
	{
		spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
		redisFree(m_pContext);
		m_pContext = NULL;
		return false;
	}

	if (reply->type == REDIS_REPLY_STRING && strncmp(reply->str, "OK", 2) == 0)
	{
		ret = true;
	}

	freeReplyObject(reply);

	return ret;
}
///////////////
CachePool::CachePool(const char *pool_name, const char *server_ip, int server_port, int db_index,
					 const char *password, int max_conn_cnt)
{
	m_pool_name = pool_name;
	m_server_ip = server_ip;
	m_server_port = server_port;
	m_db_index = db_index;
	m_password = password;
	m_max_conn_cnt = max_conn_cnt;
	m_cur_conn_cnt = MIN_CACHE_CONN_CNT;
}

CachePool::~CachePool()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_abort_request = true;
	m_cond_var.notify_all(); // 通知所有在等待的
	for (list<CacheConn *>::iterator it = m_free_list.begin(); it != m_free_list.end(); it++)
	{
		CacheConn *pConn = *it;
		delete pConn;
	}

	m_free_list.clear();
	m_cur_conn_cnt = 0;
}

int CachePool::Init()
{
	for (int i = 0; i < m_cur_conn_cnt; i++)
	{
		CacheConn *pConn = new CacheConn(m_server_ip.c_str(), m_server_port,
										 m_db_index, m_password.c_str(), m_pool_name.c_str());
		if (pConn->Init())
		{
			delete pConn;
			return 1;
		}

		m_free_list.push_back(pConn);
	}

	// spdlog::info("cache pool: %s, list size: %lu\n", m_pool_name.c_str(), m_free_list.size());
	return 0;
}

CacheConn *CachePool::GetCacheConn(const int timeout_ms)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_abort_request)
	{
		spdlog::warn("File:{} Line:{} have aboort",__FILE__,__LINE__);
		return NULL;
	}

	if (m_free_list.empty())
	{
		// 第一步先检测 当前连接数量是否达到最大的连接数量
		if (m_cur_conn_cnt >= m_max_conn_cnt)
		{
			// 如果已经到达了，看看是否需要超时等待
			if (timeout_ms <= 0) // 死等，直到有连接可以用 或者 连接池要退出
			{
				spdlog::info("File:{} Line:{} wait ms:{}",__FILE__,__LINE__, timeout_ms);
				m_cond_var.wait(lock, [this]
								{
					spdlog::info("File:{} Line:{} wait:{}, size:{}",__FILE__,__LINE__, wait_cout++, m_free_list.size());
					// 当前连接数量小于最大连接数量 或者请求释放连接池时退出
					return (!m_free_list.empty()) | m_abort_request; });
			}
			else
			{
				// return如果返回 false，继续wait(或者超时),  如果返回true退出wait
				// 1.m_free_list不为空
				// 2.超时退出
				// 3. m_abort_request被置为true，要释放整个连接池
				m_cond_var.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]
									{
					spdlog::info("File:{} Line:{} wait_for:{}, size:{}",__FILE__,__LINE__, wait_cout++, m_free_list.size());
					return (!m_free_list.empty()) | m_abort_request; });
				// 带超时功能时还要判断是否为空
				if (m_free_list.empty()) // 如果连接池还是没有空闲则退出
				{
					return NULL;
				}
			}

			if (m_abort_request)
			{
				spdlog::warn("File:{} Line:{} have aboort",__FILE__,__LINE__);
				return NULL;
			}
		}
		else
		{
			CacheConn *p_cache_conn = new CacheConn(m_server_ip.c_str(), m_server_port,
													m_db_index, m_password.c_str(), m_pool_name.c_str());
			int ret = p_cache_conn->Init();
			if (ret)
			{
				spdlog::error("File:{} Line:{} Init CacheConn failed",__FILE__,__LINE__);
				delete p_cache_conn;
				return NULL;
			}
			else
			{
				m_free_list.push_back(p_cache_conn);
				m_cur_conn_cnt++;
				// spdlog::info("new cache connection: %s, conn_cnt: %d\n", m_pool_name.c_str(), m_cur_conn_cnt);
			}
		}
	}

	CacheConn *pConn = m_free_list.front();
	m_free_list.pop_front();

	return pConn;
}

void CachePool::RelCacheConn(CacheConn *p_cache_conn)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	list<CacheConn *>::iterator it = m_free_list.begin();
	for (; it != m_free_list.end(); it++)
	{
		if (*it == p_cache_conn)
		{
			break;
		}
	}

	if (it == m_free_list.end())
	{
		m_free_list.push_back(p_cache_conn);
		m_cond_var.notify_one(); // 通知取队列
	}
	else
	{
		spdlog::error("File:{} Line:{} File:{} Line:{} RelDBConn failed",__FILE__,__LINE__); // 不再次回收连接
	}
}
