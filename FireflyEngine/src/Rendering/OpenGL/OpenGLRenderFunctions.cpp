#include "pch.h"
#include "Rendering/OpenGL/OpenGLRenderFunctions.h"

#include <glad/glad.h>

namespace Firefly
{
	void GLAPIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		if(type == GL_DEBUG_TYPE_ERROR)
			Logger::Error("FireflyEngine", "OpenGL error: {0}", message);
	}

	void OpenGLRenderFunctions::Init()
	{
		#ifdef DEBUG_MODE
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(OpenGLDebugCallback, 0);
		#endif

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_SHADING);
		glMinSampleShading(1.f);
	}

	void OpenGLRenderFunctions::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRenderFunctions::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderFunctions::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRenderFunctions::DrawIndexed(uint32_t indexCount)
	{
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	}
}