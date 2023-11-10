#include "api/api_manager.h"


int API_Manager::Register_API(API_Base::Ptr api,int indx)
{
    _apis[indx] = api;
    return 0;
}

int API_Manager::Register_APIs(std::vector<API_Base::Ptr> apis)
{
    for(int i=0;i<apis.size();i++)
    {
        _apis.push_back(apis[i]);
    }
    return _apis.size();
}