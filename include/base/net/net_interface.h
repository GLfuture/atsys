#pragma once
#ifndef NET_INTERFACE_H
#define NET_INTERFACE_H
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <memory>
enum Error_Code
{
    OK = 0,
    SOCKET_ERR = -1,
    CONNECT_ERR = -2,
};



class Net_Interface_Base
{
public:
    using Ptr = std::shared_ptr<Net_Interface_Base>;
    virtual int Sock() = 0;

    virtual int Conncet(std::string sip,uint32_t sport) = 0;

    virtual int Bind(int _fd,uint32_t port) = 0;

    virtual int Listen(int _fd,uint32_t backlog) = 0 ;

    virtual int Accept(int _fd) = 0;

    virtual ssize_t Recv(int fd,std::string& buffer,uint32_t len)  = 0;

    virtual ssize_t Send(int fd,const std::string& buffer,uint32_t len) = 0;

    virtual ~Net_Interface_Base() {}

};

class Net_Interface:public Net_Interface_Base
{
public:
    using Ptr = std::shared_ptr<Net_Interface>;
    int Sock() override;

    int Conncet(std::string sip,uint32_t sport) override;

    int Bind(int _fd,uint32_t port) override;

    int Listen(int _fd,uint32_t backlog) override;

    int Accept(int _fd) override;

    ssize_t Recv(int fd,std::string& buffer,uint32_t len) override;

    ssize_t Send(int fd,const std::string& buffer,uint32_t len) override;

};



#endif