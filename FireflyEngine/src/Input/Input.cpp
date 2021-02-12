#include "pch.h"
#include "Input/Input.h"

namespace Firefly
{
    std::unordered_map<int, bool> Input::m_isKeyPressedMap;
    std::unordered_map<int, Gamepad> Input::m_gamepadMap;
    std::unordered_map<int, bool> Input::m_isMouseButtonPressedMap;
    int Input::m_mousePositionX = 0;
    int Input::m_mousePositionY = 0;

    bool Input::IsKeyPressed(int keyCode)
    {
        auto iter = m_isKeyPressedMap.find(keyCode);
        if (iter != m_isKeyPressedMap.end())
            return iter->second;
        else
            return false;
    }

    bool Input::IsMouseButtonPressed(int mouseButton)
    {
        auto iter = m_isMouseButtonPressedMap.find(mouseButton);
        if (iter != m_isMouseButtonPressedMap.end())
            return iter->second;
        else
            return false;
    }

    int Input::GetMousePositionX()
    {
        return m_mousePositionX;
    }

    int Input::GetMousePositionY()
    {
        return m_mousePositionY;
    }

    std::string Input::GetGamepadName(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.GetName();
        else
            return "";
    }

    bool Input::IsGamepadConnected(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return true;
        else
            return false;
    }

    bool Input::IsGamepadButtonPressed(int gamepadNumber, int gamepadButton)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.IsButtonPressed(gamepadButton);
        else
            return false;
    }

    float Input::GetGamepadAxisLeftX(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.GetAxisLeftX();
        else
            return 0.0f;
    }

    float Input::GetGamepadAxisLeftY(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.GetAxisLeftY();
        else
            return 0.0f;
    }

    float Input::GetGamepadAxisRightX(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.GetAxisRightX();
        else
            return 0.0f;
    }

    float Input::GetGamepadAxisRightY(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.GetAxisRightY();
        else
            return 0.0f;
    }

    float Input::GetGamepadTriggerLeft(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.GetTriggerLeft();
        else
            return 0.0f;
    }

    float Input::GetGamepadTriggerRight(int gamepadNumber)
    {
        auto iter = m_gamepadMap.find(gamepadNumber);
        if (iter != m_gamepadMap.end())
            return iter->second.GetTriggerRight();
        else
            return 0.0f;
    }

    void Input::OnEvent(std::shared_ptr<Event> event)
    {
        if (auto mouseEvent = event->AsType<MouseEvent>())
            OnMouseEvent(mouseEvent);
        else if (auto keyEvent = event->AsType<KeyEvent>())
            OnKeyEvent(keyEvent);
        else if (auto gamepadEvent = event->AsType<GamepadEvent>())
            OnGamepadEvent(gamepadEvent);
    }

    void Input::OnKeyEvent(std::shared_ptr<KeyEvent> keyEvent)
    {
        bool isKeyDown = false;
        if (keyEvent->IsType<KeyPressEvent>() || keyEvent->IsType<KeyRepeatEvent>())
            isKeyDown = true;

        auto iter = m_isKeyPressedMap.find(keyEvent->GetKeyCode());
        if (iter != m_isKeyPressedMap.end())
            iter->second = isKeyDown;
        else
            m_isKeyPressedMap.insert(std::pair<int, bool>(keyEvent->GetKeyCode(), isKeyDown));
    }

    void Input::OnMouseEvent(std::shared_ptr<MouseEvent> mouseEvent)
    {
        if (auto mouseMoveEvent = mouseEvent->AsType<MouseMoveEvent>())
            OnMouseMoveEvent(mouseMoveEvent);
        else if (auto mouseButtonEvent = mouseEvent->AsType<MouseButtonEvent>())
            OnMouseButtonEvent(mouseButtonEvent);
    }

    void Input::OnMouseMoveEvent(std::shared_ptr<MouseMoveEvent> mouseMoveEvent)
    {
        m_mousePositionX = mouseMoveEvent->GetXPos();
        m_mousePositionY = mouseMoveEvent->GetYPos();
    }

    void Input::OnMouseButtonEvent(std::shared_ptr<MouseButtonEvent> mouseButtonEvent)
    {
        bool isButtonDown = false;
        if (mouseButtonEvent->IsType<MouseButtonPressEvent>())
            isButtonDown = true;

        auto iter = m_isMouseButtonPressedMap.find(mouseButtonEvent->GetButtonCode());
        if (iter != m_isMouseButtonPressedMap.end())
            iter->second = isButtonDown;
        else
            m_isMouseButtonPressedMap.insert(std::pair<int, bool>(mouseButtonEvent->GetButtonCode(), isButtonDown));
    }

    void Input::OnGamepadEvent(std::shared_ptr<GamepadEvent> gamepadEvent)
    {
        if (gamepadEvent->IsType<GamepadConnectedEvent>())
        {
            auto iter = m_gamepadMap.find(gamepadEvent->GetGamepadNumber());
            if (iter != m_gamepadMap.end())
                m_gamepadMap.erase(gamepadEvent->GetGamepadNumber());

            m_gamepadMap.insert(std::pair<int, Gamepad>(gamepadEvent->GetGamepadNumber(), Gamepad(gamepadEvent->GetGamepadName())));
        }
        else if (gamepadEvent->IsType<GamepadDisconnectedEvent>())
        {
            auto iter = m_gamepadMap.find(gamepadEvent->GetGamepadNumber());
            if (iter != m_gamepadMap.end())
                m_gamepadMap.erase(gamepadEvent->GetGamepadNumber());
        }

        if (auto gamepadButtonEvent = gamepadEvent->AsType<GamepadButtonEvent>())
            OnGamepadButtonEvent(gamepadButtonEvent);
        else if (auto gamepadAxisMoveEvent = gamepadEvent->AsType<GamepadAxisMoveEvent>())
            OnGamepadAxisMoveEvent(gamepadAxisMoveEvent);
        else if (auto gamepadTriggerMoveEvent = gamepadEvent->AsType<GamepadTriggerMoveEvent>())
            OnGamepadTriggerMoveEvent(gamepadTriggerMoveEvent);
    }

    void Input::OnGamepadButtonEvent(std::shared_ptr<GamepadButtonEvent> gamepadButtonEvent)
    {
        bool isButtonDown = false;
        if (gamepadButtonEvent->IsType<GamepadButtonPressEvent>())
            isButtonDown = true;

        auto pairKey = std::pair<int, int>(gamepadButtonEvent->GetGamepadNumber(), gamepadButtonEvent->GetButtonCode());
        auto iter = m_gamepadMap.find(gamepadButtonEvent->GetGamepadNumber());
        if (iter != m_gamepadMap.end())
            iter->second.SetIsButtonPressed(gamepadButtonEvent->GetButtonCode(), isButtonDown);
    }

    void Input::OnGamepadAxisMoveEvent(std::shared_ptr<GamepadAxisMoveEvent> gamepadAxisModeEvent)
    {
        if (gamepadAxisModeEvent->IsType<GamepadAxisLeftMoveEvent>())
        {
            auto iter = m_gamepadMap.find(gamepadAxisModeEvent->GetGamepadNumber());
            if (iter != m_gamepadMap.end())
                iter->second.SetAxisLeft(gamepadAxisModeEvent->GetXPos(), gamepadAxisModeEvent->GetYPos());
        }
        else if (gamepadAxisModeEvent->IsType<GamepadAxisRightMoveEvent>())
        {
            auto iter = m_gamepadMap.find(gamepadAxisModeEvent->GetGamepadNumber());
            if (iter != m_gamepadMap.end())
                iter->second.SetAxisRight(gamepadAxisModeEvent->GetXPos(), gamepadAxisModeEvent->GetYPos());
        }
    }

    void Input::OnGamepadTriggerMoveEvent(std::shared_ptr<GamepadTriggerMoveEvent> gamepadTriggerModeEvent)
    {
        if (gamepadTriggerModeEvent->IsType<GamepadTriggerLeftMoveEvent>())
        {
            auto iter = m_gamepadMap.find(gamepadTriggerModeEvent->GetGamepadNumber());
            if (iter != m_gamepadMap.end())
                iter->second.SetTriggerLeft(gamepadTriggerModeEvent->GetPos());
        }
        else if (gamepadTriggerModeEvent->IsType<GamepadTriggerRightMoveEvent>())
        {
            auto iter = m_gamepadMap.find(gamepadTriggerModeEvent->GetGamepadNumber());
            if (iter != m_gamepadMap.end())
                iter->second.SetTriggerRight(gamepadTriggerModeEvent->GetPos());
        }
    }
}