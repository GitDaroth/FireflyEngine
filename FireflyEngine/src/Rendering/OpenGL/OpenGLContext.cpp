#include "pch.h"
#include "Rendering/OpenGL/OpenGLContext.h"

#include "Window/WindowsWindow.h"

namespace Firefly
{
	void OpenGLContext::OnInit(std::shared_ptr<Window> window)
	{
		std::shared_ptr<WindowsWindow> windowsWindow = std::dynamic_pointer_cast<WindowsWindow>(window);
		FIREFLY_ASSERT(windowsWindow, "Vulkan requires a Windows window!");
		m_glfwWindow = (GLFWwindow*)(windowsWindow->GetGlfwWindow());
		FIREFLY_ASSERT(m_glfwWindow, "Vulkan requires a GLFWwindow!");

		glfwMakeContextCurrent(m_glfwWindow);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		FIREFLY_ASSERT(status, "Unable to initialize glad!");

#ifndef NDEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(DebugMessengerCallback, 0);
#endif

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		glEnable(GL_DEPTH_TEST);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.f);

		glViewport(0, 0, window->GetWidth(), window->GetHeight());

		glfwSwapInterval(1); // 1: vsync, 0: unlimited

		PrintGpuInfo();
	}

	void OpenGLContext::Destroy()
	{
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_glfwWindow);
	}

	void OpenGLContext::PrintGpuInfo()
	{
		Logger::Info("OpenGL", "API Version: {0}", glGetString(GL_VERSION));
		Logger::Info("OpenGL", "{0} {1}", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
	}

	void GLAPIENTRY OpenGLContext::DebugMessengerCallback(
		GLenum source, GLenum type, GLuint id, 
		GLenum severity, GLsizei length, 
		const GLchar* message, const void* userParam)
	{
		std::string messageTypeLabel = "unknown";
		switch (type)
		{
		case GL_DEBUG_TYPE_OTHER:
			messageTypeLabel = "other";
			break;
		case GL_DEBUG_TYPE_ERROR:
			messageTypeLabel = "error";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			messageTypeLabel = "performance";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			messageTypeLabel = "deprecated";
			break;
		}

		std::string sourceLabel = "";
		switch (source)
		{
		case GL_DEBUG_SOURCE_API:
			sourceLabel = "API";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			sourceLabel = "App";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			sourceLabel = "Shader Compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			sourceLabel = "Third Party";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			sourceLabel = "Window System";
			break;
		}

		std::string outputMessage = "[" + sourceLabel + ": " + messageTypeLabel + "(" + std::to_string(id) + ")] debug message: " + std::string(message);

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			break;
		case GL_DEBUG_SEVERITY_LOW:
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			Logger::Warn("OpenGL", outputMessage);
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			Logger::Error("OpenGL", outputMessage);
			break;
		}
	}
}