#ifndef REGISTER_API_H
#define REGISTER_API_H
#include "api/api_base.h"


class Register_API : public API_Base
{
public:
    Register_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx_ptr) override;
};




#endif