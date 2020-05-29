#pragma once

#include "Event.h"

class KeyEvent : public InputEvent
{
public:
	KeyEvent(int keyCode) :
		m_keyCode(keyCode) {}

	inline int GetKeyCode() const { return m_keyCode; }

protected:
	int m_keyCode;
};

class KeyPressEvent : public KeyEvent
{
public:
	KeyPressEvent(int keyCode) :
		KeyEvent(keyCode) {}

	virtual std::string ToString() const override
	{
		return "KeyPressEvent: " + std::to_string(m_keyCode);
	}
};

class KeyReleaseEvent : public KeyEvent
{
public:
	KeyReleaseEvent(int keyCode) :
		KeyEvent(keyCode) {}

	virtual std::string ToString() const override
	{
		return "KeyReleaseEvent: " + std::to_string(m_keyCode);
	}
};

class KeyRepeatEvent : public KeyEvent
{
public:
	KeyRepeatEvent(int keyCode) :
		KeyEvent(keyCode) {}

	virtual std::string ToString() const override
	{
		return "KeyRepeatEvent: " + std::to_string(m_keyCode);
	}
};