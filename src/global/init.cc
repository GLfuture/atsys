#include "global/init.h"

static std::mutex mtx;

int init_conf(const std::string& filename)
{
    Configer_NSP::Config_Configer::Ptr config_ptr = std::make_shared<Configer_NSP::Config_Configer>();
    int ret = config_ptr->Load_Conf(filename);
    Configer_NSP::Config_Configer::Config_Parser_Ptr parser = config_ptr->Get_Parser();
    std::string temp;
    parser->Get_Value("port",temp);
    if(temp.empty()){
        return -1;
    }else{
        global_port = atoi(temp.c_str());
        temp.clear();
    }
    parser->Get_Value("worker_num",temp);
    if(temp.empty()){
        return -1;
    }else{
        global_worker_num = atoi(temp.c_str());
        temp.clear();
    }
    parser->Get_Value("event_num",temp);
    if(temp.empty()){
        return -1;
    }else{
        global_event_num = atoi(temp.c_str());
        temp.clear();
    }
    parser->Get_Value("backlog",temp);
    if(temp.empty()){
        return -1;
    }else{
        global_backlog = atoi(temp.c_str());
        temp.clear();
    }
    parser->Get_Value("extension",temp);
    if(temp.empty()){
        return -1;
    }else{
        global_extension = atoi(temp.c_str());
        temp.clear();
    }
    parser->Get_Value("max_http_head_len",temp);
    std::cout<<temp<<std::endl;
    if(temp.empty()){
        return -1;
    }else{
        global_max_http_head_len = atoi(temp.c_str());
        temp.clear();
    }
    return 0;
}


void init_callback(App::Ptr app)
{
    global_accept_cb = std::bind(Accept_cb,std::placeholders::_1,app->Get_Net_Layer());
    global_read_cb   = std::bind(Read_cb,std::placeholders::_1,app);
    global_write_cb  = std::bind(Write_cb,std::placeholders::_1,app->Get_Net_Layer());
    global_exit_cb   = std::bind(Exit_cb,std::placeholders::_1,app->Get_Net_Layer());
}




void Accept_cb(int workerid,Net_Layer::Ptr net_layer)
{
    std::unique_lock lock(mtx);
    Server_Base::Ptr worker = net_layer->Get_Server(workerid);
    Reactor::Ptr R = net_layer->Get_Reactor(workerid);
    int connfd = worker->Accept();
    if(connfd <= 0) return ;
    Tcp_Conn_Base::Ptr conn_ptr = std::make_shared<Tcp_Conn>(connfd);
    R->Add_Reactor(worker->Get_Epoll_Fd(),connfd,EPOLLIN);
    worker->Add_Conn(conn_ptr);
}


void Read_cb(int workerid,App::Ptr app_ptr)
{
    Net_Layer::Ptr net_layer = app_ptr->Get_Net_Layer();
    Event_Layer::Ptr event_layer = app_ptr->Get_Event_Layer();
    Server_Base::Ptr worker = net_layer->Get_Server(workerid);
    Reactor::Ptr R = net_layer->Get_Reactor(workerid);
    uint32_t cfd = R->Get_Now_Event().data.fd;
    Tcp_Conn_Base::Ptr cptr = worker->Get_Conn(cfd);
    int len = worker->Recv(cptr,global_max_http_head_len);
    std::cout<<global_max_http_head_len<<std::endl;
    std::cout<<len<<std::endl;
    if(len <= 0){
        worker->callback.Exit_cb();
    }
    std::string buffer(cptr->Get_Rbuffer());
    std::cout<<buffer<<std::endl;
    HTTP_NSP::HTTP::Ptr http = std::make_shared<HTTP_NSP::HTTP>();
    int ret_len = http->Parse(buffer);
    std::string ret_body;
    if(ret_len > 0){
        http->Response_Set_Status(200);
        ret_body = event_layer->Deal_Event(http);
        http->Response_Set_Key_Value("Connection","close");
        cptr->Erase_Rbuffer(ret_len);
    }else if(ret_len == HTTP_NSP::HTTP::PROTO_ERROR){
        http->Response_Set_Status(400);
        http->Response_Set_Key_Value("Connection","close");
    }else if(ret_len == HTTP_NSP::HTTP::INCOMPLETE){
        return ;
    }
    std::string ret_msg = http->Content_Head() + ret_body;
    cptr->Appand_Wbuffer(ret_msg);
}

void Write_cb(int workerid,Net_Layer::Ptr net_layer)
{
    Server_Base::Ptr worker = net_layer->Get_Server(workerid);
    Reactor::Ptr R = net_layer->Get_Reactor(workerid);
    uint32_t cfd = R->Get_Now_Event().data.fd;
    Tcp_Conn_Base::Ptr cptr = worker->Get_Conn(cfd);
}

void Exit_cb(int workerid,Net_Layer::Ptr net_layer)
{
    Server_Base::Ptr worker = net_layer->Get_Server(workerid);
    Reactor::Ptr R = net_layer->Get_Reactor(workerid);
    int cfd = R->Get_Now_Event().data.fd;
    R->Del_Reactor(worker->Get_Epoll_Fd(),cfd,EPOLLIN);
    worker->Close(cfd);
}
