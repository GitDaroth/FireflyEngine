#include "pch.h"
#include "Rendering/Mesh.h"

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		std::shared_ptr<VertexBuffer> vertexBuffer = RenderingAPI::CreateVertexBuffer();

		vertexBuffer->Init((float*)vertices.data(), vertices.size() * sizeof(Vertex));

		VertexBuffer::Layout layout = {
			{ ShaderDataType::Float3, "position" },
			{ ShaderDataType::Float3, "normal" },
			{ ShaderDataType::Float2, "texCoords" }
		};
		vertexBuffer->SetLayout(layout);

		std::shared_ptr<IndexBuffer> indexBuffer = RenderingAPI::CreateIndexBuffer();
		indexBuffer->Init(indices.data(), indices.size() * sizeof(uint32_t));

		m_vertexArray = RenderingAPI::CreateVertexArray();
		m_vertexArray->Init();
		m_vertexArray->AddVertexBuffer(vertexBuffer);
		m_vertexArray->SetIndexBuffer(indexBuffer);
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Bind()
	{
		m_vertexArray->Bind();
	}

	uint32_t Mesh::GetIndexCount() const
	{
		return m_vertexArray->GetIndexBuffer()->GetCount();
	}
}