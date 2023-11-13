#include "api/card_api.h"

//new
Card_New_API::Card_New_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}


int Card_New_API::Function(Context_Base::Ptr ctx)
{
    Card_New_Context::Ptr card_new_ctx = std::dynamic_pointer_cast<Card_New_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    std::string cname = "'";
    cname = cname + card_new_ctx->cname +"'";
    std::string description = "'";
    description = description + card_new_ctx->description +"'";
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


//del
Card_Del_API::Card_Del_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

int Card_Del_API::Function(Context_Base::Ptr ctx)
{
    Card_Del_Context::Ptr card_del_ctx = std::dynamic_pointer_cast<Card_Del_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    if(card_del_ctx->role == MANAGER){
        std::string condition = "cid = ";
        condition += std::to_string(card_del_ctx->cid);
        std::string query = mysql_conn->Delete_Query("card",condition);
        spdlog::info("card_del_api manager: {}",query);
        mysql_conn->Delete(query);
        if(mysql_conn->Get_Errno()!= 0 || mysql_conn->Get_Affected_Rows() == 0){
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_METHOD_OP_FAIL;
        }
    }else if(card_del_ctx->role == SIMPLE){
        std::string condition = "cid = ";
        condition = condition + std::to_string(card_del_ctx->cid)+" and uid = "+std::to_string(card_del_ctx->uid);
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


//update
Card_Upd_API::Card_Upd_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

int Card_Upd_API::Function(Context_Base::Ptr ctx)
{
    Card_Upd_Context::Ptr card_upd_ctx = std::dynamic_pointer_cast<Card_Upd_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    std::string condition = "uid = ";
    condition = condition + std::to_string(card_upd_ctx->uid) + " and cid = " + std::to_string(card_upd_ctx->cid);
    std::string query = mysql_conn->Select_Query({"num"},"conn",condition);
    std::vector<std::vector<std::string>> res;
    mysql_conn->Select(query,res);
    if(res.size() == 1){
        query = mysql_conn->Insert_Query("conn",{"uid","cid","num"},{std::to_string(card_upd_ctx->uid),std::to_string(card_upd_ctx->cid),std::to_string(card_upd_ctx->num)});
        spdlog::info("card_upd_api: {}",query);
        mysql_conn->Insert(query);
    }else{
        int num = atoi(res[1][0].c_str());
        if(card_upd_ctx->num + num < 0){
            mysqlpool->Ret_Conn(mysql_conn);
            return STATUS_METHOD_OP_FAIL;
        }
        query = mysql_conn->Update_Query("conn", {"num"}, {std::to_string(card_upd_ctx->num + num)}, condition);
        spdlog::info("card_upd_api: {}", query);
        mysql_conn->Update(query);
    }
    
    if(!card_upd_ctx->description.empty()){
        std::string upd_cond = "cid=";
        upd_cond = upd_cond+std::to_string(card_upd_ctx->cid);
        std::string upd_query = mysql_conn->Update_Query("card",{"description"},{card_upd_ctx->description},upd_cond);
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