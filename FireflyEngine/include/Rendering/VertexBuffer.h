#pragma once

namespace Firefly
{
	class VertexBuffer
	{
	public:
		VertexBuffer() {};
		virtual ~VertexBuffer() {};

		virtual void Init(float* vertices, uint32_t size) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
	};
}