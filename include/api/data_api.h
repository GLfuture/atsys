#ifndef DATA_API_H
#define DATA_API_H

#include "api_base.h"


class Data_Card_API:public API_Base
{
public:
    using Ptr = std::shared_ptr<Data_Card_API>;
    Data_Card_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;
};

class Data_User_API:public API_Base
{
public:
    using Ptr = std::shared_ptr<Data_User_API>;
    Data_User_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;
};
 


#endif