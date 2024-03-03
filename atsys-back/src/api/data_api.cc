#include "api/data_api.h"


Data_Card_Concrete_API::Data_Card_Concrete_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

std::string Data_Card_Concrete_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    int code = 0;
    std::string token(http->Request_Get_Key_Value("Token"));
    if (token.empty())
    {
        code = STATUS_TOKEN_NOT_IN_HEAD;
    }
    else
    {
        jwt::jwt_object obj;
        if (Token_Analyzer::Decode_Token(token, obj).value() == static_cast<int>(jwt::VerificationErrc::TokenExpired))
        {
            code = STATUS_TOKEN_EXPIRED;
        }
        else
        {
            if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
            { // manager
                Data_Card_Concrete_Context::Ptr data_card_self_ctx = std::make_shared<Data_Card_Concrete_Context>(MANAGER, 0);
                code =Deal_Real_Event(data_card_self_ctx);
                return data_card_self_ctx->j.dump();
            }
            else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
            { // simple user
                int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                Data_Card_Concrete_Context::Ptr data_card_self_ctx = std::make_shared<Data_Card_Concrete_Context>(SIMPLE, uid);
                code = Deal_Real_Event(data_card_self_ctx);
                return data_card_self_ctx->j.dump();
            }else{
                code = STATUS_PRIVILIDGE_ERROR;
            }
        }
    }
    return "null";
}

int Data_Card_Concrete_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    Data_Card_Concrete_Context::Ptr data_card_ctx = std::dynamic_pointer_cast<Data_Card_Concrete_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    if(data_card_ctx->role == MANAGER){
        std::string query = "select A.uid,A.username,C.cid,C.cname,B.num from user A right join conn B on A.uid=B.uid left join card C ON B.cid=C.cid;";
        std::vector<std::vector<std::string>> res;
        spdlog::info("data_card_api: {}",query);
        mysql_conn->Select(query,res);
        for(int i=1;i<res.size();i++)
        {
            data_card_ctx->j[res[i][0]]["username"]             = res[i][1];
            data_card_ctx->j[res[i][0]][res[i][2]]["cardname"]  = res[i][3];
            data_card_ctx->j[res[i][0]][res[i][2]]["num"]       = atoi(res[i][4].c_str());
        }
    }else if(data_card_ctx->role == SIMPLE){
        std::string query = "select conn.cid,cname,conn.num from conn left join card on conn.cid=card.cid where uid=";
        query=query+std::to_string(data_card_ctx->uid)+";";
        std::vector<std::vector<std::string>> res;
        spdlog::info("data_card_api: {}",query);
        mysql_conn->Select(query,res);
        for(int i=1;i<res.size();i++)
        {
            data_card_ctx->j[res[i][0]]["cardname"] = res[i][1].c_str();
            data_card_ctx->j[res[i][0]]["num"] = atoi(res[i][2].c_str());
        }
    }
    mysqlpool->Ret_Conn(mysql_conn);
    return STATUS_METHOD_OP_SUCCESS;
}


Data_User_API::Data_User_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

std::string Data_User_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    int code = 0;
    std::string token(http->Request_Get_Key_Value("Token"));
    if (token.empty())
    {
        code = STATUS_TOKEN_NOT_IN_HEAD;
    }
    else
    {
        jwt::jwt_object obj;
        if (Token_Analyzer::Decode_Token(token, obj).value() == static_cast<int>(jwt::VerificationErrc::TokenExpired))
        {
            code = STATUS_TOKEN_EXPIRED;
        }
        else
        {
            if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
            { // manager
                int page = atoi(http->Request_Get_Arg_Value("page").cbegin());
                Data_User_Context::Ptr data_user_ctx = std::make_shared<Data_User_Context>(MANAGER, 0, page);
                Deal_Real_Event(data_user_ctx);
                return data_user_ctx->j.dump();
            }
            else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
            { // simple user
                int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                Data_User_Context::Ptr data_user_ctx = std::make_shared<Data_User_Context>(SIMPLE, uid, 0);
                Deal_Real_Event(data_user_ctx);
                return data_user_ctx->j.dump();
            }else{
                code = STATUS_PRIVILIDGE_ERROR;
            }
        }
    }
    json j;
    j["code"] = code;
    return j.dump();
}


int Data_User_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    Data_User_Context::Ptr data_user_ctx = std::dynamic_pointer_cast<Data_User_Context>(ctx);
    CacheConn* cache_conn = cachepool->GetCacheConn();
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    if(data_user_ctx->role == MANAGER){
        std::string query = "select uid,username,sex,email,phone,address from user limit ";
        query = query + std::to_string(data_user_ctx->page*MYSQL_MAX_PAGE_SZIE)+" , "+std::to_string(MYSQL_MAX_PAGE_SZIE)+";";
        spdlog::info("data_user_api: {}",query);
        std::vector<std::vector<std::string>> res;
        mysql_conn->Select(query,res);
        if(mysql_conn->Get_Errno()!=0){
            cachepool->RelCacheConn(cache_conn);
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_METHOD_ERROR;
        }
        for(int i=1;i<res.size();i++)
        {
            data_user_ctx->j[res[i][0]]["username"] = res[i][1];
            data_user_ctx->j[res[i][0]]["sex"]      = atoi(res[i][2].c_str());
            data_user_ctx->j[res[i][0]]["email"]    = res[i][3];
            data_user_ctx->j[res[i][0]]["phone"]    = res[i][4];
            data_user_ctx->j[res[i][0]]["address"]  = res[i][5];
        }
    }
    else if(data_user_ctx->role == SIMPLE)
    {
        std::string uid = std::to_string(data_user_ctx->uid),username,sex,email,phone,address;
        std::list<std::string> fields = {"username","sex","email","phone","address"};
        std::list<std::string> values;
        if(!cache_conn->hmget(uid,fields,values))
        {
            cachepool->RelCacheConn(cache_conn);
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_METHOD_ERROR;
        }
        for(std::list<std::string>::iterator vit = values.begin(),kit = fields.begin() ;vit!=values.end();vit++,kit++)
        {
            if(*kit == "sex"){
                data_user_ctx->j[uid][*kit] = atoi((*vit).c_str());
            }else{ 
                data_user_ctx->j[uid][*kit] = *vit;
            }
        }
    }
    cachepool->RelCacheConn(cache_conn);
    mysqlpool->Ret_Conn(mysql_conn);
    return STATUS_METHOD_OP_SUCCESS;
}

Data_Time_API::Data_Time_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool)
    :API_Base(mysqlpool,cachepool){}

std::string Data_Time_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    Data_Time_Context::Ptr data_time_ctx = std::make_shared<Data_Time_Context>();
    std::string token(http->Request_Get_Key_Value("Token"));
    if (token.empty())
    {
        data_time_ctx->j["code"] = STATUS_TOKEN_NOT_IN_HEAD;
    }
    else
    {
        jwt::jwt_object obj;
        if (Token_Analyzer::Decode_Token(token, obj).value() == static_cast<int>(jwt::VerificationErrc::TokenExpired))
        {
            data_time_ctx->j["code"] = STATUS_TOKEN_EXPIRED;
        }
        else
        {
            if (obj.payload().has_claim("role"))
            {
                Deal_Real_Event(data_time_ctx);
            }
        }
    }
    return data_time_ctx->j.dump();
}

int Data_Time_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    Data_Time_Context::Ptr data_time_ctx = std::dynamic_pointer_cast<Data_Time_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    std::string query = "select user.uid,username,sex,total_time,need_time from user left join time on user.uid = time.uid;";
    std::vector<std::vector<std::string>> res;
    spdlog::info("data_time_api: {}",query);
    mysql_conn->Select(query,res);
    if(mysql_conn->Get_Errno()!=0 || res.size() <= 1){
        mysqlpool->Ret_Conn(mysql_conn);
        return STATUS_METHOD_OP_FAIL;
    }
    for(int i=1;i<res.size();i++)
    {
        data_time_ctx->j[res[i][0]]["username"] = res[i][1];
        data_time_ctx->j[res[i][0]]["sex"] = atoi(res[i][2].c_str());
        data_time_ctx->j[res[i][0]]["total_time"] = atol(res[i][3].c_str());
        data_time_ctx->j[res[i][0]]["need_time"] = atol(res[i][4].c_str());
    }
    mysqlpool->Ret_Conn(mysql_conn);
    return STATUS_METHOD_OP_SUCCESS;
}


Data_Card_API::Data_Card_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

std::string  Data_Card_API::Function(HTTP_NSP::HTTP::Ptr http)
{
    Data_Card_Context::Ptr data_card_ctx = std::make_shared<Data_Card_Context>();
    std::string token(http->Request_Get_Key_Value("Token"));
    if (token.empty())
    {
        data_card_ctx->j["code"] = STATUS_TOKEN_NOT_IN_HEAD;
    }
    else
    {
        jwt::jwt_object obj;
        if (Token_Analyzer::Decode_Token(token, obj).value() == static_cast<int>(jwt::VerificationErrc::TokenExpired))
        {
            data_card_ctx->j["code"] = STATUS_TOKEN_EXPIRED;
        }
        else
        {
            if (obj.payload().has_claim("role"))
            {
                Deal_Real_Event(data_card_ctx);
            }
        }
        obj.remove_claim("exp");
        obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
        token = obj.signature();
        http->Response_Set_Key_Value("Token", token);
    }
    return data_card_ctx->j.dump();
}

int Data_Card_API::Deal_Real_Event(Context_Base::Ptr ctx)
{
    Data_Card_Context::Ptr data_card_ctx = std::dynamic_pointer_cast<Data_Card_Context>(ctx);
    MySqlConn* conn = mysqlpool->Get_Conn(0);
    std::string query = "select cid,cname,description from card;";
    std::vector<std::vector<std::string>> res;
    spdlog::info("data_card_api: {}",query);
    conn->Select(query,res);
    if(conn->Get_Errno()!=0)
    {
        mysqlpool->Ret_Conn(conn);
        return STATUS_METHOD_OP_FAIL;
    }
    for(int i=1;i<res.size();i++)
    {
        data_card_ctx->j[res[i][0]]["cname"] = res[i][1];
        data_card_ctx->j[res[i][0]]["description"] = res[i][2];
    }
    mysqlpool->Ret_Conn(conn);
    return STATUS_METHOD_OP_SUCCESS;
}