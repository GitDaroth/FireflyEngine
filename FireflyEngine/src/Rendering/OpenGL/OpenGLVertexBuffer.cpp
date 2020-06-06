#include "pch.h"
#include "Rendering/OpenGL/OpenGLVertexBuffer.h"

#include <glad/glad.h>

namespace Firefly
{
	OpenGLVertexBuffer::OpenGLVertexBuffer()
	{
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	void OpenGLVertexBuffer::Init(float* vertices, uint32_t size)
	{
		glGenBuffers(1, &m_id);
		Bind();
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}