#include "api/card_api.h"



int Card_API::Deal_New_Card_Event(Context_Base::Ptr ctx)
{
    Card_Context::Ptr card_ctx = std::dynamic_pointer_cast<Card_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    std::string cname = "'";
    cname = cname + card_ctx->cname +"'";
    std::string description = "'";
    description = description + card_ctx->description +"'";
    std::string query = mysql_conn->Insert_Query("card",{"cname","description"},{cname,description});
    spdlog::info("card_new_api: {}",query);
    mysql_conn->Insert(query);
    if(mysql_conn->Get_Errno()!=0){
        mysqlpool->Ret_Conn(mysql_conn);
        return STATUS_METHOD_OP_FAIL;
    }
    mysqlpool->Ret_Conn(mysql_conn);
    return STATUS_METHOD_OP_SUCCESS;
}



int Card_API::Deal_Del_Card_Event(Context_Base::Ptr ctx)
{
    Card_Context::Ptr card_ctx = std::dynamic_pointer_cast<Card_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    if(card_ctx->role == MANAGER){
        std::string condition = "cid = ";
        condition += std::to_string(card_ctx->cid);
        std::string query = mysql_conn->Delete_Query("card",condition);
        spdlog::info("card_del_api manager: {}",query);
        mysql_conn->Delete(query);
        if(mysql_conn->Get_Errno()!= 0 || mysql_conn->Get_Affected_Rows() == 0){
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_METHOD_OP_FAIL;
        }
    }else if(card_ctx->role == SIMPLE){
        std::string condition = "cid = ";
        condition = condition + std::to_string(card_ctx->cid)+" and uid = "+std::to_string(card_ctx->uid);
        std::string query = mysql_conn->Delete_Query("conn",condition);
        spdlog::info("card_del_api simple: {}",query);
        mysql_conn->Delete(query);
        if(mysql_conn->Get_Errno()!= 0 || mysql_conn->Get_Affected_Rows() == 0){
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_METHOD_OP_FAIL;
        }
    }
    mysqlpool->Ret_Conn(mysql_conn);
    return STATUS_METHOD_OP_SUCCESS;
}


int Card_API::Deal_Upd_Card_Event(Context_Base::Ptr ctx)
{
    Card_Context::Ptr card_ctx = std::dynamic_pointer_cast<Card_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    std::string condition = "uid = ";
    condition = condition + std::to_string(card_ctx->uid) + " and cid = " + std::to_string(card_ctx->cid);
    std::string query = mysql_conn->Select_Query({"num"},"conn",condition);
    std::vector<std::vector<std::string>> res;
    mysql_conn->Select(query,res);
    if(res.size() == 1){
        query = mysql_conn->Insert_Query("conn",{"uid","cid","num"},{std::to_string(card_ctx->uid),std::to_string(card_ctx->cid),std::to_string(card_ctx->num)});
        spdlog::info("card_upd_api: {}",query);
        mysql_conn->Insert(query);
    }else{
        int num = atoi(res[1][0].c_str());
        if(card_ctx->num + num < 0){
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_METHOD_OP_FAIL;
        }
        query = mysql_conn->Update_Query("conn", {"num"}, {std::to_string(card_ctx->num + num)}, condition);
        spdlog::info("card_upd_api: {}", query);
        mysql_conn->Update(query);
    }
    
    if(!card_ctx->description.empty()){
        std::string upd_cond = "cid=";
        upd_cond = upd_cond+std::to_string(card_ctx->cid);
        std::string upd_query = mysql_conn->Update_Query("card",{"description"},{card_ctx->description},upd_cond);
        spdlog::info("card_upd_api: {}",query);
        mysql_conn->Update(upd_query);
    }
    if(mysql_conn->Get_Errno() != 0)
    {
        mysqlpool->Ret_Conn(mysql_conn);
        return STATUS_METHOD_OP_FAIL;
    }

    mysqlpool->Ret_Conn(mysql_conn);
    
    return STATUS_METHOD_OP_SUCCESS;
}


Card_API::Card_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}


std::string Card_API::Function(HTTP_NSP::HTTP::Ptr http)
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
            std::string body(http->Request_Get_Body());
            int method = 0 , cid = 0, uid = 0,num = 0;
            std::string cname = "", description = "";
            if (json::accept(body))
            {
                json j = json::parse(body);
                if (j.contains("method") && j["method"].is_number())
                {
                    method = j["method"];
                    if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid")){
                        switch (method)
                        {
                        case OP_NEW_CARD:
                        {
                            if (j.contains("cname") && j["cname"].is_string()&&j.contains("description") && j["description"].is_string())
                            {
                                cname = j["cname"];
                                description = j["description"];
                            }
                            else
                            {
                                code = STATUS_JSON_NO_NEC_MEM;
                                goto end;
                            }
                            break;
                        }
                        case OP_DEL_CARD:
                        {
                            if (j.contains("cid") && j["cid"].is_number())
                            {
                                cid = j["cid"];
                            }else{
                                code = STATUS_JSON_NO_NEC_MEM;
                                goto end;
                            }
                            break;
                        }
                        case OP_UPD_CARD:
                        {
                            if(j.contains("cid") && j["cid"].is_number() && j.contains("num") && j["num"].is_number()&& j.contains("uid")&& j["uid"].is_number()){
                                cid = j["cid"];
                                num = j["num"];
                                uid = j["uid"];
                                if (j.contains("description") && j["description"].is_string())
                                {
                                    description = j["description"];
                                }
                            }else{
                                code = STATUS_JSON_NO_NEC_MEM;
                                goto end;
                            }
                            break;
                        }
                        default:
                        {
                            code = STATUS_METHOD_ERROR;
                            goto end;
                        }
                        }
                        Card_Context::Ptr card_ctx = std::make_shared<Card_Context>(method,cname,MANAGER,uid,cid,num,description);
                        code = Deal_Real_Event(card_ctx);
                    }else if(obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid")){
                        switch (method)
                        {
                        case OP_DEL_CARD:
                        {
                            if (j.contains("cid") && j["cid"].is_number())
                            {
                                cid = j["cid"];
                            }else{
                                code = STATUS_JSON_NO_NEC_MEM;
                                goto end;
                            }
                            break;
                        }
                        case OP_UPD_CARD:
                        {
                            if(j.contains("cid") && j["cid"].is_number() && j.contains("num") && j["num"].is_number()){
                                cid = j["cid"];
                                num = j["num"];
                                if (j.contains("description") && j["description"].is_string())
                                {
                                    description = j["description"];
                                }
                            }else{
                                code = STATUS_JSON_NO_NEC_MEM;
                                goto end;
                            }
                            break;
                        }
                        default:
                        {
                            code = STATUS_METHOD_ERROR;
                            goto end;
                        }
                        }
                        uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                        Card_Context::Ptr card_ctx = std::make_shared<Card_Context>(method,cname,SIMPLE,uid,cid,num,description);
                        code = Deal_Real_Event(card_ctx);
                    }else{
                        code = STATUS_PRIVILIDGE_ERROR;
                    }
        
                }
                else
                {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
            }
            else
            {
                code = STATUS_JSON_ERROR;
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

int Card_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    int code = 0;
    Card_Context::Ptr card_ctx = std::dynamic_pointer_cast<Card_Context>(ctx);
    switch (card_ctx->method)
    {
    case OP_NEW_CARD:
    {
        code = Deal_New_Card_Event(card_ctx);
        break;
    }
    case OP_DEL_CARD:
    {
        code = Deal_Del_Card_Event(card_ctx);
        break;
    }
    case OP_UPD_CARD:
    {
        code = Deal_Upd_Card_Event(card_ctx);
        break;
    }
    default:
        code = STATUS_METHOD_ERROR;
        break;
    }
    return code;
}