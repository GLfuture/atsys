#ifndef CONTEXT_H
#define CONTEXT_H
#include <memory>

class Context_Base
{
public:
    using Ptr = std::shared_ptr<Context_Base>;
};


class Http_Context:public Context_Base
{
public:
    using Ptr = std::shared_ptr<Http_Context>;
};

#endif