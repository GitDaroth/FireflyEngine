#pragma once

#include "Rendering/VertexBuffer.h"

namespace Firefly
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer();
		virtual ~OpenGLVertexBuffer() override;

		virtual void Init(float* vertices, uint32_t size) override;
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		uint32_t m_id;
	};
}