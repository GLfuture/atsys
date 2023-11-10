#ifndef USER_API_H
#define USER_API_H
#include "api_base.h"

class User_Upd_API:public API_Base
{
public:
    User_Upd_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;
private:
    std::string Generate_String_Col(std::string& data);
};




#endif