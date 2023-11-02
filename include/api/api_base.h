#ifndef APIBASE_H
#define APIBASE_H

#include "context.h"

class API_Base
{
public:
    virtual int Function(Context_Base::Ptr ctx_ptr) = 0;
};



#endif