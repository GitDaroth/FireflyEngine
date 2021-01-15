#pragma once

#include "Rendering/GraphicsContext.h"

namespace Firefly
{
	class Texture
	{
	public:
		enum class ColorSpace
		{
			RGB,
			SRGB
		};

		Texture(std::shared_ptr<GraphicsContext> context);

		void Init(const std::string& path, ColorSpace colorSpace = ColorSpace::RGB);
		virtual void Destroy() = 0;

		uint32_t GetWidth();
		uint32_t GetHeight();

	protected:
		virtual void OnInit(unsigned char* pixelData, ColorSpace colorSpace) = 0;

		std::shared_ptr<GraphicsContext> m_context;
		uint32_t m_width;
		uint32_t m_height;
	};
}