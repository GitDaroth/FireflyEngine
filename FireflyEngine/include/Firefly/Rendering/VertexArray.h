#pragma once

#include "Rendering/VertexBuffer.h"
#include "Rendering/IndexBuffer.h"

namespace Firefly
{
	class VertexArray
	{
	public:
		VertexArray() {}
		virtual ~VertexArray() {}

		virtual void Init() = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		inline void AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) 
		{
			Bind();
			vertexBuffer->Bind();
			m_vertexBuffers.push_back(vertexBuffer);
			OnAddVertexBuffer(vertexBuffer);
		}
		inline void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer)
		{
			Bind();
			indexBuffer->Bind();
			m_indexBuffer = indexBuffer;
		}

		inline const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_vertexBuffers; }
		inline std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return m_indexBuffer; }

	protected:
		virtual void OnAddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) = 0;

		std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
		std::shared_ptr<IndexBuffer> m_indexBuffer;
	};
}