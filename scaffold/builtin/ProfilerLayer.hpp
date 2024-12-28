#pragma once

#include <interface/IRenderUI.hpp>

class ProfilerLayer : public Scaffold::IRenderUI
{
public:
	void OnRenderUI(float deltaTime) override;
};