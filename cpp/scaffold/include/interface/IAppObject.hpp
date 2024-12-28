#pragma once

#include <string>

namespace Scaffold
{
    class IAppObject
    {
    public:
        std::string name;
        virtual ~IAppObject() { }
    };
} // namespace Scaffold

