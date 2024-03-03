#ifndef API_MANAGER_H
#define API_MANAGER_H
#include "api/login_api.h"
#include "api/register_api.h"
#include "api/card_api.h"
#include "api/data_api.h"
#include "api/logout_api.h"
#include "api/user_api.h"
#include "api/time_api.h"
#include "api/smtp_api.h"
#include <map>

class API_Manager
{
public:
    using Ptr = std::shared_ptr<API_Manager>;

    void Register_GET_API(std::string url , API_Base::Ptr api);

    void Register_POST_API(std::string url , API_Base::Ptr api);

    API_Base::Ptr Get_GET_API(std::string url);

    API_Base::Ptr Get_POST_API(std::string url);
private:
    std::map<std::string,API_Base::Ptr> get_apis;
    std::map<std::string,API_Base::Ptr> post_apis;
};


#endif