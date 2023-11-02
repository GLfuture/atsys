#pragma once
#ifndef RBTREE_TIMER_HPP
#define RBTREE_TIMER_HPP
#endif
#if ENABLE_RBTREE_TIMER
#include "timer.hpp"
#include <memory>
namespace RBtree_Timer_NSP
{
    using std::multiset;
    using Timer_Ptr = std::shared_ptr<Timer>;
    class RBtree_TimerManager
    {
    public:
        Timer_Ptr Add_Timer(uint16_t timer_id, uint64_t interval_time, Timer::TimerType timer_type,const std::function<void()> &timer_callback)
        {
            Timer *timer = new Timer(timer_id, interval_time, timer_type, timer_callback);
            Timer_Ptr timer_ptr = std::make_shared<Timer>(timer);
            if(timers.empty() || timer_ptr->Get_Expire_Time()<=(*timers.crbegin())->Get_Expire_Time()){
                timers.emplace(timer_ptr);
            }else {
                timers.emplace_hint(timers.end(),timer_ptr);
            }
            return timer_ptr;
        }

        bool Del_Timer(uint16_t timerid)
        {
            for (multiset<Timer_Ptr>::iterator it = timers.begin(); it != timers.end(); it++)
            {
                if ((*it)->Get_Timerid() == timerid)
                {
                    timers.erase(it);
                    return true;
                }
            }
            return false;
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
                Timer_Ptr timer = (*timers.begin());
                timers.erase(timers.begin());
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
            if (!timers.empty() && (*timers.begin())->Get_Expire_Time() <= now)
                return true;
            return false;
        }

        ~RBtree_TimerManager()
        {
            for (multiset<Timer_Ptr, MyCompare>::iterator it = this->timers.begin(); it != this->timers.end(); it++)
            {
                timers.erase(it);
            }
        }

    protected:
        void Reuse(Timer_Ptr timer, uint64_t interval_time)
        {
            timer->Add_Interval_Time(interval_time);
            timers.insert(timer);
        }

        class MyCompare
        {
        public:
            bool operator()(const Timer_Ptr &a, const Timer_Ptr &b) const
            {
                if(a->Get_Expire_Time() < b->Get_Expire_Time()){
                    return true;
                }else if(a->Get_Expire_Time()>b->Get_Expire_Time()){
                    return false;
                }
                return a->Get_Timerid() < b->Get_Timerid();
            }
        };

        multiset<Timer_Ptr, MyCompare> timers;
    };
}
#endif