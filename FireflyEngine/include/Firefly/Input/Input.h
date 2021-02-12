#pragma once

#include "Core/Core.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Event/GamepadEvent.h"
#include "Input/KeyCodes.h"
#include "Input/MouseButtonCodes.h"
#include "Input/Gamepad.h"

namespace Firefly
{
    class  Input
    {
    public:
        static bool IsKeyPressed(int keyCode);

        static bool IsMouseButtonPressed(int mouseButton);
        static int GetMousePositionX();
        static int GetMousePositionY();

        static std::string GetGamepadName(int gamepadNumber);
        static bool IsGamepadConnected(int gamepadNumber);
        static bool IsGamepadButtonPressed(int gamepadNumber, int gamepadButton);
        static float GetGamepadAxisLeftX(int gamepadNumber);
        static float GetGamepadAxisLeftY(int gamepadNumber);
        static float GetGamepadAxisRightX(int gamepadNumber);
        static float GetGamepadAxisRightY(int gamepadNumber);
        static float GetGamepadTriggerLeft(int gamepadNumber);
        static float GetGamepadTriggerRight(int gamepadNumber);

        static void OnEvent(std::shared_ptr<Event> event);

    private:
        static void OnKeyEvent(std::shared_ptr<KeyEvent> keyEvent);
        static void OnMouseEvent(std::shared_ptr<MouseEvent> mouseEvent);
        static void OnMouseMoveEvent(std::shared_ptr<MouseMoveEvent> mouseMoveEvent);
        static void OnMouseButtonEvent(std::shared_ptr<MouseButtonEvent> mouseButtonEvent);
        static void OnGamepadEvent(std::shared_ptr<GamepadEvent> gamepadEvent);
        static void OnGamepadButtonEvent(std::shared_ptr<GamepadButtonEvent> gamepadButtonEvent);
        static void OnGamepadAxisMoveEvent(std::shared_ptr<GamepadAxisMoveEvent> gamepadAxisModeEvent);
        static void OnGamepadTriggerMoveEvent(std::shared_ptr<GamepadTriggerMoveEvent> gamepadTriggerModeEvent);

        static std::unordered_map<int, bool> m_isKeyPressedMap;
        static std::unordered_map<int, Gamepad> m_gamepadMap;
        static std::unordered_map<int, bool> m_isMouseButtonPressedMap;
        static int m_mousePositionX;
        static int m_mousePositionY;
    };
}