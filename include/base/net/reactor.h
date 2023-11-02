/*
 * @Description: 
 * @Version: 4.9
 * @Author: Gong
 * @Date: 2023-09-30 11:59:38
 * @LastEditors: Gong
 * @LastEditTime: 2023-09-30 12:40:53
 */
#pragma once
#include <cstdint>
#include "server.h"
#include <fcntl.h>

using std::function;
// 注意一个端口对应一个reactor管理
class Reactor
{
public:
    using Ptr = shared_ptr<Reactor>;
    /// @brief
    /// @param event_num
    /// @param buffer_size
    Reactor(uint16_t event_num);

    uint16_t Add_Reactor(int epfd ,int fd, uint32_t event);

    uint16_t Del_Reactor(int epfd ,int fd, uint32_t event);
    

    uint16_t Mod_Reactor(int epfd ,int fd, uint32_t event);

    // 设置非阻塞
    void Set_No_Block(int fd);

    // 设置阻塞
    void Set_Block(int fd);

    // 获取事件数量
    uint32_t Get_Event_Num() { return this->event_num; }

    // 返回当前事件
    epoll_event Get_Now_Event() { return *event; }

    // 事件主循环,默认死等
    void Event_Loop(Server_Base::Ptr _server,int epfd  ,int64_t timeout = -1);

    ~Reactor()
    {
        delete event;
    }

private:
    bool quit;
    uint32_t event_num;
    epoll_event *event;
};
