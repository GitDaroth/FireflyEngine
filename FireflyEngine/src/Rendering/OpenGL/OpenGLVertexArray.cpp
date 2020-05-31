#include "pch.h"
#include "Rendering/OpenGL/OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Firefly
{
	OpenGLVertexArray::OpenGLVertexArray()
	{
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_id);
	}

	void OpenGLVertexArray::Init()
	{
		glGenVertexArrays(1, &m_id);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_id);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::OnAddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer)
	{
		auto& layout = vertexBuffer->GetLayout();
		for (int i = 0; i < layout.GetElements().size(); i++)
		{
			auto element = layout.GetElements()[i];
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i,
				element.GetCount(),
				GL_FLOAT,
				element.IsNormalized(),
				layout.GetStride(),
				(const void*)element.GetOffset());
		}
	}
}