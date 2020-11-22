#pragma once

#include "Rendering/IndexBuffer.h"

namespace Firefly
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer();
		virtual ~OpenGLIndexBuffer() override;

		virtual void Init(uint32_t* indices, uint32_t size) override;
		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const override;

	private:
		uint32_t m_id;
		uint32_t m_count;
	};
}