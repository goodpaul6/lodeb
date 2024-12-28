#include <Application.hpp>
#include <Input.hpp>
#include <Profiler.hpp>

#include <iostream>

using namespace Scaffold;

static Input* s_instance = nullptr;
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseCallback(GLFWwindow* window, int button, int action, int mods);

Scaffold::Input::Input(GLFWwindow* window)
{
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseCallback);

    m_windowHandle = window;
    s_instance = this;
}

void Scaffold::Input::SetKeyState(KeyCode key, KeyState state)
{
    m_keyMap[key] = state;
}

void Scaffold::Input::SetMouseButtonState(MouseButton button, KeyState state)
{
    m_mouseMap[button] = state;
}

KeyState Scaffold::Input::GetKeyState(KeyCode code)
{
    return m_keyMap[code];
}

KeyState Scaffold::Input::GetMouseButtonState(MouseButton button)
{
    return m_mouseMap[button];
}

glm::vec2 Scaffold::Input::GetMousePosition()
{
    double xpos, ypos;
    glfwGetCursorPos(m_windowHandle, &xpos, &ypos);
    return glm::vec2() = { xpos, ypos };
}

void Scaffold::Input::UpdateKeyStates()
{
    Application::GetProfiler().BeginMarker("Input::UpdateKeyStates");

    //TODO convert to parallel for_each?

    {
        for (auto&& pair : m_keyMap)
        {
            KeyState state = pair.second;

            if (state == KeyState::None || state == KeyState::Held) continue;

            if (state == KeyState::Pressed) state = KeyState::Held;
            if (state == KeyState::Released) state = KeyState::None;

            m_keyMap[pair.first] = state;
        }
    }

    {
        for (auto&& pair : m_mouseMap)
        {
            KeyState state = pair.second;

            if (state == KeyState::None || state == KeyState::Held) continue;

            if (state == KeyState::Pressed) state = KeyState::Held;
            if (state == KeyState::Released) state = KeyState::None;

            m_mouseMap[pair.first] = state;
        }
    }

    Application::GetProfiler().EndMarker();
}

bool Scaffold::Input::IsKeyDown(KeyCode key)
{
    KeyState state = GetKeyState(key);
    return state == KeyState::Pressed || state == KeyState::Held;
}

bool Scaffold::Input::IsMouseButtonDown(MouseButton button)
{
    KeyState state = GetMouseButtonState(button);
    return state == KeyState::Pressed || state == KeyState::Held;
}

void Scaffold::Input::SetCursorMode(CursorMode mode)
{
    if (mode == m_cursorMode) return;
    m_cursorMode = mode;

    glfwSetInputMode(m_windowHandle, GLFW_CURSOR, (int)mode);
}

std::unordered_map<KeyCode, KeyState>& Scaffold::Input::GetKeyStateMap()
{
    return m_keyMap;
}

std::unordered_map<MouseButton, KeyState>& Scaffold::Input::GetMouseButtonStateMap()
{
    return m_mouseMap;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_REPEAT) return;
    if (key == GLFW_KEY_UNKNOWN) return;

    KeyState state = (KeyState)(action);
    s_instance->SetKeyState((KeyCode)key, state);
}

void MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    KeyState state = (KeyState)(action);
    s_instance->SetMouseButtonState((MouseButton)button, state);
}
