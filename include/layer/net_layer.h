#ifndef NET_LAYER_H
#define NET_LAYER_H

#include "../global/global.h"
#include "../base/net/reactor.h"
#include <spdlog/spdlog.h>

class Net_Layer
{
public:
    using Ptr = std::shared_ptr<Net_Layer>;
    using Thread_Ptr = std::shared_ptr<std::thread>;
    Net_Layer(int worker_num);
    int start(uint32_t port,uint32_t backlog,uint16_t event_num);
    Reactor::Ptr Get_Reactor(int i){    return reactors[i]; }
    Server_Base::Ptr Get_Server(int i ){    return workers[i];  }
    

private:
    std::vector<Reactor::Ptr> reactors;
    std::vector<Server_Base::Ptr> workers;
    std::vector<Thread_Ptr> threads;
};



#endif