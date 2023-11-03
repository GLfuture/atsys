#ifndef LOGIN_API_H
#define LOGIN_API_H

#include "../global/global.h"
#include "api_base.h"
#include "base/mysqlpool/MySqlPool.h"
#include "base/cachepool/CachePool.h"
#include <cstdlib>



class Login_API : public API_Base
{
public:
    Login_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx_ptr) override;
private:
    std::string Generate_Token();

private:
    MySqlPool::Ptr mysqlpool;
    CachePool::Ptr cachepool;
};

#endif