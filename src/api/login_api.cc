#include "api/login_api.h"



Login_API::Login_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool)
{
    this->mysqlpool = mysqlpool;
    this->cachepool = cachepool;
}



int Login_API::Function(Context_Base::Ptr ctx_ptr)
{
    Login_Context::Ptr login_ctx = std::dynamic_pointer_cast<Login_Context>(ctx_ptr);
    CacheConn* redis_conn = cachepool->GetCacheConn();
    if(!login_ctx->token.empty()){
        //去redis找token
        //存在redis,则返回code 0 和user data
        if(redis_conn->isExists(login_ctx->token)){
            return 0;
        }
    }
    MySqlConn *mysql_conn = mysqlpool->Get_Conn(0);
    std::string condition = "username = '";
    condition = condition + login_ctx->username + "'";
    std::string query = mysql_conn->Select_Query(mysql_conn->Arg_List("password"), "user", condition);
    std::vector<std::vector<std::string>> res;
    mysql_conn->Select(query, res);
    mysqlpool->Ret_Conn(mysql_conn);
    if (res.size()==1)   return 2;
    if (res[1][0].compare(login_ctx->password) != 0)
        return 1;
    std::string token = Generate_Token();
    login_ctx->http->Response_Set_Key_Value("Token", token);
    //token 加入redis
    redis_conn->setex(token,60*60,"1");

    cachepool->RelCacheConn(redis_conn);
    return 0;
}



std::string Login_API::Generate_Token()
{
    int num;
    std::string temp;
    srand((unsigned int)time(NULL));
    for(int i = 0;i < TOKEN_LENGTH ;i++)
    {
        num = rand() % 26;
        temp += ('a' + num);
    }
    return temp;
}