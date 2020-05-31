#include "pch.h"
#include "Rendering/OpenGL/OpenGLIndexBuffer.h"

#include <glad/glad.h>

namespace Firefly
{
	OpenGLIndexBuffer::OpenGLIndexBuffer()
	{
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	void OpenGLIndexBuffer::Init(uint32_t* indices, uint32_t size)
	{
		m_count = size / sizeof(uint32_t);
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	uint32_t OpenGLIndexBuffer::GetCount() const
	{
		return m_count;
	}
}