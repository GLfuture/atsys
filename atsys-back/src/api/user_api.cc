#include "api/user_api.h"

User_API::User_API(MySqlPool::Ptr mysqlpool, CachePool::Ptr cachepool) : API_Base(mysqlpool, cachepool) {}

std::string User_API::Function(HTTP_NSP::HTTP::Ptr http)
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
            if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
            {
                int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                std::string body(http->Request_Get_Body());
                if (json::accept(body))
                {
                    json j = json::parse(body);
                    std::string username, password, email, phone, address;
                    int sex = -1;
                    if (j.contains("username") && j["username"].is_string())
                        username = j["username"];
                    if (j.contains("password") && j["password"].is_string())
                        password = j["password"];
                    if (j.contains("email") && j["email"].is_string())
                        email = j["email"];
                    if (j.contains("phone") && j["phone"].is_string())
                        phone = j["phone"];
                    if (j.contains("address") && j["address"].is_string())
                        address = j["address"];
                    if (j.contains("sex") && j["sex"].is_number())
                        sex = j["sex"];
                    User_Context::Ptr user_ctx = std::make_shared<User_Context>(uid, username, password, email, phone, address, sex);
                    code = Deal_Real_Event(user_ctx);
                }
                else
                {
                    code = STATUS_JSON_ERROR;
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
    json j;
    j["code"] = code;
    return j.dump();
}

int User_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    User_Context::Ptr user_ctx = std::dynamic_pointer_cast<User_Context>(ctx);
    MySqlConn *mysql_conn = mysqlpool->Get_Conn(0);
    CacheConn *cache_conn = cachepool->GetCacheConn();
    std::vector<std::string> fields, values;
    std::map<std::string, std::string> hash;
    if (!user_ctx->username.empty())
    {
        fields.push_back("username");
        values.push_back(Generate_String_Col(user_ctx->username));
        hash["username"] = user_ctx->username;
    }
    if (!user_ctx->address.empty())
    {
        fields.push_back("address");
        values.push_back(Generate_String_Col(user_ctx->address));
        hash["address"] = user_ctx->address;
    }
    if (!user_ctx->email.empty())
    {
        fields.push_back("email");
        values.push_back(Generate_String_Col(user_ctx->email));
        hash["email"] = user_ctx->email;
    }
    if (!user_ctx->password.empty())
    {
        fields.push_back("password");
        values.push_back(Generate_String_Col(user_ctx->password));
        hash["password"] = user_ctx->password;
    }
    if (!user_ctx->phone.empty())
    {
        fields.push_back("phone");
        values.push_back(Generate_String_Col(user_ctx->phone));
        hash["phone"] = user_ctx->phone;
    }
    if (user_ctx->sex != -1)
    {
        fields.push_back("sex");
        values.push_back(std::to_string(user_ctx->sex));
        hash["sex"] = std::to_string(user_ctx->sex);
    }
    std::string condition = " uid = ";
    condition = condition + std::to_string(user_ctx->uid);
    std::string query = mysql_conn->Update_Query("user", std::move(fields), std::move(values), condition);
    spdlog::info("user_upd_api: {}", query);
    mysql_conn->Update(query);
    if (mysql_conn->Get_Errno() != 0 || cache_conn->hmset(std::to_string(user_ctx->uid), hash).empty())
    {
        mysqlpool->Ret_Conn(mysql_conn);
        cachepool->RelCacheConn(cache_conn);
        return STATUS_METHOD_OP_FAIL;
    }
    mysqlpool->Ret_Conn(mysql_conn);
    cachepool->RelCacheConn(cache_conn);
    return STATUS_METHOD_OP_SUCCESS;
}

std::string User_API::Generate_String_Col(std::string &data)
{
    std::string res = "'";
    res += data;
    return res + "'";
}