#ifndef APP_H
#define APP_H

#include "../layer/net_layer.h"
#include "../layer/event_layer.h"
#include "../layer/data_layer.h"
#include "global/global.h"

class App
{
public:
    using Ptr = std::shared_ptr<App>;
    App(Net_Layer::Ptr net_layer,Event_Layer::Ptr event_layer,Data_Layer::Ptr data_layer);
    void start();
    Net_Layer::Ptr Get_Net_Layer(){ return this->net_layer; }
    Event_Layer::Ptr Get_Event_Layer(){ return this->event_layer;   }
    Data_Layer::Ptr Get_Data_Layer() {  return this->data_layer;    }

private:
    Net_Layer::Ptr net_layer;
    Event_Layer::Ptr event_layer;
    Data_Layer::Ptr data_layer;
};



#endif