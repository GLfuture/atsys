#pragma once
#ifndef TIMERWHEEL_TIMER_HPP
#define TIMERWHEEL_TIMER_HPP
#endif
#if ENABLE_TIMERWHEEL_TIMER
#include "timer.hpp"
#include <list>
#include <memory>
#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <mutex>
#include <limits>
namespace TimeWheel_Timer_NSP
{
    #define log(x) printf(x)
    using namespace std::chrono;
    using std::vector;
    using std::thread;
    using std::list;
    using Thread_Ptr = std::shared_ptr<thread>;
    // 定时器回调函数类型
    using Timer_Ptr = std::shared_ptr<Timer>;
    using Timer_List = std::list<Timer_Ptr>;
    //在时间轮中，定时器的expire_time均是相对于时间轮创建的时间而言的
    class TimeWheel_TimerManager
    {
    public:
        TimeWheel_TimerManager(uint8_t wheel_num , uint16_t slot_size,uint16_t span) 
        {
            this->time_point = 0;
            for (int i = 0; i < wheel_num; ++i)
            {
                Timer_List timer_list;
                vector<Timer_List> arr;
                arr.assign(slot_size,timer_list);
                this->slots.push_back(arr);
            }
            this->slot_size=slot_size;
            this->wheel_num=wheel_num;
            this->span=span;
        }

        // 添加定时器
        Timer_Ptr Add_Timer(uint16_t timer_id, uint64_t interval_time,const std::function<void()> &callback)
        {
            if(std::numeric_limits<unsigned long long>::max() - interval_time < this->time_point){
                log("overflow\n");
                return NULL;
            }
            uint64_t expire_time = this->time_point + interval_time;
            Timer_Ptr timer_ptr=std::make_shared<Timer>(new Timer(timer_id,interval_time,expire_time,Timer::TYPE_ONCE,callback));
            Insert_Timer(timer_ptr,expire_time/span,0);
            return timer_ptr;
        }

        bool Destory_Timewheel()
        {
            this->terminate=true;
            return false;
        }

        bool Del_Timer(Timer_Ptr &timer)
        {
            //if(timer==nullptr) return false;
            timer->Terminate();
            return true;
        }

        // 启动定时器线程
        void Start()
        {
            this->th = std::make_shared<thread>(&TimeWheel_TimerManager::Exec_Timewheel,this);
            this->th->detach();
        }
        
        uint64_t Get_Time_Point()
        {
            return this->time_point;
        }

    private:

        void Exec_Timewheel()
        {
            //int i=0;
            while(!this->terminate)
            {
                uint64_t cur_slot=this->time_point / span;
                for(auto& v:slots[0][cur_slot % slot_size]){
                    if(!v->Get_Terminate()) 
                    {
                        printf("before exec:%ld\n",this->time_point);
                        v->Exec_Callback();
                    }
                    v.reset();
                }
                slots[0][cur_slot % slot_size].clear();
                if(std::numeric_limits<unsigned long long>::max() - span < this->time_point){
                    log("destroy timewheel\n");
                    break;
                }
                this->time_point += span;
                if(cur_slot >= pow(static_cast<float>(slot_size),static_cast<float>(wheel_num))){
                    //printf("time_point:%ld,oversize:%f\n",this->time_point,pow(static_cast<float>(slot_size),static_cast<float>(wheel_num)));
                    log("vector overflow\n");
                    break;
                }
                if(cur_slot >= slot_size ){
                    Shift_Timer(cur_slot , 0);
                }
                std::this_thread::sleep_for(milliseconds(span));
            }
        }


        void Insert_Timer(Timer_Ptr timer_ptr, uint64_t flag , uint8_t timewheel_idx)
        {
            if(flag / slot_size == 0){
                std::lock_guard<std::mutex> lock(mtx);
                printf("before add:%ld\n",Get_Time_Point());
                slots[timewheel_idx][flag % slot_size].emplace_back(timer_ptr);
                //printf("time_point:%ld\tlocation: %d\t %ld\n",this->time_point,timewheel_idx,flag % slot_size);
            }else{
                Insert_Timer(timer_ptr,flag / slot_size , ++timewheel_idx);
            }
        }

        void Shift_Timer(uint64_t flag , uint8_t timewheel_idx)
        {
            if(flag / slot_size == 0){
                //printf("time_point:%ld\tlocation: %d\t %ld\n",this->time_point,timewheel_idx,flag % slot_size);
                if(slots[timewheel_idx][flag % slot_size].empty()) return ;
                for(auto &v:slots[timewheel_idx][flag % slot_size]){
                    slots[0][v->Get_Expire_Time()/span % slot_size].emplace_back(v);
                }
                slots[timewheel_idx][flag % slot_size].clear();
            }else{
                Shift_Timer(flag / slot_size,++timewheel_idx);
            }
        }

    protected:
        std::mutex mtx;
        vector<vector<Timer_List>> slots; // 时间轮槽
        uint64_t time_point;//时间指针
        uint8_t wheel_num;//时间轮层数
        uint16_t slot_size;//时间轮大小
        uint16_t span; //时间间隔
        Thread_Ptr th;//时间tick线程
        bool terminate;//是否销毁
    };


}
#endif