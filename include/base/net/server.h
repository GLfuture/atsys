/*
 * @Description: 
 * @Version: 4.9
 * @Author: Gong
 * @Date: 2023-09-30 11:59:38
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 07:16:44
 */
#pragma once
#define ENABLE_RBTREE_TIMER 1
#include <map>
#include <memory>
#include <mutex>
#include <functional>
#include <sys/epoll.h>
#include "conn.h"
#include "net_interface.h"
#include "timermanager.hpp"



using std::map;
using std::shared_ptr;
class Callback
{
public:
    Callback()
    {
        Accept_cb = NULL;
        Read_cb = NULL;
        Write_cb = NULL;
        Exit_cb = NULL;
    }
    std::function<void()> Accept_cb;
    std::function<void()> Read_cb;
    std::function<void()> Write_cb;
    std::function<void()> Exit_cb;
};


class Server_Base
{
public:
    Callback callback;
    TimerManager timermanager;

public:
    using Ptr = shared_ptr<Server_Base>;

    //return timerfd
    int Start_Timer();

    Server_Base(int fd , Net_Interface_Base::Ptr& interface);

    int Init_Epoll_Fd();

    int Accept();

    ssize_t Recv(Tcp_Conn_Base::Ptr& conn_ptr,uint32_t len);

    ssize_t Send(const Tcp_Conn_Base::Ptr& conn_ptr,uint32_t len);

    Tcp_Conn_Base::Ptr Get_Conn(int fd) ;

    void Add_Conn(const Tcp_Conn_Base::Ptr& conn_ptr);

    map<uint32_t, Tcp_Conn_Base::Ptr>::iterator Del_Conn(int fd);

    size_t Get_Conn_Num() { return connections.size(); }

    int Get_Epoll_Fd() {    return this->epfd;}

    int Close(int fd);

    void Clean_Conns();

    const char* Get_Ret_Str(Error_Code code){
        const char *arr[] =
            {
                "successful",
                "socket function is failed",
                "connect function is failed",
            };
        return arr[code];
    }

    int Get_Sock() { return _fd; }

#if ENABLE_RBTREE_TIMER | ENABLE_MINHEAP_TIMER
    Timer::Ptr Set_Timeout_cb(uint16_t timerid, uint64_t interval_time, Timer::TimerType type, std::function<void()> &&timeout_cb)
    {
        return timermanager.Add_Timer(timerid, interval_time, type, timeout_cb);
    }
#elif ENABLE_TIMERWHEEL_TIMER
    Timer::Ptr Set_Timeout_cb(uint16_t timerid, uint64_t interval_time, function<void()> &&timeout_cb)
    {
        return timermanager.Add_Timer(timerid, timeout_cb, interval_time);
    }
#endif
#if ENABLE_RBTREE_TIMER
    void Del_Timeout_cb(uint16_t timerid) { timermanager.Del_Timer(timerid); }
#endif
    void Del_Timeout_cb(Timer::Ptr &timer) { timermanager.Del_Timer(timer); }

    virtual ~Server_Base() {
        
    }

private:
    int epfd;
    std::mutex mtx;
    Net_Interface_Base::Ptr interface;
    int _fd;
    map<uint32_t,Tcp_Conn_Base::Ptr> connections;
};


class Server:public Server_Base
{
public:
    using Ptr = shared_ptr<Server>;
    Server(int fd,Net_Interface_Base::Ptr interface):Server_Base(fd,interface)
    {

    }

    ~Server() override {

    }
};
