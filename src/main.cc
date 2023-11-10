#include "app/app.h"
#include "global/init.h"

#define CONF_FILE_PATH "../conf/atsys.conf"

int main()
{   

    int ret = init_conf(CONF_FILE_PATH);
    if(ret == -1)  exit(-1);
    Data_Layer::Ptr data_layer          = std::make_shared<Data_Layer>();
    API_Manager::Ptr api_manager        = std::make_shared<API_Manager>(TATOL_API_NUM);
    api_manager->Register_API(std::make_shared<Login_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),LOGIN_API);
    api_manager->Register_API(std::make_shared<Register_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),REGISTER_API);
    api_manager->Register_API(std::make_shared<Logout_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),LOGOUT_API);
    api_manager->Register_API(std::make_shared<Card_New_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),CARD_NEW_API);
    api_manager->Register_API(std::make_shared<Card_Del_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),CARD_DEL_API);
    api_manager->Register_API(std::make_shared<Card_Upd_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),CARD_UPD_API);
    api_manager->Register_API(std::make_shared<Data_Card_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),DATA_CARD_API);
    api_manager->Register_API(std::make_shared<Data_User_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),DATA_USER_API);
    api_manager->Register_API(std::make_shared<User_Upd_API>(data_layer->Get_Mysql_Pool(),data_layer->Get_Cache_Pool()),USER_UPD_API);
    Net_Layer::Ptr net_layer = std::make_shared<Net_Layer>(global_worker_num);
    Event_Layer::Ptr event_layer = std::make_shared<Event_Layer>(api_manager);

    App::Ptr app = std::make_shared<App>(net_layer,event_layer,data_layer);
    init_callback(app);
    app->start();
    return 0;
}