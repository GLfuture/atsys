#include "api/api_manager.h"


void API_Manager::Register_GET_API(std::string url , API_Base::Ptr api)
{
    get_apis[url] = api;
}

API_Base::Ptr API_Manager::Get_GET_API(std::string url)
{
    std::map<std::string,API_Base::Ptr>::iterator it = get_apis.find(url);
    if(it == get_apis.end()) return nullptr;
    return it->second;
}


void API_Manager::Register_POST_API(std::string url , API_Base::Ptr api)
{
    post_apis[url] = api;
}

API_Base::Ptr API_Manager::Get_POST_API(std::string url)
{
    std::map<std::string,API_Base::Ptr>::iterator it = post_apis.find(url);
    if(it == post_apis.end()) return nullptr;
    return it->second;
}