#include "api/user_api.h"

User_Upd_API::User_Upd_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool):
    API_Base(mysqlpool,cachepool){}

int User_Upd_API::Function(Context_Base::Ptr ctx)
{
    User_Upd_Context::Ptr user_upd_ctx = std::dynamic_pointer_cast<User_Upd_Context>(ctx);
    MySqlConn* mysql_conn = mysqlpool->Get_Conn(0);
    CacheConn* cache_conn = cachepool->GetCacheConn();
    std::vector<std::string> fields,values;
    std::map<std::string,std::string> hash;
    if(!user_upd_ctx->address.empty()){
        fields.push_back("address");
        values.push_back(Generate_String_Col(user_upd_ctx->address));
        hash["address"] = user_upd_ctx->address;
    }
    if(!user_upd_ctx->email.empty()){
        fields.push_back("email");
        values.push_back(Generate_String_Col(user_upd_ctx->email));
        hash["email"] = user_upd_ctx->email;
    }
    if(!user_upd_ctx->password.empty()){
        fields.push_back("password");
        values.push_back(Generate_String_Col(user_upd_ctx->password));
        hash["password"] = user_upd_ctx->password;
    }
    if(!user_upd_ctx->phone.empty()){
        fields.push_back("phone");
        values.push_back(Generate_String_Col(user_upd_ctx->phone));
        hash["phone"] = user_upd_ctx->phone;
    }
    if(user_upd_ctx->sex != -1){
        fields.push_back("sex");
        values.push_back(std::to_string(user_upd_ctx->sex));
        hash["sex"] = std::to_string(user_upd_ctx->sex);
    }
    std::string condition = " uid = ";
    condition = condition + std::to_string(user_upd_ctx->uid) ;
    std::string query = mysql_conn->Update_Query("user",std::move(fields),std::move(values),condition);
    spdlog::info("user_upd_api: {}",query);
    mysql_conn->Update(query);
    if(mysql_conn->Get_Errno() != 0 || cache_conn->hmset(std::to_string(user_upd_ctx->uid),hash).empty()){
        mysqlpool->Ret_Conn(mysql_conn);
        cachepool->RelCacheConn(cache_conn);
        return STATUS_METHOD_OP_FAIL;
    }
    mysqlpool->Ret_Conn(mysql_conn);
    cachepool->RelCacheConn(cache_conn);
    return STATUS_METHOD_OP_SUCCESS;
}


std::string User_Upd_API::Generate_String_Col(std::string& data)
{
    std::string res = "'";
    res += data;
    return res+"'";
}