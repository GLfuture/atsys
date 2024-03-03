#include "base/net/conn.h"

Tcp_Conn_Base::Tcp_Conn_Base(uint32_t conn_fd)
{
    _fd = conn_fd;
    ssl_tup = {nullptr,nullptr};
}

void Tcp_Conn_Base::Erase_Rbuffer(int len)
{
    string::iterator it = this->_rbuffer.begin();
    this->_rbuffer.erase(0,len);
}


void Tcp_Conn_Base::Erase_Wbuffer(int len)
{
    string::iterator it = this->_wbuffer.begin();
    this->_wbuffer.erase(0, len);
}