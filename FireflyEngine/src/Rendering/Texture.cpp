#include "pch.h"
#include "Rendering/Texture.h"

#include <stb_image.h>

namespace Firefly
{
	Texture::Texture() :
		m_width(0),
		m_height(0)
	{
	}

	void Texture::Init(const std::string& path, ColorSpace colorSpace)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* pixelData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		FIREFLY_ASSERT(pixelData, "Failed to load image: " + path);

		m_width = width;
		m_height = height;

		OnInit(pixelData, colorSpace);

		stbi_image_free(pixelData);
	}

	uint32_t Texture::GetWidth()
	{
		return m_width;
	}

	uint32_t Texture::GetHeight()
	{
		return m_height;
	}
}