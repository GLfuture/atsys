#include "api/time_api.h"

Time_API::Time_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

std::string Time_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    int code = 0;
    json ret_j;
    std::string token(http->Request_Get_Key_Value("Token"));
    if (token.empty())
    {
        code = STATUS_TOKEN_NOT_IN_HEAD;
        goto end;
    }
    else
    {
        jwt::jwt_object obj;
        if (Token_Analyzer::Decode_Token(token, obj).value() == static_cast<int>(jwt::VerificationErrc::TokenExpired))
        {
            code = STATUS_TOKEN_EXPIRED;
            goto end;
        }
        else
        {
            int method;
            time_t time = 0;
            if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
            {
                int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                std::string body(http->Request_Get_Body());
                if (json::accept(body))
                {
                    json j = json::parse(body);
                    if (j.contains("method") && j["method"].is_number())
                        method = j["method"];
                    else
                    {
                        code = STATUS_JSON_NO_NEC_MEM;
                        goto end;
                    }
                    if(method == 2){
                        if (j.contains("time") && j["time"].is_number())
                            time = j["time"];
                        else
                        {
                            code = STATUS_JSON_NO_NEC_MEM;
                            goto end;
                        }
                        if(j.contains("uid") && j["uid"].is_number()){
                            uid = j ["uid"];
                        }
                    }
                    Time_Context::Ptr time_ctx = std::make_shared<Time_Context>(SIMPLE, uid, method, time);
                    code = Deal_Real_Event(time_ctx);
                    if (time_ctx->_time != 0)
                    {
                        ret_j["time"] = time_ctx->_time;
                    }
                }
                else
                {
                    code = STATUS_JSON_ERROR;
                }
            }
            else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
            {
                std::string body(http->Request_Get_Body());
                if (json::accept(body))
                {
                    json j = json::parse(body);
                    if (j.contains("method") && j["method"].is_number())
                        method = j["method"];
                    else
                    {
                        code = STATUS_JSON_NO_NEC_MEM;
                        goto end;
                    }
                    if(method == 1){
                        if (j.contains("time") && j["time"].is_number())
                            time = j["time"];
                        else
                        {
                            code = STATUS_JSON_NO_NEC_MEM;
                            goto end;
                        }
                    }
                    Time_Context::Ptr time_ctx = std::make_shared<Time_Context>(MANAGER, 0, method, time);
                    code = Deal_Real_Event(time_ctx);
                }
            }
            else
            {
                code = STATUS_PRIVILIDGE_ERROR;
            }
        }
        obj.remove_claim("exp");
        obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
        token = obj.signature();
        http->Response_Set_Key_Value("Token", token);
    }
end:  
    ret_j["code"] = code;
    return ret_j.dump();
}

int Time_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    Time_Context::Ptr time_ctx = std::dynamic_pointer_cast<Time_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    CacheConn* cache_conn = cachepool->GetCacheConn();
    if(time_ctx->role == SIMPLE)
    {
        if (time_ctx->method == 0)
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
        else if (time_ctx->method == 1)
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
        else if(time_ctx->method == 2)
        {
            std::string value = "need_time + ";
            value = value + std::to_string(time_ctx->settime);
            std::string condition = "uid="+std::to_string(time_ctx->uid);
            std::string query = mysql_conn->Update_Query("time",{"need_time"},{value},condition);
            spdlog::info("time_api: {}",query);
            mysql_conn->Update(query);
            if(mysql_conn->Get_Errno()!=0){
                mysqlpool->Ret_Conn(mysql_conn);
                cachepool->RelCacheConn(cache_conn);
                return STATUS_METHOD_OP_FAIL;
            }
        }else{
            mysqlpool->Ret_Conn(mysql_conn);
            cachepool->RelCacheConn(cache_conn);
            return STATUS_METHOD_ERROR;
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
        else if(time_ctx->method == 1)
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
        }else{
            mysqlpool->Ret_Conn(mysql_conn);
            cachepool->RelCacheConn(cache_conn);
            return STATUS_METHOD_ERROR;
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