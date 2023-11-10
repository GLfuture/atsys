#include "api/register_api.h"


Register_API::Register_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool)
{

}

int Register_API::Function(Context_Base::Ptr ctx_ptr)
{
    Register_Context::Ptr register_ctx = std::dynamic_pointer_cast<Register_Context>(ctx_ptr);
    
    MySqlConn *mysql_conn = mysqlpool->Get_Conn(0);
    
    std::string condition = "username = '";
    condition = condition + register_ctx->username + "'";
    std::string query = mysql_conn->Select_Query(mysql_conn->Arg_List("uid"), "user", condition);
    std::vector<std::vector<std::string>> res;
    spdlog::info("register: {}",query);
    mysql_conn->Select(query, res);
    if (res.size() > 1)
    {
        mysqlpool->Ret_Conn(mysql_conn);
        spdlog::info("has repeated username:{}", register_ctx->username);
        return STATUS_REGISTER_REPEAT_USER;
    }
    std::vector<std::string> info{register_ctx->username, register_ctx->password, register_ctx->email, register_ctx->phone, register_ctx->address,std::to_string(register_ctx->sex)};
    for (int i = 0; i < 5; i++)
    {
        std::string sub_info = "'";
        sub_info = sub_info + info[i] + "'";
        info[i] = sub_info;
    }
    query = mysql_conn->Insert_Query("user", {"username", "password", "email", "phone", "address","sex"}, std::move(info));
    spdlog::info("register: {}",query);
    if (mysql_conn->Insert(query) != 0)
    {
        mysqlpool->Ret_Conn(mysql_conn);
        return STATUS_REGISTER_FAILED;
    }
    query = mysql_conn->Select_Query(mysql_conn->Arg_List("uid"), "user", condition);
    res.clear();
    spdlog::info("register: {}",query);
    mysql_conn->Select(query, res);
    mysqlpool->Ret_Conn(mysql_conn);
    CacheConn *cache_conn = cachepool->GetCacheConn();
    std::map<std::string, std::string> hash;
    hash["username"] = register_ctx->username;
    hash["email"] = register_ctx->email;
    hash["phone"] = register_ctx->phone;
    hash["address"] = register_ctx->address;
    hash["sex"]= std::to_string(register_ctx->sex);
    cache_conn->hmset(res[1][0], hash);
    cachepool->RelCacheConn(cache_conn);
    return STATUS_REGISTER_SUCCESS;
}