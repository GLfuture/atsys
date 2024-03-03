#pragma once
#ifndef TIMER_HPP
#define TIMER_HPP
#endif
#include <iostream>
#include <functional>
#include <set>
#include <sys/timerfd.h>
#include <chrono>
#include <time.h>
#include <memory>
class Timer
{
public:
    using Ptr = std::shared_ptr<Timer>;
    enum TimerType
    {
        TYPE_BEG = 0,
        TYPE_ONCE = TYPE_BEG,
        TYPE_CIRCLE,
        TYPE_END
    };

    static inline time_t Get_Current_Time()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
#if ENABLE_MINHEAP_TIMER | ENABLE_RBTREE_TIMER
    Timer(uint16_t timer_id, uint64_t interval_time, TimerType timer_type,const std::function<void()> &timer_callback)
    {
        this->timerid = timer_id;
        this->type = timer_type;
        this->callback = timer_callback;
        this->Is_Terminate = false;
        this->Set_Interval_Time(interval_time);
    }
#elif ENABLE_TIMERWHEEL_TIMER
    Timer(uint16_t timer_id, uint64_t interval_time,uint64_t expire_time, TimerType timer_type,const std::function<void()> &timer_callback)
    {
        this->timerid = timer_id;
        this->type = timer_type;
        this->callback = timer_callback;
        this->Is_Terminate = false;
        this->time_interval = interval_time;
        this->time_expire=expire_time;
    }
#endif
    Timer(const Timer &timer)
    {
        this->timerid = timer.timerid;
        this->time_expire = timer.time_expire;
        this->time_interval = timer.time_interval;
        this->type = timer.type;
        this->callback = timer.callback;
        this->Is_Terminate = timer.Is_Terminate;
    }

    Timer(Timer* &timer)
    {
        this->timerid = timer->timerid;
        this->time_expire = timer->time_expire;
        this->time_interval = timer->time_interval;
        this->type = timer->type;
        this->callback = timer->callback;
        this->Is_Terminate = timer->Is_Terminate;
    }

    // Timer(Timer *timer)
    // {
    //     this->timerid = timer->timerid;
    //     this->time_expire = timer->time_expire;
    //     this->time_interval = timer->time_interval;
    //     this->type = timer->type;
    //     this->callback = timer->callback;
    //     this->Is_Terminate = timer->Is_Terminate;
    // }

    uint16_t Get_Timerid() const
    {
        return this->timerid;
    }

    uint64_t Get_Expire_Time() const
    {
        return time_expire;
    }

    uint64_t Get_Interval_Time() const
    {
        return time_interval;
    }

    TimerType Get_Timertype() const
    {
        return this->type;
    }
#if ENABLE_MINHEAP_TIMER | ENABLE_RBTREE_TIMER
    void Set_Interval_Time(uint64_t time)
    {
        time_interval = time;
        Set_Expire_Time(time_interval);
    }
#elif ENABLE_TIMERWHEEL_TIMER
    void Set_Interval_Time(uint64_t time)
    {
        time_interval = time;   
    }
    void Set_Expire_Time(uint64_t expire_time)
    {
        time_expire = expire_time;
    }
#endif
    bool operator<(const Timer &other) const
    {
        return time_expire < other.time_expire;
    }


    void Add_Interval_Time(uint64_t interval_time)
    {
        this->time_interval=interval_time;
        this->time_expire=this->time_expire+interval_time;
    }

    void Exec_Callback()
    {
        this->callback();
    }

    std::function<void()> Get_Callback()
    {
        return this->callback;
    }

    //销毁任务
    void Terminate()
    {
        this->Is_Terminate = true;
    }

    //是否销毁任务
    bool Get_Terminate()
    {
        return this->Is_Terminate;
    }

private:
#if ENABLE_MINHEAP_TIMER | ENABLE_RBTREE_TIMER
    void Set_Expire_Time(uint64_t time)
    {
        time_expire = Get_Current_Time() + time;
    }
#endif
private:
    uint16_t timerid; // 定时器id
    TimerType type;
    uint64_t time_interval; // 间隔时间
    uint64_t time_expire;   // 过期时间
    std::function<void()> callback;
    bool Is_Terminate;//是否释放
};