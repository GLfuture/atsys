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
    std::string_view url = http->Request_Get_Url();
    std::string ret_str;
    if(url.compare("/api/login") == 0){
        ret_str = Deal_Login_Event(http);
    }else if(url.compare("/api/register") == 0){
        ret_str = Deal_Register_Event(http);
    }else{
        std::string token(http->Request_Get_Key_Value("Token"));
        if(token.empty()){
            ret_str = Deal_No_Token_Event();
        }else{
            jwt::jwt_object obj;
            if (Token_Analyzer::Decode_Token(token, obj).value() == static_cast<int>(jwt::VerificationErrc::TokenExpired))
            {
                ret_str = Deal_Token_Expire_Event();
            }
            else
            {
                if(url.compare("/api/card") == 0){
                    ret_str = Deal_Card_Event(http,obj);
                }
                else if(url.compare("/api/logout") == 0)
                {
                    ret_str = Deal_Logout_Event(http,obj);
                    //置token过期
                    obj.remove_claim("exp");
                    obj.add_claim("exp", std::chrono::system_clock::now() - std::chrono::seconds(TOKEN_EXPIRE_TIME));
                    token = obj.signature();
                    http->Response_Set_Key_Value("Token", token);
                    return ret_str;
                }
                else if(url.compare("/api/user")==0)
                {
                    ret_str = Deal_User_Event(http,obj);
                }
                else if(url.compare("/api/time") == 0)
                {
                    ret_str = Deal_Time_Event(http,obj);
                }
                else{
                    ret_str = Deal_Incorrect_Api_Event();
                }
                obj.remove_claim("exp");
                obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
                token = obj.signature();
                http->Response_Set_Key_Value("Token", token);

            }
        }

    }
    return ret_str;
}

std::string Event_Layer::Deal_GET_Event(HTTP_NSP::HTTP::Ptr http)
{
    std::string ret_str;
    std::string_view url = http->Request_Get_Url();
    std::string token(http->Request_Get_Key_Value("Token"));
    if(token.empty()){
        ret_str = Deal_No_Token_Event();
    }else {
        jwt::jwt_object obj;
        if (Token_Analyzer::Decode_Token(token, obj).value() == static_cast<int>(jwt::VerificationErrc::TokenExpired))
        {
            ret_str = Deal_Token_Expire_Event();
        }
        else
        {
            if(url.compare("/api/data/card/self") == 0){
                ret_str =  Deal_Data_Card_Self_Event(http,obj);
            }else if(url.compare("/api/data/user") == 0){
                ret_str = Deal_Data_User_Event(http,obj);
            }else if(url.compare("/api/data/time") == 0){
                ret_str = Deal_Data_Time_Event(http,obj);
            }else if(url.compare("/api/data/card") == 0){
                ret_str = Deal_Data_Card_Event(http,obj);
            }
            else{
                ret_str = Deal_Incorrect_Api_Event();
            }

            obj.remove_claim("exp");
            obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
            token = obj.signature();
            http->Response_Set_Key_Value("Token", token);
        }
    }
    
    return ret_str;
}

std::string Event_Layer::Deal_No_Token_Event()
{
    json j;
    j["code"] = STATUS_TOKEN_NOT_IN_HEAD;
    return j.dump();
}

std::string Event_Layer::Deal_Token_Expire_Event()
{
    json j;
    j["code"] = STATUS_TOKEN_EXPIRED;
    return j.dump();
}

std::string Event_Layer::Deal_Incorrect_Api_Event()
{
    json j;
    j["code"] = STATUS_METHOD_ERROR;
    return j.dump();
}

std::string Event_Layer::Deal_Login_Event(HTTP_NSP::HTTP::Ptr http)
{
    std::string body(http->Request_Get_Body());
    std::string token(http->Request_Get_Key_Value("Token"));
    std::string ret_http_body;
    jwt::jwt_object obj;
    int code = 0;
    if (json::accept(body))
    {
        int role;
        json j = json::parse(body);
        if (j.contains("role") && j["role"].is_number())
            role = j["role"];
        if (role == SIMPLE)
        {
            if (!token.empty() && Token_Analyzer::Decode_Token(token, obj).value() != static_cast<int>(jwt::VerificationErrc::TokenExpired) && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0)
            {
                obj.remove_claim("exp");
                obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
                token = obj.signature();
                http->Response_Set_Key_Value("Token", token);
                code = STATUS_LOGIN_SUCCESS;
            }
            else
            {
                std::string username, password;
                if (j.contains("username") && j["username"].is_string()) username = j["username"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                if (j.contains("password") && j["password"].is_string()) password = j["password"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                Login_Context::Ptr ctx = std::make_shared<Login_Context>(http, role, username, password);
                code = api_manager->Get_API(LOGIN_API)->Function(ctx);
            }
        }
        else if (role == MANAGER)
        {
            if (!token.empty() && Token_Analyzer::Decode_Token(token, obj).value() != static_cast<int>(jwt::VerificationErrc::TokenExpired) && obj.payload().get_claim_value<std::string>("role") == "man")
            {
                obj.remove_claim("exp");
                obj.add_claim("exp", std::chrono::system_clock::now() + std::chrono::seconds(TOKEN_EXPIRE_TIME));
                token = obj.signature();
                http->Response_Set_Key_Value("Token", token);
                code = STATUS_LOGIN_SUCCESS;
            }
            else
            {
                std::string username, password;
                if (j.contains("username") && j["username"].is_string()) username = j["username"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                if (j.contains("password") && j["password"].is_string()) password = j["password"];
                else {
                    code = STATUS_JSON_NO_NEC_MEM;
                    goto end;
                }
                Login_Context::Ptr ctx = std::make_shared<Login_Context>(http, role, username, password);
                code = api_manager->Get_API(LOGIN_API) ->Function(ctx);
            }
        }
    }
    else{
        code = STATUS_JSON_ERROR;
    }
    // 生成json
end:
    json j;
    j["code"] = code;
    return j.dump();
}

std::string Event_Layer::Deal_Logout_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{
    int code = 0;
    if(obj.payload().has_claim("role")&&obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
    {
        int mid = atoi(obj.payload().get_claim_value<std::string>("mid").c_str());
        Logout_Context::Ptr logout_ctx = std::make_shared<Logout_Context>(MANAGER,mid);
        code = api_manager->Get_API(LOGOUT_API)->Function(logout_ctx);
    }
    else if(obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
    {
        int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
        Logout_Context::Ptr logout_ctx = std::make_shared<Logout_Context>(SIMPLE,uid);
        code = api_manager->Get_API(LOGOUT_API)->Function(logout_ctx);
    }
    json j;
    j["code"] = code;
    return j.dump();
}

std::string Event_Layer::Deal_Register_Event(HTTP_NSP::HTTP::Ptr http)
{
    int code = 0;
    std::string body(http->Request_Get_Body());
    if (json::accept(body))
    {
        json j = json::parse(body);
        std::string username, password, email, phone, address;
        int sex = -1;
        if (j.contains("username") && j["username"].is_string())
            username = j["username"];
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }
        if (j.contains("password") && j["password"].is_string())
            password = j["password"];
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }
        if (j.contains("email") && j["email"].is_string())
            email = j["email"];
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }
        if (j.contains("phone") && j["phone"].is_string())
            phone = j["phone"];
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }
        if (j.contains("address") && j["address"].is_string())
            address = j["address"];
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }
        if (j.contains("sex") && j["sex"].is_number())
            sex = j["sex"];
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }
        Register_Context::Ptr register_ctx = std::make_shared<Register_Context>(username, password, email, phone, address,sex);
        code = api_manager->Get_API(REGISTER_API) ->Function(register_ctx);
    }else{
        code = STATUS_JSON_ERROR;
    }

end:
    json j;
    j["code"] = code;
    return j.dump();
}


std::string Event_Layer::Deal_Card_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{
    int code = 0;
    std::string body(http->Request_Get_Body());
    int method,cid,uid;
    if(json::accept(body)){
        json j = json::parse(body);
        if(j.contains("method")&&j["method"].is_number()){
            method = j["method"];
            switch (method)
            {
            case OP_NEW_CARD:
            {
                if(obj.payload().has_claim("role") &&obj.payload().get_claim_value<std::string>("role").compare("man") == 0&&obj.payload().has_claim("mid")){
                    if(j.contains("cname")&&j["cname"].is_string()){
                        std::string cname = j["cname"];
                        Card_New_Context::Ptr ctx = std::make_shared<Card_New_Context>(cname);
                        code = api_manager->Get_API(CARD_NEW_API) ->Function(ctx);
                    }else{
                        code = STATUS_JSON_NO_NEC_MEM;
                    }
                }else{
                    code = STATUS_PRIVILIDGE_ERROR;
                }
                break;
            }
            case OP_DEL_CARD:
            {
                if (j.contains("cid")&&j["cid"].is_number())
                {
                    cid = j["cid"];
                    if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
                    { // manager
                        Card_Del_Context::Ptr card_del_ctx = std::make_shared<Card_Del_Context>(MANAGER,cid);
                        code = api_manager->Get_API(CARD_DEL_API)->Function(card_del_ctx);
                    }
                    else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
                    { // simple user
                        uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                        Card_Del_Context::Ptr card_del_ctx = std::make_shared<Card_Del_Context>(SIMPLE,cid,uid);
                        code = api_manager->Get_API(CARD_DEL_API)->Function(card_del_ctx);
                    }
                    else
                    {
                        code = STATUS_PRIVILIDGE_ERROR;
                    }
                }
                else
                {
                    code = STATUS_JSON_NO_NEC_MEM;
                }
                
                break;
            }
            case OP_UPD_CARD:
            {
                if (j.contains("cid")&&j["cid"].is_number()&&j.contains("num")&&j["num"].is_number())
                {
                    cid = j["cid"];
                    int num = j["num"];
                    if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
                    { // manager
                        if(j.contains("uid")&&j["uid"].is_number()){
                            uid = j["uid"];
                            Card_Upd_Context::Ptr card_upd_ctx = std::make_shared<Card_Upd_Context>(uid,cid,num);
                            code = api_manager->Get_API(CARD_UPD_API)->Function(card_upd_ctx);    
                        }else{
                            code = STATUS_JSON_NO_NEC_MEM;
                        }
                    }
                    else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
                    { // simple user
                        uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
                        Card_Upd_Context::Ptr card_upd_ctx = std::make_shared<Card_Upd_Context>(uid,cid,num);
                        code = api_manager->Get_API(CARD_UPD_API)->Function(card_upd_ctx);
                    }
                    else
                    {
                        code = STATUS_PRIVILIDGE_ERROR;
                    }
                }
                else
                {
                    code = STATUS_JSON_NO_NEC_MEM;
                }
                break;
            }
            default:
                code = STATUS_METHOD_ERROR;
                break;
            }
        }
        else
        {
            code = STATUS_JSON_NO_NEC_MEM;
            goto end;
        }

    }else{
        code = STATUS_JSON_ERROR;
    }
end:
    json j;
    j["code"] = code;
    return j.dump();
}

std::string Event_Layer::Deal_Data_Card_Self_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{
    if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
    { // manager
        Data_Card_Self_Context::Ptr data_card_self_ctx = std::make_shared<Data_Card_Self_Context>(MANAGER,0);
        api_manager->Get_API(DATA_CARD_Self_API)->Function(data_card_self_ctx);
        return data_card_self_ctx->j.dump();
    }
    else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
    { // simple user
        int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
        Data_Card_Self_Context::Ptr data_card_self_ctx = std::make_shared<Data_Card_Self_Context>(SIMPLE, uid);
        api_manager->Get_API(DATA_CARD_Self_API)->Function(data_card_self_ctx);
        return data_card_self_ctx->j.dump();
    }
    return "null";
}

std::string Event_Layer::Deal_Data_User_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{
    if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
    { // manager
        int page = atoi(http->Request_Get_Arg_Value("page").cbegin());
        Data_User_Context::Ptr data_user_ctx = std::make_shared<Data_User_Context>(MANAGER,0,page);
        api_manager->Get_API(DATA_USER_API)->Function(data_user_ctx);
        return data_user_ctx->j.dump();
    }
    else if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
    { // simple user
        int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
        Data_User_Context::Ptr data_user_ctx = std::make_shared<Data_User_Context>(SIMPLE, uid ,0);
        api_manager->Get_API(DATA_USER_API)->Function(data_user_ctx);
        return data_user_ctx->j.dump();
    }
    return "null";
}

std::string Event_Layer::Deal_Data_Card_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{
    Data_Card_Context::Ptr data_card_ctx = std::make_shared<Data_Card_Context>();
    if (obj.payload().has_claim("role"))
    {
        api_manager->Get_API(DATA_CARD_API)->Function(data_card_ctx);
    }
    return data_card_ctx->j.dump();
}


std::string Event_Layer::Deal_User_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{
    int code = 0;
    if (obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
    {
        int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
        std::string body(http->Request_Get_Body());
        if (json::accept(body))
        {
            json j = json::parse(body);
            std::string username, password, email, phone, address;
            int sex = -1;
            if (j.contains("username") && j["username"].is_string())
                username = j["username"];
            if (j.contains("password") && j["password"].is_string())
                password = j["password"];
            if (j.contains("email") && j["email"].is_string())
                email = j["email"];
            if (j.contains("phone") && j["phone"].is_string())
                phone = j["phone"];
            if (j.contains("address") && j["address"].is_string())
                address = j["address"];
            if (j.contains("sex") && j["sex"].is_number())
                sex = j["sex"];
            User_Upd_Context::Ptr user_upd_ctx = std::make_shared<User_Upd_Context>(uid,username, password, email, phone, address, sex);
            code = api_manager->Get_API(USER_UPD_API)->Function(user_upd_ctx);
        }
        else
        {
            code = STATUS_JSON_ERROR;
        }
    }
    else
    {
        code = STATUS_PRIVILIDGE_ERROR;
    }

    json j;
    j["code"] = code;
    return j.dump();
}

std::string Event_Layer::Deal_Time_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{
    int code = 0;
    json ret_j;
    if(obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("sim") == 0 && obj.payload().has_claim("uid"))
    {
        int uid = atoi(obj.payload().get_claim_value<std::string>("uid").c_str());
        std::string body(http->Request_Get_Body());
        if (json::accept(body))
        {
            json j = json::parse(body);
            int method;
            if (j.contains("method") && j["method"].is_number())
                method = j["method"];
            else
            {
                code = STATUS_JSON_NO_NEC_MEM;
                goto end;
            }
            Time_Context::Ptr time_ctx = std::make_shared<Time_Context>(SIMPLE,uid,method,0);
            code = api_manager->Get_API(TIME_API)->Function(time_ctx);
            if(time_ctx->_time != 0){
                ret_j["time"] = time_ctx->_time;
            }
        }
        else
        {
            code = STATUS_JSON_ERROR;
        }
    }else if(obj.payload().has_claim("role") && obj.payload().get_claim_value<std::string>("role").compare("man") == 0 && obj.payload().has_claim("mid"))
    {
        std::string body(http->Request_Get_Body());
        if (json::accept(body))
        {
            json j = json::parse(body);
            int method;
            time_t time;
            if (j.contains("method") && j["method"].is_number())
                method = j["method"];
            else
            {
                code = STATUS_JSON_NO_NEC_MEM;
                goto end;
            }
            if (j.contains("time") && j["time"].is_number())
                time = j["time"];
            else
            {
                code = STATUS_JSON_NO_NEC_MEM;
                goto end;
            }
            Time_Context::Ptr time_ctx = std::make_shared<Time_Context>(MANAGER, 0,method,time);
            code = api_manager->Get_API(TIME_API)->Function(time_ctx);
        }
    }
    else{
        code = STATUS_PRIVILIDGE_ERROR;
    }


end:  
    ret_j["code"] = code;
    return ret_j.dump();
}

std::string Event_Layer::Deal_Data_Time_Event(HTTP_NSP::HTTP::Ptr http,const jwt::jwt_object& obj)
{

    Data_Time_Context::Ptr data_time_ctx = std::make_shared<Data_Time_Context>();
    if(obj.payload().has_claim("role") )
    {
        api_manager->Get_API(DATA_TIME_API)->Function(data_time_ctx);
    }
    return data_time_ctx->j.dump();
}