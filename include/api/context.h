#ifndef CONTEXT_H
#define CONTEXT_H

#include "../base/parser/HTTP.hpp"

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
    Login_Context(HTTP_NSP::HTTP::Ptr http){
        username="";
        password="";
        token = "";
        this->http = http;
    }

public:
    std::string username;
    std::string password;
    std::string token;
    HTTP_NSP::HTTP::Ptr http;
};

#endif