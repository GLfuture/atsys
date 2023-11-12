#include "api/time_api.h"

Time_API::Time_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}


int Time_API::Function(Context_Base::Ptr ctx)
{
    Time_Context::Ptr time_ctx = std::dynamic_pointer_cast<Time_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    CacheConn* cache_conn = cachepool->GetCacheConn();
    if(time_ctx->role == SIMPLE)
    {
        if (time_ctx->method == 1)
        {
            std::string tid = "time:";
            tid += std::to_string(time_ctx->uid);
            if (cache_conn->isExists(tid))
            {
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_REPEATEDLY_CLOCK;
            }
            auto now = std::chrono::system_clock::now();
            time_t expire_time = Get_Expire_Time(now);
            time_t now_time = std::chrono::system_clock::to_time_t(now);
            if (cache_conn->setex(tid, expire_time, std::to_string(now_time)).empty())
            {
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_METHOD_OP_FAIL;
            }
        }
        else if (time_ctx->method == 0)
        {
            std::string tid = "time:";
            tid += std::to_string(time_ctx->uid);
            std::string res = cache_conn->get(tid);
            if (res.empty())
            {
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_NOT_CLOCK_IN;
            }
            if (cache_conn->del(tid) == 0)
            {
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_METHOD_OP_FAIL;
            }
            time_t ago = atol(res.c_str());
            time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string value = "total_time+";
            value += std::to_string(now - ago);
            time_ctx->_time = now - ago;
            std::string condition = "uid=";
            condition += std::to_string(time_ctx->uid);
            std::string query = mysql_conn->Update_Query("time", {"total_time"}, {value}, condition);
            spdlog::info("time_api: {}", query);
            mysql_conn->Update(query);
            if (mysql_conn->Get_Errno() != 0)
            {
                auto now = std::chrono::system_clock::now();
                time_t expire_time = Get_Expire_Time(now);
                cache_conn->setex(tid, expire_time, res);
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_METHOD_OP_FAIL;
            }
        }
    }
    else if(time_ctx->role == MANAGER)
    {
        if(time_ctx->method == 0){
            std::string query = "update time set total_time=0;";
            spdlog::info("time_api: {}", query);
            mysql_conn->Update(query);
            if (mysql_conn->Get_Errno() != 0)
            {
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_METHOD_OP_FAIL;
            }
        }
        else if(time_ctx->role == 1)
        {
            std::string query = "update time set need_time=";
            query =query + std::to_string(time_ctx->settime) + ";";
            spdlog::info("time_api: {}", query);
            mysql_conn->Update(query);
            if(mysql_conn->Get_Errno() != 0){
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_METHOD_OP_FAIL;
            }
        }
    }
    mysqlpool->Ret_Conn(mysql_conn);
    cachepool->RelCacheConn(cache_conn);
    return STATUS_METHOD_OP_SUCCESS;
}


time_t Time_API::Get_Expire_Time(const std::chrono::_V2::system_clock::time_point now)
{
    auto tomorrow = now + std::chrono::hours(24);
    std::time_t tomorrow_time = std::chrono::system_clock::to_time_t(tomorrow);
    tm *tomorrow_tm = std::localtime(&tomorrow_time);
    tomorrow_tm->tm_hour = 0;
    tomorrow_tm->tm_min = 0;
    tomorrow_tm->tm_sec = 0;
    auto tomorrow_midnight = std::chrono::system_clock::from_time_t(std::mktime(tomorrow_tm));
    auto tomorrow_timestamp = std::chrono::duration_cast<std::chrono::seconds>(tomorrow_midnight.time_since_epoch()).count();
    time_t now_time = std::chrono::system_clock::to_time_t(now);
    return tomorrow_timestamp - now_time;
}