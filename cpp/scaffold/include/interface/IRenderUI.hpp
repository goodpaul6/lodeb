#pragma once

#include <interface/IAppObject.hpp>

namespace Scaffold
{
    class IRenderUI : virtual public Scaffold::IAppObject
    {
    public:
        virtual void OnRenderUI(float deltaTime) = 0;
        virtual ~IRenderUI() override { };
    };
} // namespace Scaffold

