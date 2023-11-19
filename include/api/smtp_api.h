#ifndef SMTP_API_H
#define SMTP_API_H

#include "api_base.h"
class Smtp_API : public API_Base
{
public:
    std::string Function(HTTP_NSP::HTTP::Ptr http) override;
};


#endif