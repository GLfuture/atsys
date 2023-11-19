#ifndef CONTEXT_H
#define CONTEXT_H

#include "../base/parser/HTTP.hpp"
#include "nlohmann/json.hpp"

class Context_Base
{
public:
    using Ptr = std::shared_ptr<Context_Base>;
    virtual ~Context_Base() {}
};


class Login_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<Login_Context>;
    // enum STATUS{
    //     LOGIN_WITHOUT_TOKEN,
    //     LOGIN_WITH_TOKEN,
    // };
    Login_Context(HTTP_NSP::HTTP::Ptr http,int role,std::string username,std::string password){
        this->http = http;
        this->role = role;
        this->username = username;
        this->password = password;
    }

public:
    int role;
    HTTP_NSP::HTTP::Ptr http;
    std::string username;
    std::string password;
    // STATUS status;
};


class Register_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Register_Context>;
    Register_Context(std::string username,std::string password,std::string email,
    std::string phone,std::string address,int sex){
        this->username = username;
        this->password = password;
        this->sex = sex;
        this->email = email;
        this->phone = phone;
        this->address = address;
    }   
public:
    std::string username;
    std::string password;
    std::string email;
    std::string phone;
    std::string address;
    int sex;
};

class Logout_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Logout_Context>;
    Logout_Context(int role,int id)
    {
        this->role = role;
        this->id = id;
    }

public:
    int role;
    int id;
};

class Card_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Card_Context>;
    Card_Context(int method,std::string cname,int role,int uid,int cid,int num,std::string description)
    {
        this->method = method;
        this->cname = cname;
        this->role = role;
        this->uid = uid;
        this->cid = cid;
        this->num = num;
        this->description = description;
    }
public:
    int method;
    std::string cname;
    int role;
    int uid;
    int cid;
    int num;
    std::string description;
};

class Data_Card_Concrete_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Data_Card_Concrete_Context>;
    Data_Card_Concrete_Context(int role,int uid)
    {
        this->role = role;
        this->uid = uid;
        this->j = nlohmann::json();
    }
public:
    int role;
    int uid;
    nlohmann::json j;
};

class Data_User_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<Data_User_Context>;
    Data_User_Context(int role,int uid,int page)
    {
        this->role = role;
        this->uid = uid;
        this->page = page;
        this->j = nlohmann::json();
    }

public:
    int role;
    int page;
    int uid;
    nlohmann::json j;
};

class Data_Card_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<Data_Card_Context>;
    Data_Card_Context()
    {
        j = nlohmann::json();
    }
public:
    nlohmann::json j;
};

class Data_Time_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<Data_Time_Context>;
    Data_Time_Context(){
        j = nlohmann::json();
    }
public:
    nlohmann::json j;
};


class User_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<User_Context>;
    User_Context(int uid,std::string username,std::string password,std::string email,
    std::string phone,std::string address,int sex)
    {
        this->uid = uid;
        this->username = username;
        this->password = password;
        this->sex = sex;
        this->email = email;
        this->phone = phone;
        this->address = address;
    }
public:
    int uid;
    std::string username;
    std::string password;
    std::string email;
    std::string phone;
    std::string address;
    int sex;
};

class Time_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Time_Context>;
    Time_Context(int role,int uid,int method,time_t settime)
    {
        this->method = method;
        this->uid = uid;
        this->role = role;
        _time = 0;
        this->settime = settime;
    }
public:
    time_t _time;
    time_t settime;
    int method;
    int uid;
    int role;
};


#endif