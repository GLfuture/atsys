#ifndef APIBASE_H
#define APIBASE_H

#include "context.h"

class API_Base
{
public:
    using Ptr = std::shared_ptr<API_Base>;
    virtual int Function(Context_Base::Ptr ctx_ptr) = 0;
    virtual ~API_Base() {}
};



#endif