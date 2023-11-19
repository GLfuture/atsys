#include "api/smtp_api.h"

Smtp_API::Smtp_API(MySqlPool::Ptr mysqlpool, CachePool::Ptr cachepool)
    :API_Base(mysqlpool,cachepool)
{
    std::random_device rd;
    this->gen = std::mt19937(rd());
    this->dis = std::uniform_int_distribution<int>(100000,999999);
}

std::string Smtp_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    int code = 0;
    json ret_j;
    std::string body(http->Request_Get_Body());
    if (json::accept(body))
    {
        json j = json::parse(body);
        std::vector<std::string> emails;
        int method;
        std::string content = "",subject="",content_type="",charset="";
        if (j.contains("method") && j["method"].is_number())
        {
            method = j["method"];
            if (method == 0)
            {
                if (j.contains("email") && j["email"].is_string())
                {
                    emails.push_back(j["email"]);
                }
            }
            else if (method == 1)
            {
                std::string token(http->Request_Get_Key_Value("Token"));
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
                    if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
                    {
                        if (j.contains("emails") && j["emails"].is_array() && j.contains("content") && j["content"].is_string() &&
                            j.contains("subject")&&j["subject"].is_string()&&j.contains("content_type")&&j["content_type"].is_string()
                            &&j.contains("charset")&&j["charset"].is_string())
                        {
                            for (int i = 0; i < j["emails"].size(); i++)
                            {
                                if (j["emails"][i].is_string())
                                    emails.push_back(j["emails"][i]);
                                else
                                {
                                    code = STATUS_JSON_ERROR;
                                    goto end;
                                }
                            }
                            subject = j["subject"];
                            content_type = j["content_type"];
                            charset = j["charset"];
                            content = j["content"];
                        }
                        else
                        {
                            code = STATUS_JSON_NO_NEC_MEM;
                            goto end;
                        }
                    }else{
                        code = STATUS_PRIVILIDGE_ERROR;
                        goto end;
                    }
                }
                
            }
            else{
                code = STATUS_METHOD_ERROR;
                goto end;
            }
        }
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }
        Smtp_Context::Ptr smtp_ctx = std::make_shared<Smtp_Context>(method,std::move(emails),content,subject,content_type,charset);
        code = Deal_Real_Event(smtp_ctx);
    }
end:
    ret_j["code"] = code;
    return ret_j.dump();
}


int Smtp_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    int code = STATUS_SEND_EMAIL_SUCCESS;
    Smtp_Context::Ptr smtp_ctx = std::dynamic_pointer_cast<Smtp_Context>(ctx);
    if(smtp_ctx->method == 0){
        int num = 0;
        for (int i = 0; i < CHALLENGE_LENGTH; i++)
        {
            num = this->dis(this->gen);
        }
        std::string msg = "<html><body><h1>Verification Code</h1><p>Your verification code is: <span style=\"color: red\">";
        msg = msg + std::to_string(num) + "</span></p><p>please input your verification code in 5 minutes</p></body></html>";
        if(global_smtp_client->Send_Msg(smtp_ctx->emails,msg,"Verification","text/html","utf8mb4") != 0){
            code = STATUS_SEND_EMAIL_ERROR;
        }
        CacheConn* conn = cachepool->GetCacheConn();
        if(conn->setex(smtp_ctx->emails[0],global_redis_expire_time,std::to_string(num)).empty())
        {
            code = STATUS_SEND_EMAIL_ERROR;
        }
        cachepool->RelCacheConn(conn);
    }
    else if(smtp_ctx->method == 1)
    {
        if(global_smtp_client->Send_Msg(smtp_ctx->emails,smtp_ctx->content,smtp_ctx->subject,smtp_ctx->content_type,smtp_ctx->charset) != 0){
            code = STATUS_SEND_EMAIL_ERROR;
        }
    }
    return code;
}