#include "base/net/reactor.h"

Reactor::Reactor(uint16_t event_num)
{
    this->quit = false;
    this->event_num = event_num;
    this->event = new epoll_event;
}

uint16_t Reactor::Add_Reactor(int epfd , int fd, uint32_t event)
{
    epoll_event ev = {0};
    ev.events = event;
    ev.data.fd = fd;
    uint16_t ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    return ret;
}

uint16_t Reactor::Del_Reactor(int epfd , int fd, uint32_t event)
{
    epoll_event ev = {0};
    ev.events = event;
    ev.data.fd = fd;
    uint16_t ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    return ret;
}

uint16_t Reactor::Mod_Reactor(int epfd , int fd, uint32_t event)
{
    epoll_event ev = {0};
    ev.events = event;
    ev.data.fd = fd;
    uint16_t ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
    return ret;
}

void Reactor::Set_No_Block(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

void Reactor::Set_Block(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    flag &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, flag);
}

void Reactor::Event_Loop( Server_Base::Ptr _server ,int epfd  , int64_t timeout)
{
    epoll_event *events = new epoll_event[this->event_num];
    while (!this->quit)
    {
        _server->timermanager.Update_Timerfd();
        int ready = epoll_wait(epfd, events, this->event_num, timeout); // 默认死等
        if (ready == 0)
            continue;
        for (int i = 0; i < ready; i++)
        {
            *event = events[i];
            if (events[i].data.fd == _server->Get_Sock())
            {
                if (_server->callback.Accept_cb)
                    _server->callback.Accept_cb();
            }
            else if (events[i].events & EPOLLIN)
            {
                if (_server->timermanager.Get_Timerfd() == events[i].data.fd)
                    _server->timermanager.Tick();
                else{
                    if (_server->callback.Read_cb)
                    _server->callback.Read_cb();
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                if (_server->callback.Write_cb)
                    _server->callback.Write_cb();
            }
        }
    }
}