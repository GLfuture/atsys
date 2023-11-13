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
    std::string Deal_No_Token_Event();
    std::string Deal_Token_Expire_Event();
    std::string Deal_Incorrect_Api_Event();

    std::string Deal_Login_Event(HTTP_NSP::HTTP::Ptr http);
    std::string Deal_Register_Event(HTTP_NSP::HTTP::Ptr http);
    std::string Deal_Logout_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);
    std::string Deal_Card_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);
    std::string Deal_Time_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);
    std::string Deal_Data_Card_Concrete_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);
    std::string Deal_Data_Card_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);
    std::string Deal_Data_User_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);
    std::string Deal_Data_Time_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);
    std::string Deal_User_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj);

private:
    API_Manager::Ptr api_manager;
};



#endif