/*
 * @Description: 
 * @Version: 1.0
 * @Author: Gong
 * @Date: 2023-09-30 11:59:38
 * @LastEditors: Gong
 * @LastEditTime: 2023-10-03 12:59:03
 */

#include "base/net/server.h"

int Server_Base::Start_Timer()
{
    return timermanager.Create_Timerfd();
}

Server_Base::Server_Base(int fd, Net_Interface_Base::Ptr &interface)
{
    this->_fd = fd;
    this->interface = interface;
    this->_ssl_tup = {nullptr,nullptr};
}

int Server_Base::Init_Epoll_Fd()
{
    epfd = epoll_create(1);
    return epfd;
}

void Server_Base::SSL_Env_Init()
{
    interface->SSL_Env_Init();
}

Net_Interface_Base::SSL_Tup Server_Base::SSL_Init(long min_version,long max_version)
{
    return interface->SSL_Init(min_version,max_version);
}

void Server_Base::SSL_Set_Fd(SSL* ssl,int fd)
{
    interface->SSL_Set_Fd(ssl,fd);
}

void Server_Base::SSL_Destory(Net_Interface_Base::SSL_Tup ssl_tup)
{
    interface->SSL_Destory(std::get<SSL*>(ssl_tup),std::get<SSL_CTX*>(ssl_tup));
}

void Server_Base::SSL_Env_Destory()
{
    interface->SSL_Env_Destory();
}

int Server_Base::Accept()
{
    return  interface->Accept(_fd);
}

int Server_Base::SSL_Accept(SSL* ssl)
{
    return interface->SSL_Accept(ssl);
}

ssize_t Server_Base::Recv(Tcp_Conn_Base::Ptr &conn_ptr, uint32_t len)
{
    std::string buffer="";
    ssize_t ret = interface->Recv(conn_ptr->Get_Conn_fd(),buffer,len);
    if (ret <= 0)
        return ret; 
    conn_ptr->Appand_Rbuffer(buffer);
    return ret;
}

ssize_t Server_Base::SSL_Recv(SSL* ssl, Tcp_Conn_Base::Ptr& conn_ptr,uint32_t len)
{
    std::string buffer;
    int ret_len = interface->SSL_Recv(ssl,buffer,len);
    if(ret_len == -1) return ret_len;
    conn_ptr->Appand_Rbuffer(buffer);
    return ret_len;
}

ssize_t Server_Base::Send(const Tcp_Conn_Base::Ptr &conn_ptr, uint32_t len)
{
    return interface->Send(conn_ptr->Get_Conn_fd(),std::string(conn_ptr->Get_Wbuffer()),len);
}

ssize_t Server_Base::SSL_Send(SSL* ssl, const Tcp_Conn_Base::Ptr& conn_ptr,uint32_t len)
{
    return interface->SSL_Send(ssl,std::string(conn_ptr->Get_Wbuffer()),len);
}

Tcp_Conn_Base::Ptr Server_Base::Connect(std::string sip,uint32_t sport)
{
    int fd = interface->Conncet(sip,sport);
    if(fd <= 0) return nullptr;
    Tcp_Conn_Base::Ptr conn = std::make_shared<Tcp_Conn_Base>(fd);
    return conn;
}

int Server_Base::SSL_Connect(SSL *ssl)
{
    return interface->SSL_Connect(ssl);
}

Tcp_Conn_Base::Ptr Server_Base::Get_Conn(int fd)
{
    
    map<uint32_t,Tcp_Conn_Base::Ptr>::iterator it = connections.find(fd);
    if(it == connections.end()) return nullptr;
    return it->second;
}

void Server_Base::Add_Conn(const Tcp_Conn_Base::Ptr &conn_ptr)
{
    std::lock_guard lock(this->mtx);
    this->connections[conn_ptr->Get_Conn_fd()] = conn_ptr;
}


int Server_Base::Close(int fd)
{
    return close(fd);
}

void Server_Base::Clean_Conns()
{
    for (map<uint32_t, Tcp_Conn_Base::Ptr>::iterator it = connections.begin(); it != connections.end(); it++)
    {
        Close((*it).first);
        it = Del_Conn((*it).first);
        if(it != connections.end()){
            it--;
        }
    }
}


map<uint32_t, Tcp_Conn_Base::Ptr>::iterator Server_Base::Del_Conn(int fd)
{
    std::lock_guard lock(this->mtx);
    map<uint32_t,Tcp_Conn_Base::Ptr>::iterator it = connections.find(fd);
    if(it == connections.end()) return it;
    return connections.erase(it);
}
