#pragma once

#include <glfw3.h>
#include <glm/glm.hpp>

#include <KeyCodes.h>

#include <unordered_map>

namespace Scaffold
{
    class Input
    {
    public:
        Input(GLFWwindow* window);
        ~Input() = default;

    public:
        void SetKeyState(KeyCode key, KeyState state);
        void SetMouseButtonState(MouseButton button, KeyState state);

        void UpdateKeyStates();

        KeyState GetKeyState(KeyCode key);
        KeyState GetMouseButtonState(MouseButton button);

        glm::vec2 GetMousePosition();

        bool IsKeyDown(KeyCode key);
        bool IsMouseButtonDown(MouseButton button);

        void SetCursorMode(CursorMode mode);

        std::unordered_map<KeyCode, KeyState>& GetKeyStateMap();
        std::unordered_map<MouseButton, KeyState>& GetMouseButtonStateMap();

    private:
        GLFWwindow* m_windowHandle = nullptr;

        CursorMode m_cursorMode = CursorMode::Normal;

        std::unordered_map<KeyCode, KeyState> m_keyMap;
        std::unordered_map<MouseButton, KeyState> m_mouseMap;
    };
}