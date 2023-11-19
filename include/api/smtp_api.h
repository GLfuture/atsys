#ifndef SMTP_API_H
#define SMTP_API_H

#include "api_base.h"
#include <random>

class Smtp_API : public API_Base
{
public:
    Smtp_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    int Deal_Real_Event(Context_Base::Ptr ctx) override;
private:
    std::mt19937 gen;
    std::uniform_int_distribution<int> dis;
};


#endif