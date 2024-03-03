#include "app/app.h"

App::App(Net_Layer::Ptr net_layer,Event_Layer::Ptr event_layer,Data_Layer::Ptr data_layer)
{
    this->data_layer = data_layer;
    this->event_layer = event_layer;
    this->net_layer = net_layer;
}

void App::start()
{
    net_layer->start(global_port,global_backlog,global_event_num);
}