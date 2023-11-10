#include "api/login_api.h"



Login_API::Login_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool)
{
}



int Login_API::Function(Context_Base::Ptr ctx_ptr)
{
    Login_Context::Ptr login_ctx = std::dynamic_pointer_cast<Login_Context>(ctx_ptr);
    
    MySqlConn *mysql_conn = mysqlpool->Get_Conn(0);
    std::string condition = "username = '";
    condition = condition + login_ctx->username + "'";
    std::string token;
    if(login_ctx->role == SIMPLE)
    {
        std::string query = mysql_conn->Select_Query(mysql_conn->Arg_List("uid","password"), "user", condition);
        std::vector<std::vector<std::string>> res;
        mysql_conn->Select(query, res);
        spdlog::info("login: {}",query);
        mysqlpool->Ret_Conn(mysql_conn);
        if (res.size() == 1)
            return STATUS_LOGIN_NO_USER_ERROR;
        if (res[1][1].compare(login_ctx->password) != 0)
            return STATUS_LOGIN_PASSWORD_ERROR;
        token = Generate_Token({{"uid",res[1][0]},{"role","sim"}});
    }
    else if(login_ctx->role == MANAGER)
    {
        std::string query = mysql_conn->Select_Query(mysql_conn->Arg_List("mid","password"), "manager", condition);
        std::vector<std::vector<std::string>> res;
        mysql_conn->Select(query, res);
        spdlog::info("login: {}",query);
        mysqlpool->Ret_Conn(mysql_conn);
        if (res.size() == 1)
            return STATUS_LOGIN_NO_USER_ERROR;
        if (res[1][1].compare(login_ctx->password) != 0)
            return STATUS_LOGIN_PASSWORD_ERROR;
        token = Generate_Token({{"mid",res[1][0]},{"role","man"}});
    }
    login_ctx->http->Response_Set_Key_Value("Token", token);
    return STATUS_LOGIN_SUCCESS;
}



std::string Login_API::Generate_Token(const param_init_list_t& params)
{
    return Token_Analyzer::Encode_Token(TOKEN_ISS,std::chrono::system_clock::now()+std::chrono::seconds(TOKEN_EXPIRE_TIME),params);
}