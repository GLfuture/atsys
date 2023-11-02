#include "layer/event_layer.h"

std::string Event_Layer::Deal_Event(HTTP_NSP::HTTP::Ptr http)
{
    std::string_view url = http->Request_Get_Url();
    std::string ret_str;
    if(url.compare("/api/login") == 0){
        ret_str = Deal_Login_Event(http);
    }else if(url.compare("/api/register") == 0){

    }else{

    }
    return ret_str;
}

std::string Event_Layer::Deal_Login_Event(HTTP_NSP::HTTP::Ptr http)
{
    std::string body(http->Request_Get_Body());
    Parser_NSP::Parser_Json::Ptr json_ptr = std::make_shared<Parser_NSP::Parser_Json>();
    if(json_ptr->Prase(body) == 0){
        std::string username,password;
        json_ptr->Get_Value<std::string>(json_ptr->Get_Root(),"username",username);
        json_ptr->Get_Value<std::string>(json_ptr->Get_Root(),"password",password);
        std::cout<<username<<"  "<<password<<std::endl;
    }

    return body;
}