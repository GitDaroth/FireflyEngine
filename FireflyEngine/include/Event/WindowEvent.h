#pragma once

#include "Event.h"

class WindowEvent : public Event {};

class WindowResizeEvent : public WindowEvent
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

class WindowCloseEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowCloseEvent";
	}
};

class WindowMaximizeEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowMaximizeEvent";
	}
};

class WindowMinimizeEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowMinimizeEvent";
	}
};

class WindowRestoreEvent : public WindowEvent
{
public:
	virtual std::string ToString() const override
	{
		return "WindowRestoreEvent";
	}
};

class WindowMoveEvent : public WindowEvent
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