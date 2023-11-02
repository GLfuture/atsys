#include "global/global.h"

int global_port = 0;
int global_worker_num = 0 ;
int global_event_num = 0;
int global_backlog = 0;
//1 on 0 off
int global_extension = 0;
int global_max_http_head_len = 0;


std::function<void(int)> global_accept_cb = NULL;
std::function<void(int)> global_read_cb = NULL;
std::function<void(int)> global_write_cb = NULL;
std::function<void(int)> global_exit_cb = NULL;

