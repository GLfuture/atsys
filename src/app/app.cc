#include "app/app.h"

App::App()
{
    net_layer = std::make_shared<Net_Layer>(global_worker_num);
    mysqlpool = std::make_shared<MySqlPool>("pool",global_mysql_ip,global_mysql_db_name,global_mysql_username,global_mysql_password,global_mysql_port,MYSQL_MIN_CONN,MYSQL_MAX_CONN);
    cachepool = std::make_shared<CachePool>("pool",global_redis_ip.c_str(),global_redis_port,global_redis_db_index,global_redis_password.c_str(),REDIS_MAX_CONN);
    event_layer = std::make_shared<Event_Layer>(mysqlpool,cachepool);
}

void App::start()
{
    net_layer->start(global_port,global_backlog,global_event_num);
}