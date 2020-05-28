#pragma once

#include "Event.h"

class FIREFLY_API MouseEvent : public InputEvent {};

class FIREFLY_API MouseButtonEvent : public MouseEvent
{
public:
	MouseButtonEvent(int buttonCode) :
		m_buttonCode(buttonCode) {}

	inline int GetButtonCode() const { return m_buttonCode; }

protected:
	int m_buttonCode;
};

class FIREFLY_API MouseButtonPressEvent : public MouseButtonEvent
{
public:
	MouseButtonPressEvent(int buttonCode) :
		MouseButtonEvent(buttonCode) {}

	virtual std::string ToString() const override
	{
		return "MouseButtonPressEvent: " + std::to_string(m_buttonCode);
	}
};

class FIREFLY_API MouseButtonReleaseEvent : public MouseButtonEvent
{
public:
	MouseButtonReleaseEvent(int buttonCode) :
		MouseButtonEvent(buttonCode) {}

	virtual std::string ToString() const override
	{
		return "MouseButtonReleaseEvent: " + std::to_string(m_buttonCode);
	}
};

class FIREFLY_API MouseMoveEvent : public MouseEvent
{
public:
	MouseMoveEvent(float xPos, float yPos) :
		m_xPos(xPos), m_yPos(yPos) {}

	inline float GetXPos() const { return m_xPos; }
	inline float GetYPos() const { return m_yPos; }

	virtual std::string ToString() const override
	{
		return "MouseMoveEvent: " + std::to_string(m_xPos) + ", " + std::to_string(m_yPos);
	}

protected:
	float m_xPos, m_yPos;
};

class FIREFLY_API MouseScrollEvent : public MouseEvent
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