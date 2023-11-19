#ifndef USER_API_H
#define USER_API_H
#include "api_base.h"

class User_API:public API_Base
{
public:
    User_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;
private:
    std::string Generate_String_Col(std::string& data);
};




#endif