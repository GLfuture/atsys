#ifndef DATA_API_H
#define DATA_API_H

#include "api_base.h"


class Data_Card_Concrete_API:public API_Base
{
public:
    using Ptr = std::shared_ptr<Data_Card_Concrete_API>;
    Data_Card_Concrete_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;
};

class Data_User_API:public API_Base
{
public:
    using Ptr = std::shared_ptr<Data_User_API>;
    Data_User_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;
};

class Data_Time_API:public API_Base
{
public:
    using Ptr = std::shared_ptr<Data_Time_API>;
    Data_Time_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;
};


class Data_Card_API:public API_Base
{
public:
    using Ptr = std::shared_ptr<Data_Card_API>;
    Data_Card_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;
};

#endif