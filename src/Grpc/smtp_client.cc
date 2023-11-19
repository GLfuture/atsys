#include <grpcpp/grpcpp.h>
#include "Grpc/smtp.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using SMTP::Captcha::SMTP_Captcha_Req;
using SMTP::Captcha::SMTP_Captcha_Rsp;
using SMTP::Captcha::Send_Captcha;

class SendCaptchaImplClient
{
public:
    SendCaptchaImplClient(std::shared_ptr<Channel> channel):
        stub(Send_Captcha::NewStub(channel)){}

    int Send_Captcha(std::vector<std::string> emails,std::string captcha)
    {
        SMTP_Captcha_Req request;
        for(int i=0;i<emails.size();i++)
        {
            request.add_emails(emails[i]);
        }
        request.set_captcha(captcha);

        SMTP_Captcha_Rsp response;
        ClientContext context;
        Status status = stub->SendCaptcha(&context,request,&response);
        if(status.ok()){
            std::cout<<response.code()<<std::endl;
        }else{
            std::cout<<"failed"<<std::endl;
        }
        return 0;
    }

private:
    std::unique_ptr<SMTP::Captcha::Send_Captcha::Stub> stub;
};

int main()
{
    std::string saddr = "127.0.0.1:5555";
    SendCaptchaImplClient client(grpc::CreateChannel(saddr,grpc::InsecureChannelCredentials()));
    std::vector<std::string> arr = {"1790188287@qq.com"};
    std::string code = "23123123";
    std::cout << client.Send_Captcha(arr,code);

    return 0;
}