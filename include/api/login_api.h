#ifndef LOGIN_API_H
#define LOGIN_API_H


#include <cstdlib>
#include "api_base.h"




class Login_API : public API_Base
{
public:
    Login_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx_ptr) override;
private:
    std::string Generate_Token(const param_init_list_t& params);
};

#endif