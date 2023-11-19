#include <iostream>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "Grpc/smtp.pb.h"
#include "Grpc/smtp.grpc.pb.h"
#include "Grpc/base64.h"
#include "base/net/net_interface.h"
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <fstream>

#define SMTP_PASSWORD           "EPOXVZMINXCXHXUO"
#define SMTP_EMAIL              "bigdata_C1004@163.com"
#define SMTP_DOMAIN_NAME        "smtp.163.com"
#define SMTP_PORT               465
#define PID_FILE_PATH           "../smtp_server.pid"    
#define LOG_FILE_PATH           "../log/smtp.log"
#define MAX_LOG_SIZE            10*1024*1024
#define MAX_LOG_FILE_NUM        1
#define LOG_LEVEL               spdlog::level::info
#define MAX_RECV_BUFFER_SIZE    1024


using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using SMTP::Email_Msg::SMTP_Req;
using SMTP::Email_Msg::SMTP_Rsp;
using SMTP::Email_Msg::Send_Email_Msg;

class SendEmailMsgImpl final : public Send_Email_Msg::Service
{
public:
    SendEmailMsgImpl(Net_Interface_Base::Ptr interface,int sock)
    {
        this->_interface = interface;
        this->sock = sock;
    }
    virtual Status Send_Msg(ServerContext* context,const SMTP_Req* request,SMTP_Rsp* response) override
    {
        std::vector<std::string> emails;
        for(auto it = request->emails().cbegin();it!=request->emails().end();it++){
            emails.push_back(*it);
        }
        response->set_code(Send_Msg_To_SMTP_Server(emails,request->content(),request->subject(),request->content_type(),request->charset()));
        return Status::OK;
    }

private:
    int Exchange_Msg(SSL* ssl,std::string msg)
    {
        int slen = _interface->SSL_Send(ssl,msg,msg.length());
        spdlog::info("ssl_send msg :{}",msg);
        if(slen == -1){
            spdlog::error("{} {} : ssl_send {},{}",__FILE__, __LINE__ ,errno, strerror(errno));
            return -1;
        }
        std::string rmsg;
        int rlen = _interface->SSL_Recv(ssl, rmsg, MAX_RECV_BUFFER_SIZE);
        spdlog::info("ssl_recv msg :{}",rmsg);
        if(rlen <= 0){
            spdlog::error("{}:{}  {},{}",__FILE__, __LINE__ ,errno, strerror(errno));
            return -1;
        }
        return 0;
    }

    int ping(SSL* ssl)
    {
        return Exchange_Msg(ssl,"HELO MSG\r\n");
    }

    int identify(SSL* ssl, std::string email, std::string password)
    {
        std::string msg = "AUTH LOGIN\r\n";
        if(Exchange_Msg(ssl,msg)==-1) return -1;
        msg.clear();
        msg = base64_encode(email) + "\r\n";
        if(Exchange_Msg(ssl, msg)==-1) return -1;
        msg.clear();
        msg = base64_encode(password) + "\r\n";
        if(Exchange_Msg(ssl, msg)==-1) return -1;
        return 0;
    }

    int send_content(SSL* ssl, std::string email, std::vector<std::string> recivers, std::string content,
        std::string subject,std::string content_type,std::string charset)
    {
        std::string msg = "MAIL FROM: <";
        msg = msg + email + ">\r\n";
        Exchange_Msg(ssl,msg);
        msg.clear();
        for (int i = 0; i < recivers.size(); i++)
        {
            msg = "RCPT TO: <" + recivers[i] + ">\r\n";
            Exchange_Msg(ssl,msg);
            msg.clear();
        }
        msg = "DATA\r\n";
        Exchange_Msg(ssl,msg);
        msg =  "From: <" ;
        msg = msg + email + ">\r\nTo: ";
        std::string recvs = "";
        for (int i = 0; i < recivers.size(); i++)
        {
            recvs = recvs + "<" + recivers[i] + ">";
            if (i != recivers.size() - 1)
            {
                recvs += ", ";
            }
        }
        msg = msg + recvs + "\r\n";
        msg = msg +"Subject: " + subject +"\r\n"+"Content-Type: "+content_type+";"+"charset="+
            charset+"\r\n\r\n" + content + "\r\n\r\n\r\n.\r\n\r\n";
        Exchange_Msg(ssl, msg);
        msg.clear();
        Exchange_Msg(ssl,"QUIT\r\n\r\n");
        return 0;
    }

    int Send_Msg_To_SMTP_Server(std::vector<std::string> emails,std::string content,
        std::string subject,std::string content_type,std::string charset)
    {
        
        Net_Interface::SSL_Tup tuple = _interface->SSL_Init(TLS1_3_VERSION,TLS1_3_VERSION);
        hostent *host = gethostbyname(SMTP_DOMAIN_NAME);
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, host->h_addr_list[0], ip, INET_ADDRSTRLEN);
        int connfd = _interface->Conncet(ip,SMTP_PORT);
        if(connfd == -1)    return -1;
        _interface->SSL_Set_Fd(std::get<SSL*>(tuple),connfd);
        _interface->SSL_Connect(std::get<SSL*>(tuple));
        if(ping(std::get<SSL*>(tuple))==-1) return -1;
        if(identify(std::get<SSL*>(tuple),SMTP_EMAIL,SMTP_PASSWORD)==-1) return -1;
        if(send_content(std::get<SSL*>(tuple),SMTP_EMAIL,emails,content,subject,content_type,charset)==-1) return -1; 
        _interface->SSL_Destory(std::get<SSL*>(tuple),std::get<SSL_CTX*>(tuple));
        close(connfd);
        return 0;
    }


private:
    int sock;
    Net_Interface_Base::Ptr _interface;
};


int main()
{
    pid_t pid = fork();
    if(pid < 0){
        
    }
    else if(pid == 0)
    {
        pid = setsid();
        if(pid == -1) std::cout<<"error"<<std::endl;
        std::ofstream stream(PID_FILE_PATH);
        if (stream.fail())
        {
            std::cout << "can not open pid file" << std::endl;
        }
        std::string pid = std::to_string(getpid());
        stream.write(pid.c_str(), pid.length());
        stream.close();
        auto m_logger = spdlog::rotating_logger_mt("global_logger", LOG_FILE_PATH, MAX_LOG_SIZE, 5);
        m_logger->set_level(LOG_LEVEL);
        spdlog::set_default_logger(m_logger);
        Net_Interface::Ptr interface = std::make_shared<Net_Interface>();
        interface->SSL_Env_Init();
        int sock = interface->Sock();
        SendEmailMsgImpl service(interface,sock);
        ServerBuilder builder;
        builder.AddListeningPort("127.0.0.1:5555", grpc::InsecureServerCredentials());
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 5000);
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
        builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
        builder.RegisterService(&service);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        server->Wait();
        interface->SSL_Env_Destory();
    }
    return 0;
}