#pragma once

#include <interface/IRenderUI.hpp>

class InputInfoLayer : public Scaffold::IRenderUI
{
public:
    void OnRenderUI(float deltaTime) override;
};