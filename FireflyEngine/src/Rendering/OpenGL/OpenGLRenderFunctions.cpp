#include "pch.h"
#include "Rendering/OpenGL/OpenGLRenderFunctions.h"

#include <glad/glad.h>

namespace Firefly
{
	void OpenGLRenderFunctions::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRenderFunctions::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderFunctions::DrawIndexed(std::shared_ptr<VertexArray> vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}