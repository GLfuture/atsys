#include "app/app.h"
#include "global/init.h"

#define CONF_FILE_PATH  "../conf/atsys.conf"
#define PID_FILE_PATH   "../atsys.pid"

int main()
{   
    // pid_t pid = fork();
    // if(pid < 0){
    //     std::cout<<"create pid fail"<<std::endl;
    //     exit(-1);
    // }
    // else if(pid == 0)
    // {
    //     pid = setsid();
    //     if( pid == -1) {
    //         std::cout<<"create group pid fail"<<std::endl;
    //         exit(-1);
    //     }
    //     std::ofstream stream(PID_FILE_PATH);
    //     if(stream.fail()){
    //         std::cout<<"open pid file fail"<<std::endl;
    //         exit(-1);
    //     }
    //     std::string s_pid = std::to_string(getpid());
    //     stream.write(s_pid.c_str(),s_pid.length());
    //     stream.close();
        int ret = init_conf(CONF_FILE_PATH);
        if (ret == -1)
            exit(-1);
        Data_Layer::Ptr data_layer = std::make_shared<Data_Layer>();
        API_Manager::Ptr api_manager = std::make_shared<API_Manager>();
        api_manager->Register_POST_API("/api/login",std::make_shared<Login_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_POST_API("/api/register",std::make_shared<Register_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_POST_API("/api/logout",std::make_shared<Logout_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_POST_API("/api/card",std::make_shared<Card_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_GET_API("/api/data/card/self",std::make_shared<Data_Card_Concrete_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_GET_API("/api/data/user",std::make_shared<Data_User_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_POST_API("/api/user",std::make_shared<User_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_GET_API("/api/data/time",std::make_shared<Data_Time_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_POST_API("/api/time",std::make_shared<Time_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        api_manager->Register_GET_API("/api/data/card",std::make_shared<Data_Card_API>(data_layer->Get_Mysql_Pool(), data_layer->Get_Cache_Pool()));
        Net_Layer::Ptr net_layer = std::make_shared<Net_Layer>(global_worker_num);
        Event_Layer::Ptr event_layer = std::make_shared<Event_Layer>(api_manager);

        App::Ptr app = std::make_shared<App>(net_layer, event_layer, data_layer);
        init_callback(app);
        app->start();
    //}

    
    return 0;
}