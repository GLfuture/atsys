#include "api/logout_api.h"

Logout_API::Logout_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

std::string Logout_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    int code = 0;
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
            if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
            {
                int mid = atoi(obj.payload().get_claim_value<std::string>("mid").c_str());
                Logout_Context::Ptr logout_ctx = std::make_shared<Logout_Context>(MANAGER, mid);
                code = Deal_Real_Event(logout_ctx);
            }
            else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
            {
                int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                Logout_Context::Ptr logout_ctx = std::make_shared<Logout_Context>(SIMPLE, uid);
                code = Deal_Real_Event(logout_ctx);
            }
        }
        obj.remove_claim("exp");
        obj.add_claim("exp", std::chrono::system_clock::now() - std::chrono::seconds(TOKEN_EXPIRE_TIME));
        token = obj.signature();
        http->Response_Set_Key_Value("Token", token);
    }
end:
    json j;
    j["code"] = code;
    return j.dump();
}

int Logout_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    Logout_Context::Ptr logout_ctx = std::dynamic_pointer_cast<Logout_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    if(logout_ctx->role == MANAGER){
        std::string condition = "mid = ";
        condition = condition + std::to_string(logout_ctx->id);
        std::string query = mysql_conn->Delete_Query("manager",condition);
        mysql_conn->Delete(query);
        if(mysql_conn->Get_Errno()!=0){
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_LOGOUT_FAILED;
        }
    }
    else if(logout_ctx->role == SIMPLE){
        CacheConn* cache_conn = cachepool->GetCacheConn();
        std::string key = std::to_string(logout_ctx->id);
        if(cache_conn->del(key) == 0){
            mysqlpool->Ret_Conn(mysql_conn);
            cachepool->RelCacheConn(cache_conn);
            return STATUS_LOGOUT_FAILED;
        }
        std::string condition = "uid = ";
        condition = condition + std::to_string(logout_ctx->id);
        std::string query = mysql_conn->Delete_Query("user",condition);
        spdlog::info("logout_api: {}",query);
        mysql_conn->Delete(query);
        if(mysql_conn->Get_Errno()!=0){
            mysqlpool->Ret_Conn(mysql_conn);
            cachepool->RelCacheConn(cache_conn);
            return STATUS_LOGOUT_FAILED;
        }
    }
    mysqlpool->Ret_Conn(mysql_conn);
    return STATUS_LOGOUT_SUCCESS;
}