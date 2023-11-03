#pragma once
#ifndef CACHEPOOL_H_
#define CACHEPOOL_H_

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <mutex>
#include <condition_variable>
#include <string>
#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <spdlog/spdlog.h>

#define MIN_CACHE_CONN_CNT 2
#define MAX_CACHE_CONN_FAIL_NUM 10

using std::list;
using std::map;
using std::string;
using std::vector;

class CachePool;

class CacheConn
{
public:
	CacheConn(const char *server_ip, int server_port, int db_index, const char *password,
			  const char *pool_name = "");
	CacheConn(CachePool *pCachePool);
	virtual ~CacheConn();
	// ------------ 通用 ------------
	// 初始化(生成redis上下文)
	int Init();
	// 删除redis上下文
	void DeInit();
	const char *GetPoolName();
	// 通用操作
	// 判断一个key是否存在
	bool isExists(string &key);
	// 删除某个key
	long del(string &key);
	// 删除数据库所有信息
	bool flushdb();

	// ------------------- 字符串相关 -------------------
	string get(string key);
	string set(string key, string &value);
	string setex(string key, int timeout, string value);

	// string mset(string key, map);
	// 批量获取
	bool mget(const vector<string> &keys, map<string, string> &ret_value);
	// 原子加减1
	long incr(string key);
	long decr(string key);

	// ---------------- 哈希相关 ------------------------
	template <typename... Str>
	long hdel(string key, Str... fields)
	{
		if (Init())
		{
			return 0;
		}
		string commad = "HDEL %s";
		for (int i = 0; i < sizeof...(fields); i++)
		{
			commad += " %s";
		}
		redisReply *reply = (redisReply *)redisCommand(m_pContext, commad.c_str(), key.c_str(), fields...);
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

	string hget(string key, string field);
	bool hgetAll(string key, map<string, string> &ret_value);
	long hset(string key, string field, string value);

	long hincrBy(string key, string field, long value);
	long incrBy(string key, long value);
	string hmset(string key, map<string, string> &hash);
	bool hmget(string key, list<string> &fields, list<string> &ret_value);

	// ------------ 链表相关 ------------
	template <typename... Str>
	long lpush(string key, Str... values)
	{
		if (Init())
		{
			return -1;
		}
		string commad = "LPUSH %s";
		for (int i = 0; i < sizeof...(values); i++)
		{
			commad += " %s";
		}
		redisReply *reply = (redisReply *)redisCommand(m_pContext, commad.c_str(), key.c_str(), values...);
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
	template <typename... Str>
	long rpush(string key, Str... values)
	{
		if (Init())
		{
			return -1;
		}
		string commad = "RPUSH %s";
		for (int i = 0; i < sizeof...(values); i++)
		{
			commad += " %s";
		}
		redisReply *reply = (redisReply *)redisCommand(m_pContext, commad.c_str(), key.c_str(), values...);
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
	long llen(string key);
	bool lrange(string key, long start, long end, list<string> &ret_value);
	long lrem(string key,int count,string del_value);

	// ------------ 集合相关 ------------
	long sadd(string key, string member);
	bool smembers(vector<string>& ret_value,string key);
	long srem(string key, string member);
	template<typename ...Str>
	bool sinter(vector<string>& res,Str... keys)
	{
		if (Init())
		{
			return false;
		}
		string commad = "SINTER";
		for (int i = 0; i < sizeof...(keys); i++)
		{
			commad += " %s";
		}
		redisReply *reply = (redisReply *)redisCommand(m_pContext, commad.c_str(), keys...);
		if (!reply)
		{
			spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
			return false;
		}
		for(size_t i=0;i<reply->elements;i++)
		{
			redisReply *reply_value=reply->element[i];
			string value(reply_value->str,reply_value->len);
			res.push_back(value);
		}
		freeReplyObject(reply);
		return true;
	}
	// ------------ 有序集合相关(跳表) ------------
	long zadd(string key,map<string,float>& values);
	bool zrange(string key,uint32_t beg,uint32_t end,vector<string>& members);
	string zscore(string key,string member);
	template<typename...Str>
	long zrem(string key,Str...members)
	{
		long ret_value;
		if (Init())
		{
			return ret_value;
		}
		string commad = "ZREM %s";
		for(int i=0;i<sizeof...(members);i++)
		{
			commad+=" %s";
		}
		redisReply* reply=(redisReply*)redisCommand(m_pContext,commad.c_str(),key.c_str(),members...);
		if (!reply)
		{
			spdlog::error("File:{} Line:{} redisCommand failed:{}",__FILE__,__LINE__, m_pContext->errstr);
			redisFree(m_pContext);
			m_pContext = NULL;
			return ret_value;
		}
		ret_value=reply->integer;
		freeReplyObject(reply);
		return ret_value;
	}
	
	
private:
	CachePool *m_pCachePool;
	redisContext *m_pContext; // 每个redis连接 redisContext redis客户端编程的对象
	uint64_t m_last_connect_time;
	uint16_t m_server_port;
	string m_server_ip;
	string m_password;
	uint16_t m_db_index;
	string m_pool_name;
};

class CachePool
{
public:
	using Ptr = std::shared_ptr<CachePool>;
	// db_index和mysql不同的地方
	CachePool(const char *pool_name, const char *server_ip, int server_port, int db_index,
			  const char *password, int max_conn_cnt);
	virtual ~CachePool();

	int Init();
	// 获取空闲的连接资源
	CacheConn *GetCacheConn(const int timeout_ms = 0);
	// Pool回收连接资源
	void RelCacheConn(CacheConn *pCacheConn);

	const char *GetPoolName() { return m_pool_name.c_str(); }
	const char *GetServerIP() { return m_server_ip.c_str(); }
	const char *GetPassword() { return m_password.c_str(); }
	int GetServerPort() { return m_server_port; }
	int GetDBIndex() { return m_db_index; }

private:
	string m_pool_name;
	string m_server_ip;
	string m_password;
	int m_server_port;
	int m_db_index; // redis db index

	int m_cur_conn_cnt;
	int m_max_conn_cnt;
	list<CacheConn *> m_free_list;
	std::mutex m_mutex;
	std::condition_variable m_cond_var;
	bool m_abort_request = false;

	int wait_cout = 0; // 这里只是为测试了测试用
};

#endif /* CACHEPOOL_H_ */
