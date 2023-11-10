#include "api/data_api.h"


Data_Card_API::Data_Card_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

int Data_Card_API::Function(Context_Base::Ptr ctx)
{
    Data_Card_Context::Ptr data_card_ctx = std::dynamic_pointer_cast<Data_Card_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    if(data_card_ctx->role == MANAGER){
        std::string query = "select A.uid,A.username,C.cid,C.name,B.num from user A right join conn B on A.uid=B.uid left join card C ON B.cid=C.cid;";
        std::vector<std::vector<std::string>> res;
        spdlog::info("data_card_api: {}",query);
        mysql_conn->Select(query,res);
        for(int i=1;i<res.size();i++)
        {
            data_card_ctx->j[res[i][1]]["uid"] = atoi(res[i][0].c_str());
            data_card_ctx->j[res[i][1]][res[i][3]]["cid"] = atoi(res[i][2].c_str());
            data_card_ctx->j[res[i][1]][res[i][3]]["num"] = atoi(res[i][4].c_str());
        }
    }else if(data_card_ctx->role == SIMPLE){
        std::string query = "select conn.cid,name,conn.num from conn left join card on conn.cid=card.cid where uid=";
        query=query+std::to_string(data_card_ctx->uid)+";";
        std::vector<std::vector<std::string>> res;
        spdlog::info("data_card_api: {}",query);
        mysql_conn->Select(query,res);
        for(int i=1;i<res.size();i++)
        {
            data_card_ctx->j[res[i][1]]["cid"] = atoi(res[i][0].c_str());
            data_card_ctx->j[res[i][1]]["num"] = atoi(res[i][2].c_str());
        }
    }
    mysqlpool->Ret_Conn(mysql_conn);
    return STATUS_METHOD_OP_SUCCESS;
}


Data_User_API::Data_User_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

int Data_User_API::Function(Context_Base::Ptr ctx)
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
            data_user_ctx->j[res[i][0]]["phone"] = res[i][4];
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