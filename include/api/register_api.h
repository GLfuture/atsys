#ifndef REGISTER_API_H
#define REGISTER_API_H
#include "api/api_base.h"


class Register_API : public API_Base
{
public:
    Register_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;
};




#endif