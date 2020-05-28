#pragma once

#include "Event.h"

class FIREFLY_API WindowEvent : public Event {};

class FIREFLY_API WindowResizeEvent : public WindowEvent
{
public:
	WindowResizeEvent(int width, int height) :
		m_width(width), m_height(height) {}

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }

	virtual std::string ToString() const override
	{
		return "WindowResizeEvent: " + std::to_string(m_width) + ", " + std::to_string(m_height);
	}

protected:
	int m_width, m_height;
};

class FIREFLY_API WindowCloseEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowCloseEvent";
	}
};
