#include "base/net/net_interface.h"
int Net_Interface::Sock()
{
    int _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd <= 0)
    {
        printf("socket fails");
        exit(-1);
    }
    return _fd;
}


int Net_Interface::Conncet(std::string sip, uint32_t sport)
{
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd <= 0) return conn_fd;
    sockaddr_in sin = {0};
    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = inet_addr(sip.c_str());
    sin.sin_family = AF_INET;
    sin.sin_port = htons(sport);
    int ret = connect(conn_fd, (sockaddr *)&sin, sizeof(sin));
    if (ret == -1) return ret;
    return conn_fd;
}

int Net_Interface::Bind(int _fd, uint32_t port)
{
    sockaddr_in sin = {0};
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;
    return bind(_fd, (sockaddr *)&sin, sizeof(sin));
}

int Net_Interface::Listen(int _fd,uint32_t backlog)
{
    return listen(_fd, backlog);
}

int Net_Interface::Accept(int _fd)
{
    sockaddr_in sin = {0};
    memset(&sin, 0, sizeof(sin));
    socklen_t len = sizeof(sin);
    int conn_fd = accept(_fd, (sockaddr *)&sin, &len);
    return conn_fd;
}

ssize_t Net_Interface::Recv(int fd,std::string& buffer,uint32_t len)
{
    char *temp = new char[len];
    memset(temp, 0, len);
    ssize_t ret = recv(fd, temp, len, 0);
    if(ret <= 0) return ret;
    buffer.assign(temp,ret);
    return ret;
}

ssize_t Net_Interface::Send(int fd,const std::string& buffer,uint32_t len)
{
    return send(fd, buffer.c_str() ,len,0);
}

void Net_Interface::SSL_Env_Init()
{
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
}

Net_Interface_Base::SSL_Tup Net_Interface::SSL_Init(long min_version,long max_version)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method()); // 创建客户端 SSL 上下文对象
    SSL_CTX_set_min_proto_version(ctx, min_version); // 最小支持 TLSv1.3 协议
    SSL_CTX_set_max_proto_version(ctx, max_version); // 最大支持 TLSv1.3 协议

    SSL* ssl = SSL_new(ctx);
    SSL_Tup tup = {ssl,ctx};
    return tup;
}

void Net_Interface::SSL_Set_Fd(SSL* ssl,int fd)
{
    SSL_set_fd(ssl,fd);
}

int Net_Interface::SSL_Connect(SSL* ssl)
{
    return SSL_connect(ssl);
}

int Net_Interface::SSL_Recv(SSL* ssl,std::string& buffer,int len)
{
    char* temp = new char[len];
    memset(temp,0,len);
    int ret_len = SSL_read(ssl,temp,len);
    if( ret_len == -1){
        delete temp;
        return ret_len;
    }
    buffer.assign(temp,ret_len);
    delete temp;
    return ret_len;
}


int Net_Interface::SSL_Send(SSL* ssl,const std::string& buffer,int len)
{
    return SSL_write(ssl,buffer.c_str(),len);
}

int Net_Interface::SSL_Accept(SSL* ssl)
{
    return SSL_accept(ssl);
}

void Net_Interface::SSL_Destory(SSL* ssl,SSL_CTX* ctx)
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
}

void Net_Interface::SSL_Env_Destory()
{
    ERR_free_strings();
    EVP_cleanup();
}