#include "pch.h"
#include "Window/WindowsWindow.h"

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
		}

		m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_window);
		EnableVSync(true);

		s_windowCount++;
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
		glfwSetWindowSize(m_window, width, height);
	}

	void WindowsWindow::OnEnableVSync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
	}
}