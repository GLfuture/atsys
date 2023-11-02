#pragma once
#ifndef MINHEAP_TIMER_HPP
#define MINHEAP_TIMER_HPP
#endif
#if ENABLE_MINHEAP_TIMER
#include "timer.hpp"
#include <memory>
#include <queue>
namespace Minheap_Timer_NSP
{
    using std::priority_queue;
    using Timer_Ptr = std::shared_ptr<Timer>;

    class Minheap_TimerManager
    {
    public:
        Timer_Ptr Add_Timer(uint16_t timer_id, uint64_t interval_time, Timer::TimerType timer_type,const std::function<void()> &timer_callback)
        {
            Timer *timer = new Timer(timer_id, interval_time, timer_type, timer_callback);
            Timer_Ptr timer_ptr = std::make_shared<Timer>(timer);
            timers.emplace(timer_ptr);
            return timer_ptr;
        }

        bool Del_Timer(Timer_Ptr &timer)
        {
            //if(timer==nullptr) return false;
            timer->Terminate();
            return true;
        }

        void Tick()
        {
            while (Is_Ready())
            {
                Timer_Ptr timer = timers.top();
                timers.pop();
                if (timer->Get_Terminate())
                {
                    timer.reset();
                    continue;
                }
                timer->Exec_Callback();
                if (timer->Get_Timertype() == Timer::TYPE_CIRCLE)
                {
                    Reuse(timer, timer->Get_Interval_Time());
                }
            }
        }

        // 判断最近的定时器是否到期
        bool Is_Ready()
        {
            time_t now = Timer::Get_Current_Time();
            if (!timers.empty() && timers.top()->Get_Expire_Time() <= now)
                return true;
            return false;
        }

        ~Minheap_TimerManager()
        {
            for (int i = 0; i < timers.size(); i++)
            {
                timers.pop();
            }
        }

    protected:
        void Reuse(Timer_Ptr timer, uint64_t interval_time)
        {
            timer->Add_Interval_Time(interval_time);
            // std::cout<<"Reuse "<<timer.get()<<std::endl;
            timers.push(timer);
        }

        class MyCompare
        {
        public:
            bool operator()(const Timer_Ptr &a, const Timer_Ptr &b)
            {
                if(a->Get_Expire_Time() > b->Get_Expire_Time()){
                    return true;
                }else if(a->Get_Expire_Time() < b->Get_Expire_Time()){
                    return false;
                } 
                return a->Get_Timerid() > b->Get_Timerid();
            }
        };

    protected:
        priority_queue<Timer_Ptr, std::vector<Timer_Ptr>, MyCompare> timers;
    };

}
#endif