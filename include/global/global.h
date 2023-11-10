#ifndef GLOBAL_H
#define GLOBAL_H
#include <functional>
#include <iostream>
#include <atomic>

#define MYSQL_MIN_CONN              4
#define MYSQL_MAX_CONN              6
#define MYSQL_MAX_PAGE_SZIE         50

#define REDIS_MAX_CONN              8


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

extern std::atomic<uint64_t> global_uid;


extern std::function<void(int)> global_accept_cb;
extern std::function<void(int)> global_read_cb;
extern std::function<void(int)> global_write_cb;
extern std::function<void(int)> global_exit_cb;


enum RET_CODE{
    STATUS_LOGIN_SUCCESS,               //login success
    STATUS_LOGIN_PASSWORD_ERROR,        //login password failed
    STATUS_LOGIN_NO_USER_ERROR,         //login no user
    STATUS_REGISTER_SUCCESS,            //register success
    STATUS_REGISTER_REPEAT_USER,        //register repeat user
    STATUS_REGISTER_FAILED,             //register fail
    STATUS_LOGOUT_SUCCESS,              //logout success
    STATUS_LOGOUT_FAILED,               //logout fail
    STATUS_TOKEN_NOT_IN_HEAD,           //token is not in head
    STATUS_TOKEN_EXPIRED,               //token expire
    STATUS_JSON_ERROR,                  //json error
    STATUS_JSON_NO_NEC_MEM,             //json doesn't has necessary member
    STATUS_METHOD_OP_SUCCESS,           //card method operates successfully
    STATUS_METHOD_OP_FAIL,              //card method operates fail
    STATUS_METHOD_ERROR,                //card method error
    STATUS_PRIVILIDGE_ERROR,            //incorrect privilidge
};

enum ROLE_TYPE{
    MANAGER,                            //管理员
    SIMPLE,                             //普通用户
};

// enum CARD_TYPE{
//     ADD_TIME_CARD,                      //增加他人打卡时间
//     RECDUCE_TIME_CARD,                  //减少他人或自己打卡时间
//     RECORD_DEL_CARD,                    //删除一次未打卡完成记录
// };

enum OP_CARD_METHOD{
    OP_NEW_CARD,
    OP_DEL_CARD,
    OP_UPD_CARD,
};

//apis
enum API_TYPE
{
    LOGIN_API,
    REGISTER_API,
    LOGOUT_API,
    CARD_NEW_API,
    CARD_DEL_API,
    CARD_UPD_API,
    DATA_CARD_API,
    DATA_USER_API,
    USER_UPD_API,
    TATOL_API_NUM
};


#endif