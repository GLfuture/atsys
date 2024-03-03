#ifndef TIME_API_H
#define TIME_API_H
#include"api_base.h"
#include <ctime>
class Time_API: public API_Base
{
public:
    Time_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;
private:
    time_t Get_Expire_Time(const std::chrono::_V2::system_clock::time_point now);
};




#endif