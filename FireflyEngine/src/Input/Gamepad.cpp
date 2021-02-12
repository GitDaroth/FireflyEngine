#include "Input/Gamepad.h"

namespace Firefly
{
    Gamepad::Gamepad(const std::string& name) :
        m_name(name),
        m_axisLeftX(0.f),
        m_axisLeftY(0.f),
        m_axisRightX(0.f),
        m_axisRightY(0.f),
        m_triggerLeft(-1.f),
        m_triggerRight(-1.f)
    {
    }

    Gamepad::Gamepad(const Gamepad& gamepad) :
        m_name(gamepad.GetName()),
        m_axisLeftX(gamepad.GetAxisLeftX()),
        m_axisLeftY(gamepad.GetAxisLeftY()),
        m_axisRightX(gamepad.GetAxisRightX()),
        m_axisRightY(gamepad.GetAxisRightY()),
        m_triggerLeft(gamepad.GetTriggerLeft()),
        m_triggerRight(gamepad.GetTriggerRight())
    {
        for (int buttonCode = 0; buttonCode < FIREFLY_GAMEPAD_BUTTON_COUNT; buttonCode++)
        {
            m_isButtonPressedMap.insert(std::pair<int, bool>(buttonCode, gamepad.IsButtonPressed(buttonCode)));
        }
    }

    Gamepad::~Gamepad()
    {
        m_isButtonPressedMap.clear();
    }

    void Gamepad::SetName(const std::string& name)
    {
        m_name = name;
    }

    std::string Gamepad::GetName() const
    {
        return m_name;
    }

    void Gamepad::SetIsButtonPressed(int buttonCode, bool isPressed)
    {
        auto iter = m_isButtonPressedMap.find(buttonCode);
        if (iter != m_isButtonPressedMap.end())
            iter->second = isPressed;
        else
            m_isButtonPressedMap.insert(std::pair<int, bool>(buttonCode, isPressed));
    }

    bool Gamepad::IsButtonPressed(int buttonCode) const
    {
        auto iter = m_isButtonPressedMap.find(buttonCode);
        if (iter != m_isButtonPressedMap.end())
            return iter->second;
        else
            return false;
    }

    void Gamepad::SetAxisLeft(float xPos, float yPos)
    {
        m_axisLeftX = xPos;
        m_axisLeftY = yPos;
    }

    float Gamepad::GetAxisLeftX() const
    {
        return m_axisLeftX;
    }

    float Gamepad::GetAxisLeftY() const
    {
        return m_axisLeftY;
    }

    void Gamepad::SetAxisRight(float xPos, float yPos)
    {
        m_axisRightX = xPos;
        m_axisRightY = yPos;
    }

    float Gamepad::GetAxisRightX() const
    {
        return m_axisRightX;
    }

    float Gamepad::GetAxisRightY() const
    {
        return m_axisRightY;
    }

    void Gamepad::SetTriggerLeft(float pos)
    {
        m_triggerLeft = pos;
    }

    float Gamepad::GetTriggerLeft() const
    {
        return m_triggerLeft;
    }

    void Gamepad::SetTriggerRight(float pos)
    {
        m_triggerRight = pos;
    }

    float Gamepad::GetTriggerRight() const
    {
        return m_triggerRight;
    }
}