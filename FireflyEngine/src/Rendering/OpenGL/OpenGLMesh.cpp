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
        glCreateBuffers(1, &m_vertexBuffer);
        glNamedBufferData(m_vertexBuffer, sizeof(Mesh::Vertex) * vertices.size(), (float*)vertices.data(), GL_STATIC_DRAW);

        glCreateBuffers(1, &m_indexBuffer);
        glNamedBufferData(m_indexBuffer, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

        size_t bindingIndex = 0;
        size_t attributeIndex = 0;
        GLsizei stride = sizeof(Mesh::Vertex);
        glCreateVertexArrays(1, &m_vertexArray);
        glVertexArrayVertexBuffer(m_vertexArray, bindingIndex, m_vertexBuffer, 0, stride);
        glVertexArrayElementBuffer(m_vertexArray, m_indexBuffer);

        glEnableVertexArrayAttrib(m_vertexArray, attributeIndex);
        glVertexArrayAttribFormat(m_vertexArray, attributeIndex, 3, GL_FLOAT, GL_FALSE, offsetof(Mesh::Vertex, position));
        glVertexArrayAttribBinding(m_vertexArray, attributeIndex, bindingIndex);
        attributeIndex++;
        glEnableVertexArrayAttrib(m_vertexArray, attributeIndex);
        glVertexArrayAttribFormat(m_vertexArray, attributeIndex, 3, GL_FLOAT, GL_FALSE, offsetof(Mesh::Vertex, normal));
        glVertexArrayAttribBinding(m_vertexArray, attributeIndex, bindingIndex);
        attributeIndex++;
        glEnableVertexArrayAttrib(m_vertexArray, attributeIndex);
        glVertexArrayAttribFormat(m_vertexArray, attributeIndex, 3, GL_FLOAT, GL_FALSE, offsetof(Mesh::Vertex, tangent));
        glVertexArrayAttribBinding(m_vertexArray, attributeIndex, bindingIndex);
        attributeIndex++;
        glEnableVertexArrayAttrib(m_vertexArray, attributeIndex);
        glVertexArrayAttribFormat(m_vertexArray, attributeIndex, 3, GL_FLOAT, GL_FALSE, offsetof(Mesh::Vertex, bitangent));
        glVertexArrayAttribBinding(m_vertexArray, attributeIndex, bindingIndex);
        attributeIndex++;
        glEnableVertexArrayAttrib(m_vertexArray, attributeIndex);
        glVertexArrayAttribFormat(m_vertexArray, attributeIndex, 2, GL_FLOAT, GL_FALSE, offsetof(Mesh::Vertex, texCoords));
        glVertexArrayAttribBinding(m_vertexArray, attributeIndex, bindingIndex);
    }
}