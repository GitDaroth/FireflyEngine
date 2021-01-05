#include "pch.h"
#include "Core/Application.h"

#include "Window/WindowsWindow.h"
#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	Application::Application() 
	{
#ifdef FIREFLY_WINDOWS
		m_window = std::make_unique<WindowsWindow>();
#endif

		m_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}

	Application::~Application()
	{
	}

	void Application::Update(float deltaTime)
	{
		OnUpdate(deltaTime);

		m_window->SetTitle(std::to_string(1.f / deltaTime));
		m_window->OnUpdate(deltaTime);
	}

	void Application::RequestShutdown()
	{
		m_isShutdownRequested = true;
	}

	bool Application::IsShutdownRequested() const
	{
		return m_isShutdownRequested;
	}

	void Application::OnEvent(std::shared_ptr<Event> event)
	{
		Input::OnEvent(event);

		if (auto windowEvent = event->AsType<WindowEvent>())
		{
			OnWindowEvent(windowEvent);

			if (auto closeEvent = windowEvent->AsType<WindowCloseEvent>())
			{
				RequestShutdown();
			}
			else if (auto resizeEvent = windowEvent->AsType<WindowResizeEvent>())
			{
				int width = resizeEvent->GetWidth();
				int height = resizeEvent->GetHeight();

				//if (width != 0 && height != 0)
				//	RenderingAPI::GetRenderFunctions()->SetViewport(0, 0, width, height);
			}
		}
		else if (auto keyEvent = event->AsType<KeyEvent>())
		{
			OnKeyEvent(keyEvent);
		}
		else if (auto mouseEvent = event->AsType<MouseEvent>())
		{
			OnMouseEvent(mouseEvent);
		}
		else if (auto gamepadEvent = event->AsType<GamepadEvent>())
		{
			OnGamepadEvent(gamepadEvent);
		}
	}
}