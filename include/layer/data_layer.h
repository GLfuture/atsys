#ifndef DATA_LAYER_H
#define DATA_LAYER_H
#include "../global/global.h"
#include "base/mysqlpool/MySqlPool.h"
#include "base/cachepool/CachePool.h"

class Data_Layer
{
public:
    using Ptr = std::shared_ptr<Data_Layer>;
    Data_Layer()
    {
        mysqlpool = std::make_shared<MySqlPool>("pool",global_mysql_ip,global_mysql_db_name,global_mysql_username,global_mysql_password,global_mysql_port,MYSQL_MIN_CONN,MYSQL_MAX_CONN);
        cachepool = std::make_shared<CachePool>("pool",global_redis_ip.c_str(),global_redis_port,global_redis_db_index,global_redis_password.c_str(),REDIS_MAX_CONN);
    }

    MySqlPool::Ptr Get_Mysql_Pool() {   return this->mysqlpool; }
    CachePool::Ptr Get_Cache_Pool() {   return this->cachepool; }

private:
    MySqlPool::Ptr mysqlpool;
    CachePool::Ptr cachepool;
};


#endif