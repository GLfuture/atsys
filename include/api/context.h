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

class Card_New_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<Card_New_Context>;
    Card_New_Context(std::string cname){
        this->cname = cname;
    }
public:
    std::string cname;
};

class Card_Del_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Card_Del_Context>;
    Card_Del_Context(int role,int cid,int uid = 0){
        this->role = role;
        this->cid = cid;
        this->uid = uid;
    }
public:
    int role;
    int uid;
    int cid;
};

class Card_Upd_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Card_Upd_Context>;
    Card_Upd_Context(int uid,int cid,int num){
        this->cid = cid;
        this->num = num;
        this->uid = uid;
    }
public:
    int uid;
    int cid;
    int num;
};

class Data_Card_Context: public Context_Base
{
public:
    using Ptr = std::shared_ptr<Data_Card_Context>;
    Data_Card_Context(int role,int uid)
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

class User_Upd_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<User_Upd_Context>;
    User_Upd_Context(int uid,std::string username,std::string password,std::string email,
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
    Time_Context(int role,int uid,int method)
    {
        this->method = method;
        this->uid = uid;
        this->role = role;
        _time = 0;
    }
public:
    time_t _time;
    int method;
    int uid;
    int role;
};


#endif