#ifndef EVENT_LAYER_H
#define EVENT_LAYER_H
#include "../base/parser/parser.h"
#include "../base/parser/HTTP.hpp"

class Event_Layer{
public:
    using Ptr = std::shared_ptr<Event_Layer>;
    std::string Deal_Event(HTTP_NSP::HTTP::Ptr http);
private:
    std::string Deal_Login_Event(HTTP_NSP::HTTP::Ptr http);
};



#endif