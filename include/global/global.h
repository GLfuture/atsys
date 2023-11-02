#ifndef GLOBAL_H
#define GLOBAL_H
#include <functional>
#include <iostream>


extern int global_port;
extern int global_worker_num;
extern int global_event_num;
extern int global_backlog;
//1 on 0 off
extern int global_extension;
extern int global_max_http_head_len;

extern std::function<void(int)> global_accept_cb;
extern std::function<void(int)> global_read_cb;
extern std::function<void(int)> global_write_cb;
extern std::function<void(int)> global_exit_cb;

#endif