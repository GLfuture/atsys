#ifndef APIBASE_H
#define APIBASE_H


#include "base/mysqlpool/MySqlPool.h"
#include "base/cachepool/CachePool.h"
#include "context.h"
#include "../base/parser/parser.h"
#include "nlohmann/json.hpp"
#include "../base/parser/HTTP.hpp"
#include "../base/encoder/encoder.h"
#include "../global/global.h"
#include "../base/token_analyzer/token_analyzer.h"

using nlohmann::json;

class API_Base
{
public:
    API_Base(MySqlPool::Ptr mysqlpool,CachePool::Ptr cachepool){
        this->mysqlpool = mysqlpool;
        this->cachepool = cachepool;
    }
    using Ptr = std::shared_ptr<API_Base>;

    //virtual bool Check_Token(const std::string& token) = 0;
    virtual std::string Function(HTTP_NSP::HTTP::Ptr http) = 0;
    virtual int Deal_Real_Event(Context_Base::Ptr ctx) = 0;
    virtual ~API_Base() {}
protected:
    MySqlPool::Ptr mysqlpool;
    CachePool::Ptr cachepool;
};



#endif