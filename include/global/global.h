#ifndef GLOBAL_H
#define GLOBAL_H
#include <functional>
#include <iostream>


#define MYSQL_MIN_CONN  4
#define MYSQL_MAX_CONN  6

#define REDIS_MAX_CONN  8
#define TOKEN_LENGTH    26

extern int global_port;
extern int global_worker_num;
extern int global_event_num;
extern int global_backlog;
//1 on 0 off
extern int global_extension;
extern int global_max_http_head_len;
//mysql
extern std::string global_mysql_ip;
extern int global_mysql_port;
extern std::string global_mysql_db_name;
extern std::string global_mysql_username;
extern std::string global_mysql_password;
//redis
extern std::string global_redis_ip;
extern int global_redis_db_index;
extern std::string global_redis_password;
extern int global_redis_port;
extern int global_redis_expire_time;


extern std::function<void(int)> global_accept_cb;
extern std::function<void(int)> global_read_cb;
extern std::function<void(int)> global_write_cb;
extern std::function<void(int)> global_exit_cb;

#endif