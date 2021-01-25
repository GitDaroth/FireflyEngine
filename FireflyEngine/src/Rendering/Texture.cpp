#include "pch.h"
#include "Rendering/Texture.h"

#include <stb_image.h>

namespace Firefly
{
	Texture::Texture()
	{
	}

	void Texture::Init(const std::string& path, bool useLinearColorSpace)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);

		void* pixelData = nullptr;
		if (stbi_is_hdr(path.c_str()))
		{
			pixelData = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb);
			m_description.format = Format::RGB_32_FLOAT;
		}
		else
		{
			pixelData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			if(useLinearColorSpace)
				m_description.format = Format::RGBA_8;
			else
				m_description.format = Format::RGBA_8_NON_LINEAR;
		}

		FIREFLY_ASSERT(pixelData, "Failed to load image: " + path);

		m_description.type = Type::TEXTURE_2D;
		m_description.width = width;
		m_description.height = height;
		m_description.sampleCount = SampleCount::SAMPLE_1;
		m_description.useSampler = true;
		m_description.sampler.isMipMappingEnabled = true;
		m_description.sampler.isAnisotropicFilteringEnabled = true;
		m_description.sampler.maxAnisotropy = 16;
		m_description.sampler.wrapMode = WrapMode::REPEAT;
		m_description.sampler.magnificationFilterMode = FilterMode::LINEAR;
		m_description.sampler.minificationFilterMode = FilterMode::LINEAR;
		m_description.sampler.mipMapFilterMode = FilterMode::LINEAR;

		m_mipMapLevels = CalcMipMapLevels(m_description.width, m_description.height, m_description.depth);

		OnInit(pixelData);

		stbi_image_free(pixelData);
	}

	void Texture::Init(const Description& description)
	{
		m_description = description;
		m_mipMapLevels = CalcMipMapLevels(m_description.width, m_description.height, m_description.depth);
		OnInit(nullptr);
	}

	uint32_t Texture::GetWidth()
	{
		return m_description.width;
	}

	uint32_t Texture::GetHeight()
	{
		return m_description.height;
	}

	uint32_t Texture::CalcMipMapLevels(uint32_t width, uint32_t height, uint32_t depth)
	{
		return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height, depth)))) + 1;
	}
}