#pragma once

#include "GraphicsContext.h"

struct GLFWwindow;

namespace Firefly
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* glfwWindow);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_glfwWindow;
	};
}