#pragma once

#include <interface/IRenderUI.hpp>

class DemoLayer : public Scaffold::IRenderUI
{
public:
    void OnRenderUI(float deltaTime) override;
};
