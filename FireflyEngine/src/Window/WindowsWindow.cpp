#include "pch.h"
#include "Window/WindowsWindow.h"

#include "Event/WindowEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

namespace Firefly
{
	bool WindowsWindow::s_isWindowInitialized = false;
	int WindowsWindow::s_windowCount = 0;

	WindowsWindow::WindowsWindow(const std::string& title, int width, int height) :
		Window(title, width, height)
	{
		Logger::Info("Firefly Engine", "Creating a Windows window: {0} ({1}x{2})", title, width, height);

		if (!s_isWindowInitialized)
		{
			int success = glfwInit();
			FIREFLY_ASSERT(success, "Unable to initialize GLFW!");
			s_isWindowInitialized = true;

			glfwSetErrorCallback([](int code, const char* description)
			{
				Logger::Error("FireflyEngine", "GLFW error [{0}]: {1}", code, description);
			});
		}

		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		s_windowCount++;
		glfwMakeContextCurrent(m_window);
		glfwSetWindowUserPointer(m_window, this);
		EnableVSync(true);

		SetupWindowEvents();
		SetupInputEvents();
	}

	WindowsWindow::~WindowsWindow()
	{
		glfwDestroyWindow(m_window);
		s_windowCount--;
		if (s_windowCount == 0)
			glfwTerminate();
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}

	void WindowsWindow::OnSetTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_window, title.c_str());
	}

	void WindowsWindow::OnSetSize(int width, int height)
	{
		int windowWidth, windowHeight;
		glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
		if(width != windowWidth || height != windowHeight)
			glfwSetWindowSize(m_window, width, height);
	}

	void WindowsWindow::OnEnableVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
	}

	void WindowsWindow::SetupWindowEvents()
	{
		glfwSetWindowSizeCallback(m_window, [](GLFWwindow* glfwWindow, int width, int height)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			window->SetSize(width, height);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<WindowResizeEvent>(width, height);
			eventCallback(event);
		});

		glfwSetWindowCloseCallback(m_window, [](GLFWwindow* glfwWindow)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<WindowCloseEvent>();
			eventCallback(event);
		});

		glfwSetWindowMaximizeCallback(m_window, [](GLFWwindow* glfwWindow, int maximized)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event;
			if (maximized == GLFW_TRUE)
				event = std::make_shared<WindowMaximizeEvent>();
			else
				event = std::make_shared<WindowRestoreEvent>();
			eventCallback(event);
		});

		glfwSetWindowIconifyCallback(m_window, [](GLFWwindow* glfwWindow, int iconified)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event;
			if (iconified == GLFW_TRUE)
				event = std::make_shared<WindowMinimizeEvent>();
			else
				event = std::make_shared<WindowRestoreEvent>();
			eventCallback(event);
		});

		glfwSetWindowPosCallback(m_window, [](GLFWwindow* glfwWindow, int xPos, int yPos)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<WindowMoveEvent>(xPos, yPos);
			eventCallback(event);
		});
	}

	void WindowsWindow::SetupInputEvents()
	{
		glfwSetKeyCallback(m_window, [](GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event;
			switch (action)
			{
				case GLFW_PRESS:
					event = std::make_shared<KeyPressEvent>(key);
					break;
				case GLFW_RELEASE:
					event = std::make_shared<KeyReleaseEvent>(key);
					break;
				case GLFW_REPEAT:
					event = std::make_shared<KeyRepeatEvent>(key);
					break;
			}
			eventCallback(event);
		});

		glfwSetMouseButtonCallback(m_window, [](GLFWwindow* glfwWindow, int button, int action, int mods)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event;
			switch (action)
			{
				case GLFW_PRESS:
					event = std::make_shared<MouseButtonPressEvent>(button);
					break;
				case GLFW_RELEASE:
					event = std::make_shared<MouseButtonReleaseEvent>(button);
					break;
			}
			eventCallback(event);
		});

		glfwSetScrollCallback(m_window, [](GLFWwindow* glfwWindow, double xOffset, double yOffset)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<MouseScrollEvent>((float)xOffset, (float)yOffset);
			eventCallback(event);
		});

		glfwSetCursorPosCallback(m_window, [](GLFWwindow* glfwWindow, double xPos, double yPos)
		{
			Window* window = (WindowsWindow*)glfwGetWindowUserPointer(glfwWindow);
			auto eventCallback = window->GetEventCallback();

			std::shared_ptr<Event> event = std::make_shared<MouseMoveEvent>((int)xPos, (int)yPos);
			eventCallback(event);
		});
	}
}