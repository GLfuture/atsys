#include "api/login_api.h"



Login_API::Login_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool)
{
}

std::string Login_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    std::string body(http->Request_Get_Body());
    std::string token(http->Request_Get_Key_Value("Token"));
    std::string ret_http_body;
    jwt::jwt_object obj;
    int code = 0;
    if (json::accept(body))
    {
        int role;
        json j = json::parse(body);
        if (j.contains("role") && j["role"].is_number())
            role = j["role"];
        if (role == SIMPLE)
        {
            if (!token.empty() && Token_Analyzer::Decode_Token(token, obj).value() != static_cast<int>(jwt::VerificationErrc::TokenExpired) && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0)
            {
                obj.remove_claim("exp");
                obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
                token = obj.signature();
                http->Response_Set_Key_Value("Token", token);
                code = STATUS_LOGIN_SUCCESS;
            }
            else
            {
                std::string username, password;
                if (j.contains("username") && j["username"].is_string()) username = j["username"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                if (j.contains("password") && j["password"].is_string()) password = j["password"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                Login_Context::Ptr ctx = std::make_shared<Login_Context>(http, role, username, password);
                code = Deal_Real_Event(ctx);
            }
        }
        else if (role == MANAGER)
        {
            if (!token.empty() && Token_Analyzer::Decode_Token(token, obj).value() != static_cast<int>(jwt::VerificationErrc::TokenExpired) && obj.payload().get_claim_value<std::string>("role") == "man")
            {
                obj.remove_claim("exp");
                obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
                token = obj.signature();
                http->Response_Set_Key_Value("Token", token);
                code = STATUS_LOGIN_SUCCESS;
            }
            else
            {
                std::string username, password;
                if (j.contains("username") && j["username"].is_string()) username = j["username"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                if (j.contains("password") && j["password"].is_string()) password = j["password"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                Login_Context::Ptr ctx = std::make_shared<Login_Context>(http, role, username, password);
                code = Deal_Real_Event(ctx);
            }
        }
    }
    else{
        code = STATUS_JSON_ERROR;
    }
    // 生成json
end:
    json j;
    j["code"] = code;
    return j.dump();
}


int Login_API::Deal_Real_Event(Context_Base::Ptr ctx_ptr)
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