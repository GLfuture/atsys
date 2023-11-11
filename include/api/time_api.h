#ifndef TIME_API_H
#define TIME_API_H
#include"api_base.h"
#include <ctime>
class Time_API: public API_Base
{
public:
    Time_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;
private:
    time_t Get_Expire_Time(const std::chrono::_V2::system_clock::time_point now);
};




#endif