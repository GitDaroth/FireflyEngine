#pragma once

namespace Firefly
{
	class IndexBuffer
	{
	public:
		IndexBuffer() {};
		virtual ~IndexBuffer() {};

		virtual void Init(uint32_t* indices, uint32_t size) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;
	};
}