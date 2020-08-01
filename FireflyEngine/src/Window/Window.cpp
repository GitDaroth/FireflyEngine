#include "pch.h"
#include "Window/Window.h"

namespace Firefly
{
	Window::Window(const std::string& title, int width, int height) :
		m_title(title),
		m_width(width),
		m_height(height),
		m_isVSyncEnabled(false),
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
		m_width = width;
		m_height = height;
		OnSetSize(width, height);
	}

	void Window::EnableVSync(bool enabled)
	{
		m_isVSyncEnabled = enabled;
		OnEnableVSync(enabled);
	}

	const std::function<void(std::shared_ptr<Event>)>& Window::GetEventCallback()
	{
		return m_eventCallback;
	}

	const std::string& Window::GetTitle() const
	{
		return m_title;
	}

	int Window::GetWidth() const
	{
		return m_width;
	}

	int Window::GetHeight() const
	{
		return m_height;
	}

	bool Window::IsVSyncEnabled() const
	{
		return m_isVSyncEnabled;
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