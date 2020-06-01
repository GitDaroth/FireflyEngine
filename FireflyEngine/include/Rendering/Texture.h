#pragma once

namespace Firefly
{
	class Texture2D
	{
	public:
		Texture2D() {}
		virtual ~Texture2D() {}

		virtual void Init(const std::string& path) = 0;
		virtual void Bind(uint32_t slot) = 0;

		inline uint32_t GetWidth() { return m_width; }
		inline uint32_t GetHeight() { return m_height; }

	protected:
		uint32_t m_width;
		uint32_t m_height;
	};
}