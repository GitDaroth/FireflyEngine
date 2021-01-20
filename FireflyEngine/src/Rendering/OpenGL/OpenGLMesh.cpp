#include "pch.h"
#include "Rendering/OpenGL/OpenGLMesh.h"

#include <glad/glad.h>

namespace Firefly
{
	OpenGLMesh::OpenGLMesh() :
		m_vertexArray(0),
		m_vertexBuffer(0),
		m_indexBuffer(0)
	{
	}

	void OpenGLMesh::Destroy()
	{
		glDeleteBuffers(1, &m_indexBuffer);
		glDeleteBuffers(1, &m_vertexBuffer);
		glDeleteVertexArrays(1, &m_vertexArray);
	}

	void OpenGLMesh::Bind()
	{
		glBindVertexArray(m_vertexArray);
	}

	void OpenGLMesh::OnInit(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		glGenVertexArrays(1, &m_vertexArray);
		glBindVertexArray(m_vertexArray);

		glGenBuffers(1, &m_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Mesh::Vertex) * vertices.size(), (float*)vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &m_indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

		GLsizei stride = sizeof(Mesh::Vertex);
		glEnableVertexAttribArray(0); 
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Mesh::Vertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Mesh::Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Mesh::Vertex, tangent));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Mesh::Vertex, bitangent));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Mesh::Vertex, texCoords));
	}
}