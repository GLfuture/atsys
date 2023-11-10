#ifndef CARD_API_H
#define CARD_API_H

#include "api_base.h"

class Card_New_API: public API_Base
{
public:
    Card_New_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;
};

class Card_Del_API: public API_Base
{
public:
    Card_Del_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;
};

class Card_Upd_API:public API_Base
{
public:
    Card_Upd_API(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool);
    int Function(Context_Base::Ptr ctx) override;
};
#endif