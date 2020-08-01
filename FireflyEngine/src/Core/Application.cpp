#include "pch.h"
#include "Core/Application.h"

#include "Window/WindowsWindow.h"
#include "Event/WindowEvent.h"
#include "Rendering/RenderingAPI.h"

#include <GLFW/glfw3.h>

namespace Firefly
{
	Application::Application() :
		m_lastFrameTime(0.f)
	{
#ifdef FIREFLY_WINDOWS
		m_window = std::make_unique<WindowsWindow>();
#endif

		m_window->SetEventCallback(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		m_lastFrameTime = (float)glfwGetTime();
		while (m_isRunning)
		{
			float time = (float)glfwGetTime();
			float deltaTime = time - m_lastFrameTime;
			m_lastFrameTime = time;

			m_window->SetTitle(std::to_string(1.f / deltaTime));

			OnUpdate(deltaTime);
			m_window->OnUpdate();
		}
	}

	void Application::OnEvent(std::shared_ptr<Event> event)
	{
		Input::OnEvent(event);

		if (auto windowEvent = event->AsType<WindowEvent>())
		{
			OnWindowEvent(windowEvent);

			if (auto closeEvent = windowEvent->AsType<WindowCloseEvent>())
			{
				m_isRunning = false;
			}
			else if (auto resizeEvent = windowEvent->AsType<WindowResizeEvent>())
			{
				int width = resizeEvent->GetWidth();
				int height = resizeEvent->GetHeight();

				if (width != 0 && height != 0)
					RenderingAPI::GetRenderFunctions()->SetViewport(0, 0, width, height);
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