#ifndef EVENT_LAYER_H
#define EVENT_LAYER_H
#include "../base/parser/parser.h"
#include "../base/parser/HTTP.hpp"
#include "../api/login_api.h"
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>



class Event_Layer{
public:
    using Ptr = std::shared_ptr<Event_Layer>;
    Event_Layer(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    std::string Deal_Event(HTTP_NSP::HTTP::Ptr http);
private:
    std::string Deal_OPTIONS_Event(HTTP_NSP::HTTP::Ptr http);
    std::string Deal_POST_Event(HTTP_NSP::HTTP::Ptr http);
    std::string Deal_Login_Event(HTTP_NSP::HTTP::Ptr http);
    std::string Deal_Register_Event(HTTP_NSP::HTTP::Ptr http);
private:
    Login_API::Ptr login_api;
};



#endif