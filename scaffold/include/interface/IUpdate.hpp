#pragma once

#include <interface/IAppObject.hpp>

namespace Scaffold
{
    class IUpdate : virtual public Scaffold::IAppObject
    {
    public:
        virtual void OnUpdate(float deltaTime) = 0;
        virtual ~IUpdate() override { };
    };
} // namespace Scaffold