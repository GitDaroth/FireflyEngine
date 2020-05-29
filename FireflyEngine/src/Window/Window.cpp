#include "pch.h"
#include "Window/Window.h"

namespace Firefly
{
	Window::Window(const std::string& title, int width, int height) :
		m_title(title),
		m_width(width),
		m_height(height),
		m_isVSyncEnabled(false)
	{
	}

	Window::~Window()
	{
	}

	void Window::SetEventCallback(const std::function<void(std::shared_ptr<Event>)>& eventCallback)
	{
		m_eventCallback = eventCallback;
		SetupKeyCodeConversionMap();
		SetupMouseButtonCodeConversionMap();
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
}