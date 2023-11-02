#include "app/app.h"
#include "global/init.h"

#define CONF_FILE_PATH "../conf/atsys.conf"

int main()
{   

    int ret = init_conf(CONF_FILE_PATH);
    if(ret == -1)  exit(-1);
    App::Ptr app = std::make_shared<App>();
    init_callback(app);
    app->start();
    return 0;
}