#include "layer/event_layer.h"

Event_Layer::Event_Layer(API_Manager::Ptr api_manager)
{
    this->api_manager = api_manager;

}

std::string Event_Layer::Deal_Event(HTTP_NSP::HTTP::Ptr http)
{
    http->Response_Set_Status(200);
    std::string ret_str;
    if (http->Request_Get_Http_Type().compare("OPTIONS") == 0)
    {
        ret_str = Deal_OPTIONS_Event(http);
    }else{
        if (http->Request_Get_Http_Type().compare("POST") == 0)
        {
            ret_str = Deal_POST_Event(http);
        }
        else if (http->Request_Get_Http_Type().compare("GET") == 0)
        {
            ret_str = Deal_GET_Event(http);
        }
    }
    http->Response_Set_Key_Value("Connection","close");
    http->Response_Set_Key_Value("Content-Length",std::to_string(ret_str.length()));
    return ret_str;
}

std::string Event_Layer::Deal_OPTIONS_Event(HTTP_NSP::HTTP::Ptr http)
{
    http->Response_Set_Key_Value("Access-Control-Allow-Origin","*");
    http->Response_Set_Key_Value("Access-Control-Allow-Methods","GET, POST");
    http->Response_Set_Key_Value("Access-Control-Allow-Headers","Content-Type, Authorization, X-Requested-With, Content-Length");
    return "";
}


std::string Event_Layer::Deal_POST_Event(HTTP_NSP::HTTP::Ptr http)
{
    std::string url(http->Request_Get_Url());
    API_Base::Ptr api = api_manager->Get_POST_API(url);
    if(api == nullptr){
        json j;
        j["code"] = STATUS_URL_ERROR;
        return j.dump();
    }
    return api->Function(http);
}

std::string Event_Layer::Deal_GET_Event(HTTP_NSP::HTTP::Ptr http)
{
    std::string url(http->Request_Get_Url());
    API_Base::Ptr api = api_manager->Get_GET_API(url);
    if(api == nullptr){
        json j;
        j["code"] = STATUS_URL_ERROR;
        return j.dump();
    }
    return api->Function(http);
}