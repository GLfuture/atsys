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
        int role,method;
        std::string username, password,email,captcha;
        json j = json::parse(body);
        if (j.contains("role") && j["role"].is_number() && j.contains("method")&&j["method"].is_number())
        {
            role = j["role"];
            method = j["method"];
        }else{
            code = STATUS_JSON_NO_NEC_MEM;
        }
        
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
                if(method == 0){
                    //用户名密码登录
                    if (j.contains("username") && j["username"].is_string()&& j.contains("password") && j["password"].is_string())
                    {
                        username = j["username"];
                        password = j["password"];
                    }
                    else
                    {
                        code = STATUS_JSON_NO_NEC_MEM;
                        goto end;
                    }
                }
                else if(method == 1)
                {
                    //邮箱登录
                    if(j.contains("email")&& j["email"].is_string()&&j.contains("captcha")&&j["captcha"].is_string())
                    {
                        email = j["email"];
                        captcha = j["captcha"];
                    }
                }
                else
                {
                    code = STATUS_METHOD_ERROR;
                    goto end;
                }
                Login_Context::Ptr ctx = std::make_shared<Login_Context>(http,method, role, username, password,email,captcha);
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
                if(method == 0)
                {
                    if (j.contains("username") && j["username"].is_string()&& j.contains("password") && j["password"].is_string())
                    {
                        password = j["password"];
                        username = j["username"];
                    }
                    else
                    {
                        code = STATUS_JSON_NO_NEC_MEM;
                        goto end;
                    }
                }else{
                    code = STATUS_METHOD_ERROR;
                    goto end;
                }
                Login_Context::Ptr ctx = std::make_shared<Login_Context>(http,method, role, username, password,email,captcha);
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
        switch (login_ctx->method)
        {
        case 0:
            {
                
                std::string query = mysql_conn->Select_Query(mysql_conn->Arg_List("uid", "password"), "user", condition);
                std::vector<std::vector<std::string>> res;
                mysql_conn->Select(query, res);
                spdlog::info("login: {}", query);
                mysqlpool->Ret_Conn(mysql_conn);
                if (res.size() == 1)
                    return STATUS_LOGIN_NO_USER_ERROR;
                if (res[1][1].compare(login_ctx->password) != 0)
                    return STATUS_LOGIN_PASSWORD_ERROR;
                token = Generate_Token({{"uid", res[1][0]}, {"role", "sim"}});
            }
            break;
        case 1:
            {
                CacheConn* cache_conn = cachepool->GetCacheConn();
                std::string captcha = cache_conn->get(login_ctx->email);
                if(captcha.empty())
                {
                    cachepool->RelCacheConn(cache_conn);
                    return STATUS_LOGIN_CAPTCHA_ERROR;
                }
                if(captcha.compare(login_ctx->captcha) != 0){
                    cachepool->RelCacheConn(cache_conn);
                    return STATUS_LOGIN_CAPTCHA_ERROR;
                }   
                cachepool->RelCacheConn(cache_conn);
                std::string query = "select uid where email = '";
                query = query + login_ctx->email + "';";
                std::vector<std::vector<std::string>> res;
                mysql_conn->Select(query,res);
                if(mysql_conn->Get_Errno() != 0){
                    mysqlpool->Ret_Conn(mysql_conn);
                    return STATUS_UNKOWN_ERROR;
                }
                mysqlpool->Ret_Conn(mysql_conn);
                if(res.size()==1){
                    return STATUS_LOGIN_NO_USER_ERROR;
                }
                token = Generate_Token({{"uid", res[1][0]}, {"role", "sim"}});  
            }
            break;
        default:
            break;
        }
            
        
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