#include <imgui.h>

#include "DemoLayer.hpp"

void DemoLayer::OnRenderUI(float deltaTime)
{
    ImGui::ShowDemoWindow();
}