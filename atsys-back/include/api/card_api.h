#ifndef CARD_API_H
#define CARD_API_H

#include "api_base.h"

class Card_API: public API_Base
{
public:
    Card_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) override;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) override;

private:
    int Deal_New_Card_Event(Context_Base::Ptr ctx);
    int Deal_Del_Card_Event(Context_Base::Ptr ctx);
    int Deal_Upd_Card_Event(Context_Base::Ptr ctx);
};

#endif