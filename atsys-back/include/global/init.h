#ifndef INIT_H
#define INIT_H

#include "../base/parser/configer.h"
#include "../app/app.h"
#include "global.h"

extern int  init_conf(const std::string& filename);
extern void init_callback(App::Ptr app);
extern void init_smtp();

void Accept_cb(int workerid,Net_Layer::Ptr net_layer);
void Read_cb(int workerid,App::Ptr app_ptr);
void Write_cb(int workerid,Net_Layer::Ptr net_layer);
void Exit_cb(int workerid,Net_Layer::Ptr net_layer);


#endif