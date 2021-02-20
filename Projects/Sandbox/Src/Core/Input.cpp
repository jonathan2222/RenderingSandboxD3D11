#include "PreCompiled.h"
#include "Input.h"

#include "Core/Display.h"

using namespace RS;

std::shared_ptr<Input> Input::Get()
{
    static std::shared_ptr<Input> s_Input = std::make_shared<Input>();
    return s_Input;
}

void Input::Init()
{
    GLFWwindow* wnd = static_cast<GLFWwindow*>(Display::Get()->GetGLFWWindow());
    glfwSetKeyCallback(wnd, Input::KeyCallback);
    glfwSetCursorPosCallback(wnd, Input::CursorPositionCallback);
    glfwSetMouseButtonCallback(wnd, Input::MouseButtonCallback);
    glfwSetScrollCallback(wnd, Input::MouseScrollCallback);

    for (int i = (int)Key::FIRST; i <= (int)Key::LAST; i++)
        s_KeyMap[(Key)i] = KeyState::RELEASED;

    for (int i = (int)MB::LEFT; i <= (int)MB::MIDDLE; i++)
        s_MBMap[(MB)i] = KeyState::RELEASED;
}

void Input::Update()
{
    for (auto& key : s_KeyMap)
    {
        if (key.second == KeyState::FIRST_PRESSED)
            key.second = KeyState::PRESSED;
        if (key.second == KeyState::FIRST_RELEASED)
            key.second = KeyState::RELEASED;
    }

    s_MouseDelta = s_MousePos - s_MousePosPre;
    s_MousePosPre = s_MousePos;
}

bool Input::IsKeyPressed(const Key& key) const
{
    return s_KeyMap[key] == KeyState::FIRST_PRESSED || s_KeyMap[key] == KeyState::PRESSED;
}

bool Input::IsKeyReleased(const Key& key) const
{
    return !IsKeyPressed(key);
}

KeyState Input::GetKeyState(const Key& key)
{
    auto& it = s_KeyMap.find(key);
    if (it == s_KeyMap.end())
        return KeyState::RELEASED;
    else return it->second;
}

glm::vec2 Input::GetCursorDelta()
{
    return s_MouseDelta;
}

glm::vec2 Input::GetScrollDelta()
{
    return s_ScrollDelta;
}

glm::vec2 Input::GetMousePos() const
{
    return s_MousePos;
}

bool Input::IsMBPressed(const MB& button) const
{
    return s_MBMap[button] == KeyState::PRESSED;
}

bool Input::IsMBReleased(const MB& button) const
{
    return s_MBMap[button] == KeyState::RELEASED;
}

void Input::CenterMouse() const
{
    GLFWwindow* wnd = static_cast<GLFWwindow*>(Display::Get()->GetGLFWWindow());
    s_MouseDelta = s_MousePos - s_MousePosPre;
    s_MousePos.x = (float)Display::Get()->GetWidth() * 0.5f;
    s_MousePos.y = (float)Display::Get()->GetHeight() * 0.5f;
    glfwSetCursorPos(wnd, (double)s_MousePos.x, (double)s_MousePos.y);
}

void Input::LockMouse() const
{
    GLFWwindow* wnd = static_cast<GLFWwindow*>(Display::Get()->GetGLFWWindow());
    glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    s_MousePosPre = s_MousePos;
    s_MouseDelta.x = 0.0f;
    s_MouseDelta.y = 0.0f;
}

void Input::UnlockMouse() const
{
    GLFWwindow* wnd = static_cast<GLFWwindow*>(Display::Get()->GetGLFWWindow());
    glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    s_MousePosPre = s_MousePos;
    s_MouseDelta.x = 0.0f;
    s_MouseDelta.y = 0.0f;
}

std::string Input::KeyStateToStr(KeyState state)
{
    std::string str;
    switch (state)
    {
    case KeyState::RELEASED:
        str = "RELEASED";
        break;
    case KeyState::FIRST_RELEASED:
        str = "FIRST_RELEASED";
        break;
    case KeyState::PRESSED:
        str = "PRESSED";
        break;
    case KeyState::FIRST_PRESSED:
        str = "FIRST_PRESSED";
        break;
    default:
        str = "UNKNOWN_STATE";
        break;
    }
    return str;
}

void Input::KeyCallback(GLFWwindow* wnd, int key, int scancode, int action, int mods)
{
    //ImGuiImpl* imGuiImpl = Display::get()->getImGuiImpl();
    //if (imGuiImpl == nullptr || (imGuiImpl != nullptr && imGuiImpl->needInput() == false))
    {
        Key ymKey = (Key)key;
        if (action == GLFW_PRESS)
            s_KeyMap[ymKey] = KeyState::FIRST_PRESSED;
        if (action == GLFW_RELEASE)
            s_KeyMap[ymKey] = KeyState::FIRST_RELEASED;
    }
}

void Input::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    //ImGuiImpl* imGuiImpl = Display::get()->getImGuiImpl();
    //if (imGuiImpl == nullptr || (imGuiImpl != nullptr && imGuiImpl->needInput() == false))
    {
        s_MousePos.x = (float)xpos;
        s_MousePos.y = (float)ypos;
    }
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    //ImGuiImpl* imGuiImpl = Display::get()->getImGuiImpl();
    //if (imGuiImpl == nullptr || (imGuiImpl != nullptr && imGuiImpl->needInput() == false))
    {
        MB ymButton = (MB)button;
        if (action == GLFW_PRESS)
            s_MBMap[ymButton] = KeyState::PRESSED;
        if (action == GLFW_RELEASE)
            s_MBMap[ymButton] = KeyState::RELEASED;
    }
}

void Input::MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    s_ScrollDelta.x = (float)xOffset;
    s_ScrollDelta.y = (float)yOffset;
}
