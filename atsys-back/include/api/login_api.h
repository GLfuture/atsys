#ifndef LOGIN_API_H
#define LOGIN_API_H


#include <cstdlib>
#include "api_base.h"


class Login_API : public API_Base
{
public:
    Login_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    
    std::string Function(HTTP_NSP::HTTP::Ptr) override;
    int Deal_Real_Event(Context_Base::Ptr ctx) override;
private:
    std::string Generate_Token(const param_init_list_t& params);
};

#endif