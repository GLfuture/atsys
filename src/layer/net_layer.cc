#include "layer/net_layer.h"

Net_Layer::Net_Layer(int worker_num)
{
    reactors.assign(worker_num,nullptr);
    workers.assign(worker_num,nullptr);
    threads.assign(worker_num,nullptr);
}


int Net_Layer::start(uint32_t port,uint32_t backlog,uint16_t event_num)
{
    Net_Interface_Base::Ptr interface = std::make_shared<Net_Interface>();
    int fd = interface->Sock();
    if(fd <= 0){
        spdlog::error("File:{} Line:{}  sock fail",__FILE__,__LINE__);
        return -1;
    }
    int ret = interface->Bind(fd,port);
    if(ret == -1){
        spdlog::error("File:{} Line:{} bind file",__FILE__,__LINE__);
        return ret;
    }
    ret = interface->Listen(fd,backlog);
    if(ret == -1){
        spdlog::error("FILE:{} Line:{} listen fail",__FILE__,__LINE__);
        return ret;
    }

    for(int i=0;i<workers.size();i++)
    {
        workers[i] = std::make_shared<Server>(fd,interface);
        int epfd = workers[i]->Init_Epoll_Fd();
        int timerfd = workers[i]->Start_Timer();
        workers[i]->callback.Accept_cb = std::bind(global_accept_cb,i);
        workers[i]->callback.Read_cb = std::bind(global_read_cb,i);
        workers[i]->callback.Write_cb = std::bind(global_read_cb,i);
        workers[i]->callback.Exit_cb = std::bind(global_exit_cb,i);
        reactors[i] = std::make_shared<Reactor>(event_num);
        reactors[i]->Add_Reactor(epfd,fd,EPOLLIN);
        reactors[i]->Add_Reactor(epfd,timerfd,EPOLLIN|EPOLLET);
        threads[i] = std::make_shared<std::thread>(&Reactor::Event_Loop,reactors[i],workers[i],epfd,-1);    
    }

    for(int i=0;i<workers.size();i++)
    {
        if(threads[i]->joinable())  threads[i]->join();
    }

    return 0;
}