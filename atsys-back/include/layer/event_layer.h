#ifndef EVENT_LAYER_H
#define EVENT_LAYER_H

#include "../api/api_manager.h"


class Event_Layer{
public:
    using Ptr = std::shared_ptr<Event_Layer>;
    Event_Layer(API_Manager::Ptr api_manager);
    std::string Deal_Event(HTTP_NSP::HTTP::Ptr http);
private:
    std::string Deal_OPTIONS_Event(HTTP_NSP::HTTP::Ptr http);
    std::string Deal_POST_Event(HTTP_NSP::HTTP::Ptr http);
    std::string Deal_GET_Event(HTTP_NSP::HTTP::Ptr http);

private:
    API_Manager::Ptr api_manager;
};



#endif