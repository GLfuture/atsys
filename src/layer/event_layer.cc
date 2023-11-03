#include "layer/event_layer.h"

Event_Layer::Event_Layer(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool)
{
    login_api = std::make_shared<Login_API>(mysqlpool,cachepool);
}

std::string Event_Layer::Deal_Event(HTTP_NSP::HTTP::Ptr http)
{
    http->Response_Set_Status(200);
    std::string ret_str;
    if(http->Request_Get_Http_Type().compare("OPTIONS") == 0){
        ret_str = Deal_OPTIONS_Event(http);
    }else if(http->Request_Get_Http_Type().compare("POST") == 0){
        ret_str = Deal_POST_Event(http);
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
    std::string_view url = http->Request_Get_Url();
    std::string ret_str;
    if(url.compare("/api/login") == 0){
        ret_str = Deal_Login_Event(http);
    }else if(url.compare("/api/register") == 0){
        ret_str = Deal_Register_Event(http);
    }else{

    }
    return ret_str;
}

std::string Event_Layer::Deal_Login_Event(HTTP_NSP::HTTP::Ptr http)
{
    std::string body(http->Request_Get_Body());
    std::string ret_http_body;
    Parser_NSP::Parser_Json::Ptr json_ptr = std::make_shared<Parser_NSP::Parser_Json>();
    int code = 0;
    if(json_ptr->Parse(body) == 0){
        std::string username,password;
        json_ptr->Get_Value<std::string>(json_ptr->Get_Root(),"username",username);
        json_ptr->Get_Value<std::string>(json_ptr->Get_Root(),"password",password);
        std::string token(http->Request_Get_Key_Value("Token"));
        //std::cout<<username<<"  "<<password<<" "<<token<<std::endl;
        Login_Context::Ptr ctx = std::make_shared<Login_Context>(http);
        ctx->username = username;
        ctx->password = password;
        ctx->token = token;
        ctx->http = http;
        code = login_api->Function(ctx);
    }else{
        code = 3;
    }
    // 生成json
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Value value;
    value.SetInt(code);
    doc.AddMember("code", value, doc.GetAllocator());
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    body = buffer.GetString();
    return body;
}

std::string Event_Layer::Deal_Register_Event(HTTP_NSP::HTTP::Ptr http)
{
    

    
}
