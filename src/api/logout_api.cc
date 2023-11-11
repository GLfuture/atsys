#include "api/logout_api.h"

Logout_API::Logout_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

int Logout_API::Function(Context_Base::Ptr ctx)
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