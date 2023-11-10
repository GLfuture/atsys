#ifndef API_MANAGER_H
#define API_MANAGER_H
#include "api/login_api.h"
#include "api/register_api.h"
#include "api/card_api.h"
#include "api/data_api.h"
#include "api/logout_api.h"
#include "api/user_api.h"

class API_Manager
{
public:
    using Ptr = std::shared_ptr<API_Manager>;
    API_Manager(int num){
        _apis.assign(num,nullptr);
    }
    //注册api并返回该api的下标
    int Register_API(API_Base::Ptr api,int indx);

    int Register_APIs(std::vector<API_Base::Ptr> apis);

    API_Base::Ptr Get_API(int i){
        return _apis[i];
    }

private:
    std::vector<API_Base::Ptr> _apis;
};


#endif