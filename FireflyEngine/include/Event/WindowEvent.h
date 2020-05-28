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

class FIREFLY_API WindowMaximizeEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowMaximizeEvent";
	}
};

class FIREFLY_API WindowMinimizeEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowMinimizeEvent";
	}
};

class FIREFLY_API WindowRestoreEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowRestoreEvent";
	}
};

class FIREFLY_API WindowMoveEvent : public WindowEvent
{
public:
	WindowMoveEvent(int xPos, int yPos) :
		m_xPos(xPos), m_yPos(yPos) {}

	inline int GetXPos() const { return m_xPos; }
	inline int GetYPos() const { return m_yPos; }

	virtual std::string ToString() const override
	{
		return "WindowMoveEvent: " + std::to_string(m_xPos) + ", " + std::to_string(m_yPos);
	}

protected:
	int m_xPos, m_yPos;
};