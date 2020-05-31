#pragma once

#include "Rendering/GraphicsContext.h"

struct GLFWwindow;

namespace Firefly
{
	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext();
		virtual ~OpenGLContext() override;

		virtual void Init(void* window) override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_glfwWindow;
	};
}