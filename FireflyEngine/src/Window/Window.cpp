#include "pch.h"
#include "Window/Window.h"

namespace Firefly
{
	Window::Window(const std::string& title, int width, int height) :
		m_title(title),
		m_context(nullptr)
	{
		Logger::Info("Firefly Engine", "Creating a window: {0} ({1}x{2})", title, width, height);
	}

	Window::~Window()
	{
	}

	void Window::SetEventCallback(const std::function<void(std::shared_ptr<Event>)>& eventCallback)
	{
		m_eventCallback = eventCallback;
		SetupKeyCodeConversionMap();
		SetupMouseButtonCodeConversionMap();
		SetupGamepadButtonCodeConversionMap();
	}

	void Window::SetTitle(const std::string& title)
	{
		m_title = title;
		OnSetTitle(title);
	}

	void Window::SetSize(int width, int height)
	{
		OnSetSize(width, height);
	}

	const std::function<void(std::shared_ptr<Event>)>& Window::GetEventCallback()
	{
		return m_eventCallback;
	}

	const std::string& Window::GetTitle() const
	{
		return m_title;
	}

	int Window::ToFireflyKeyCode(int keyCode) const
	{
		auto iter = m_keyCodeConversionMap.find(keyCode);
		if (iter != m_keyCodeConversionMap.end())
			return iter->second;
		else
			return -1;
	}

	int Window::ToFireflyMouseButtonCode(int keyCode) const
	{
		auto iter = m_mouseButtonCodeConversionMap.find(keyCode);
		if (iter != m_mouseButtonCodeConversionMap.end())
			return iter->second;
		else
			return -1;
	}

	int Window::ToFireflyGamepadButtonCode(int keyCode) const
	{
		auto iter = m_gamepadButtonCodeConversionMap.find(keyCode);
		if (iter != m_gamepadButtonCodeConversionMap.end())
			return iter->second;
		else
			return -1;
	}
}