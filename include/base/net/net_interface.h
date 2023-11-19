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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <variant>



class Net_Interface_Base
{
public:
    using SSL_Tup = std::tuple<SSL*,SSL_CTX*>;

    using Ptr = std::shared_ptr<Net_Interface_Base>;
    virtual int Sock() = 0;

    virtual int Conncet(std::string sip,uint32_t sport) = 0;

    virtual int Bind(int _fd,uint32_t port) = 0;

    virtual int Listen(int _fd,uint32_t backlog) = 0 ;

    virtual int Accept(int _fd) = 0;

    virtual ssize_t Recv(int fd,std::string& buffer,uint32_t len)  = 0;

    virtual ssize_t Send(int fd,const std::string& buffer,uint32_t len) = 0;

    virtual void SSL_Env_Init() = 0;

    virtual SSL_Tup SSL_Init(long min_version,long max_version) = 0;

    virtual void SSL_Set_Fd(SSL* ssl,int fd) = 0;

    virtual int SSL_Connect(SSL* ssl) = 0;

    virtual int SSL_Accept(SSL* ssl) = 0;

    virtual int SSL_Recv(SSL* ssl,std::string& buffer,int len) = 0;

    virtual int SSL_Send(SSL* ssl,const std::string& buffer,int len) = 0;

    virtual void SSL_Destory(SSL* ssl,SSL_CTX* ctx) = 0;

    virtual void SSL_Env_Destory() = 0;

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

    void SSL_Env_Init() override;

    SSL_Tup SSL_Init(long min_version,long max_version) override;

    void SSL_Set_Fd(SSL* ssl,int fd) override;

    int SSL_Accept(SSL* ssl) override;

    int SSL_Connect(SSL* ssl) override;

    int SSL_Recv(SSL* ssl,std::string& buffer,int len) override;

    int SSL_Send(SSL* ssl,const std::string& buffer,int len) override;

    void SSL_Destory(SSL* ssl,SSL_CTX* ctx) override;

    void SSL_Env_Destory() override;

};



#endif