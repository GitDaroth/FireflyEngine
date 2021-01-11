//#include "pch.h"
//#include "Rendering/OpenGL/OpenGLContext.h"
//
//#include <GLFW/glfw3.h>
//#include <glad/glad.h>
//
//#include <memory>
//
//namespace Firefly
//{
//	OpenGLContext::OpenGLContext() :
//		m_glfwWindow(nullptr)
//	{
//	}
//
//	OpenGLContext::~OpenGLContext()
//	{
//	}
//
//	void OpenGLContext::Init(void* window)
//	{
//		m_glfwWindow = (GLFWwindow*)(window);
//
//		glfwMakeContextCurrent(m_glfwWindow);
//		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
//		FIREFLY_ASSERT(status, "Unable to initialize glad!");
//
//		Logger::Info("Firefly Engine", "OpenGL Info:");
//		Logger::Info("Firefly Engine", "  -> Version: {0}", glGetString(GL_VERSION));
//		Logger::Info("Firefly Engine", "  -> Vendor: {0}", glGetString(GL_VENDOR));
//		Logger::Info("Firefly Engine", "  -> GPU: {0}", glGetString(GL_RENDERER));
//	}
//
//	void OpenGLContext::SwapBuffers()
//	{
//		glfwSwapBuffers(m_glfwWindow);
//	}
//}