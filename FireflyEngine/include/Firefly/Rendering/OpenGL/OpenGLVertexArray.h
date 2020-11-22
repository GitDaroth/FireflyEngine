#pragma once

#include "Rendering/VertexArray.h"

namespace Firefly
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray() override;

		virtual void Init() override;
		virtual void Bind() const override;
		virtual void Unbind() const override;

	protected:
		virtual void OnAddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) override;

	private:
		uint32_t m_id;
	};
}