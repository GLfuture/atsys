#pragma once
#ifndef TIMERMANAGER_HPP
#define TIMERMANAGER_HPP
#endif
#if ENABLE_RBTREE_TIMER
#include"underlying/rbtree-timer.hpp"
#elif ENABLE_MINHEAP_TIMER
#include"underlying/minheap-timer.hpp"
#elif ENABLE_TIMERWHEEL_TIMER
#include "underlying/timewheel-timer.hpp"
#endif

#if ENABLE_RBTREE_TIMER
class TimerManager:public RBtree_Timer_NSP::RBtree_TimerManager
{
public:
    using Ptr = std::shared_ptr<TimerManager>;
    int Get_Timerfd()
    {
        return this->timerfd;
    }

    int Create_Timerfd(){
        timerfd = timerfd_create(CLOCK_MONOTONIC,0);
        return timerfd;
    }

    virtual void Update_Timerfd() {
        struct timespec abstime;
        auto iter = timers.begin();
        if (iter != timers.end()) {
            abstime.tv_sec = (*iter)->Get_Expire_Time() / 1000;
            abstime.tv_nsec = ((*iter)->Get_Expire_Time() % 1000) * 1000000;
        } else {
            abstime.tv_sec = 0;
            abstime.tv_nsec = 0;
        }
        struct itimerspec its = {
            .it_interval = {},
            .it_value = abstime
        };
        timerfd_settime(this->timerfd, TFD_TIMER_ABSTIME, &its, nullptr);
    } 
private:
    int timerfd;
};

#elif ENABLE_MINHEAP_TIMER
class TimerManager:public Minheap_Timer_NSP::Minheap_TimerManager
{
public:
    int Get_Timerfd()
    {
        return this->timerfd;
    }

    int Create_Timerfd(){
        timerfd = timerfd_create(CLOCK_MONOTONIC,0);
        return timerfd;
    }

    virtual void Update_Timerfd() {
        struct timespec abstime;
        if(timers.empty()){
            abstime.tv_sec = 0;
            abstime.tv_nsec = 0;
        }else {
            abstime.tv_sec = timers.top()->Get_Expire_Time() / 1000;
            abstime.tv_nsec = (timers.top()->Get_Expire_Time() % 1000) * 1000000;
        } 
        struct itimerspec its = {
            .it_interval = {},
            .it_value = abstime
        };
        timerfd_settime(this->timerfd, TFD_TIMER_ABSTIME, &its, nullptr);
    } 
private:
    int timerfd;
};

#elif ENABLE_TIMERWHEEL_TIMER
class TimerManager:public TimeWheel_Timer_NSP::TimeWheel_TimerManager
{
public:
    TimerManager(uint8_t wheel_num , uint16_t slot_size,uint16_t span):
        TimeWheel_TimerManager(wheel_num,slot_size,span)
    {

    }
};

#endif