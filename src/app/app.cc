#include "app/app.h"

App::App()
{
    net_layer = std::make_shared<Net_Layer>(global_worker_num);
    event_layer = std::make_shared<Event_Layer>();
}

void App::start()
{
    net_layer->start(global_port,global_backlog,global_event_num);
}