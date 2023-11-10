#ifndef LOGOUT_API_H
#define LOGOUT_API_H

#include "api_base.h"

class Logout_API:public API_Base
{
public:
    
    Logout_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;

};


#endif