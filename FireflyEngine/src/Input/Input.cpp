#include "pch.h"
#include "Input/Input.h"

namespace Firefly
{
	std::unordered_map<int, bool> Input::m_isKeyPressedMap;
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

	void Input::OnEvent(std::shared_ptr<Event> event)
	{
		if (auto mouseMoveEvent = event->AsType<MouseMoveEvent>())
		{
			m_mousePositionX = mouseMoveEvent->GetXPos();
			m_mousePositionY = mouseMoveEvent->GetYPos();
		}
		else if (auto mouseButtonEvent = event->AsType<MouseButtonEvent>())
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
		else if (auto keyEvent = event->AsType<KeyEvent>())
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
	}
}