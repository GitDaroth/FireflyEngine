#pragma once

#include "Event.h"

class MouseEvent : public InputEvent {};

class MouseButtonEvent : public MouseEvent
{
public:
	MouseButtonEvent(int buttonCode) :
		m_buttonCode(buttonCode) {}

	inline int GetButtonCode() const { return m_buttonCode; }

protected:
	int m_buttonCode;
};

class MouseButtonPressEvent : public MouseButtonEvent
{
public:
	MouseButtonPressEvent(int buttonCode) :
		MouseButtonEvent(buttonCode) {}

	virtual std::string ToString() const override
	{
		return "MouseButtonPressEvent: " + std::to_string(m_buttonCode);
	}
};

class MouseButtonReleaseEvent : public MouseButtonEvent
{
public:
	MouseButtonReleaseEvent(int buttonCode) :
		MouseButtonEvent(buttonCode) {}

	virtual std::string ToString() const override
	{
		return "MouseButtonReleaseEvent: " + std::to_string(m_buttonCode);
	}
};

class MouseMoveEvent : public MouseEvent
{
public:
	MouseMoveEvent(int xPos, int yPos) :
		m_xPos(xPos), m_yPos(yPos) {}

	inline int GetXPos() const { return m_xPos; }
	inline int GetYPos() const { return m_yPos; }

	virtual std::string ToString() const override
	{
		return "MouseMoveEvent: " + std::to_string(m_xPos) + ", " + std::to_string(m_yPos);
	}

protected:
	int m_xPos, m_yPos;
};

class MouseScrollEvent : public MouseEvent
{
public:
	MouseScrollEvent(float xOffset, float yOffset) :
		m_xOffset(xOffset), m_yOffset(yOffset) {}

	inline float GetXOffset() const { return m_xOffset; }
	inline float GetYOffset() const { return m_yOffset; }

	virtual std::string ToString() const override
	{
		return "MouseScrollEvent: " + std::to_string(m_xOffset) + ", " + std::to_string(m_yOffset);
	}

protected:
	float m_xOffset, m_yOffset;
};