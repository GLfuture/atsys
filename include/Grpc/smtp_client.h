#ifndef SMTP_CLIENT_H
#define SMTP_CLINET_H

#include <spdlog/spdlog.h>
#include <grpcpp/grpcpp.h>
#include "smtp.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using SMTP::Email_Msg::SMTP_Req;
using SMTP::Email_Msg::SMTP_Rsp;
using SMTP::Email_Msg::Send_Email_Msg;

class SendEmailMsgImplClient
{
public:
    using Ptr = std::shared_ptr<SendEmailMsgImplClient>;
    SendEmailMsgImplClient(std::shared_ptr<Channel> channel):
        stub(Send_Email_Msg::NewStub(channel)){}

    int Send_Msg(std::vector<std::string> emails,std::string content,
        std::string subject,std::string content_type,std::string charset)
    {
        SMTP_Req request;
        for(int i=0;i<emails.size();i++)
        {
            request.add_emails(emails[i]);
        }
        request.set_content(content);
        request.set_content_type(content_type);
        request.set_subject(subject);
        request.set_charset(charset);
        SMTP_Rsp response;
        ClientContext context;
        Status status = stub->Send_Msg(&context,request,&response);
        if(status.ok()){
            return response.code();
        }else{
            spdlog::info("send_captcha :{}",status.error_message());
            return -1;
        }
        return 0;
    }

private:
    std::unique_ptr<Send_Email_Msg::Stub> stub;
};


#endif