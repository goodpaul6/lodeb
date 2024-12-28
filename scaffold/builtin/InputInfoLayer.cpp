#include <imgui.h>
#include <glm/glm.hpp>

#include <Application.hpp>
#include "InputInfoLayer.hpp"
#include <Input.hpp>
#include <KeyCodes.h>

std::string StateToString(Scaffold::KeyState state)
{
    std::string stateName;

    switch (state)
    {
    case Scaffold::KeyState::Held:
        stateName = "Held";
        break;
    case Scaffold::KeyState::Pressed:
        stateName = "Pressed";
        break;
    case Scaffold::KeyState::Released:
        stateName = "Released";
        break;
    case Scaffold::KeyState::None:
        stateName = "None";
        break;
    }

    return stateName;
}

void InputInfoLayer::OnRenderUI(float deltaTime)
{
    bool show = ImGui::Begin("Input Info");
    if (!show) { ImGui::End(); return; }

    Scaffold::Input& input = Scaffold::Application::GetInstance().GetInput();
    glm::vec2 mousePos = input.GetMousePosition();

    ImGui::Text("Mouse position | x: %.2f, y: %.2f", mousePos.x, mousePos.y);

    ImGui::SeparatorText("STATE MAPS");

    auto keyMap = input.GetKeyStateMap();
    auto mouseButtonMap = input.GetMouseButtonStateMap();

    if (ImGui::BeginListBox("key log box", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y)))
    {
        for (auto&& pair : keyMap)
        {
            Scaffold::KeyCode key = pair.first;
            Scaffold::KeyState state = pair.second;

            ImGui::Text("KeyCode: %i, KeyState: %s", (int)key, StateToString(state).c_str());
        }

        for (auto&& pair : mouseButtonMap)
        {
            Scaffold::MouseButton key = pair.first;
            Scaffold::KeyState state = pair.second;

            ImGui::Text("MouseButton: %i, KeyState: %s", (int)key, StateToString(state).c_str());
        }

        ImGui::EndListBox();
    }

    ImGui::End();
}
