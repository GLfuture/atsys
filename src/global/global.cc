#include "global/global.h"

int global_port = 0;
int global_worker_num = 0 ;
int global_event_num = 0;
int global_backlog = 0;
//1 on 0 off
int global_extension = 0;
int global_max_http_head_len = 0;

//mysql
std::string global_mysql_ip = "";
int global_mysql_port = 3306;
std::string global_mysql_db_name = "";
std::string global_mysql_username="";
std::string global_mysql_password="";

//redis
std::string global_redis_ip = "";
int global_redis_db_index = 0;
std::string global_redis_password = "";
int global_redis_port = 6379;
int global_redis_expire_time = 0;

std::atomic<uint64_t> global_uid = 0;

std::function<void(int)> global_accept_cb = NULL;
std::function<void(int)> global_read_cb = NULL;
std::function<void(int)> global_write_cb = NULL;
std::function<void(int)> global_exit_cb = NULL;

