#ifndef LOGIN_API_H
#define LOGIN_API_H

#include "api_base.h"
class Login_API : public API_Base
{
public:
    int Function(Context_Base::Ptr ctx_ptr) override;
};

#endif