#include "pch.h"
#include "Rendering/Texture.h"

#include <stb_image.h>

namespace Firefly
{
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

		m_mipMapLevels = CalcMipMapLevels(m_description.width, m_description.height);

		OnInit(pixelData);

		stbi_image_free(pixelData);
	}

	void Texture::Init(const Texture::Description& description)
	{
		m_description = description;
		m_mipMapLevels = CalcMipMapLevels(m_description.width, m_description.height);
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

	Texture::Type Texture::GetType() const
	{
		return m_description.type;
	}

	Texture::SampleCount Texture::GetSampleCount() const
	{
		return m_description.sampleCount;
	}

	uint32_t Texture::GetMipMapLevels() const
	{
		return m_mipMapLevels;
	}

	Texture::Format Texture::GetFormat() const
	{
		return m_description.format;
	}

	std::string Texture::GetFormatAsString() const
	{
		return ConvertFormatToString(m_description.format);
	}

	std::string Texture::ConvertFormatToString(Texture::Format format)
	{
		std::string formatName = "";

		switch (format)
		{
		case Format::R_8:
			formatName = "R_8";
			break;
		case Format::R_8_NON_LINEAR:
			formatName = "R_8_NON_LINEAR";
			break;
		case Format::R_16_FLOAT:
			formatName = "R_16_FLOAT";
			break;
		case Format::R_32_FLOAT:
			formatName = "R_32_FLOAT";
			break;
		case Format::RG_8:
			formatName = "RG_8";
			break;
		case Format::RG_8_NON_LINEAR:
			formatName = "RG_8_NON_LINEAR";
			break;
		case Format::RG_16_FLOAT:
			formatName = "RG_16_FLOAT";
			break;
		case Format::RG_32_FLOAT:
			formatName = "RG_32_FLOAT";
			break;
		case Format::RGB_8:
			formatName = "RGB_8";
			break;
		case Format::RGB_8_NON_LINEAR:
			formatName = "RGB_8_NON_LINEAR";
			break;
		case Format::RGB_16_FLOAT:
			formatName = "RGB_16_FLOAT";
			break;
		case Format::RGB_32_FLOAT:
			formatName = "RGB_32_FLOAT";
			break;
		case Format::RGBA_8:
			formatName = "RGBA_8";
			break;
		case Format::RGBA_8_NON_LINEAR:
			formatName = "RGBA_8_NON_LINEAR";
			break;
		case Format::RGBA_16_FLOAT:
			formatName = "RGBA_16_FLOAT";
			break;
		case Format::RGBA_32_FLOAT:
			formatName = "RGBA_32_FLOAT";
			break;
		case Format::DEPTH_32_FLOAT:
			formatName = "DEPTH_32_FLOAT";
			break;
		case Format::DEPTH_24_STENCIL_8:
			formatName = "DEPTH_24_STENCIL_8";
			break;
		}

		return formatName;
	}

	bool Texture::HasColorFormat() const
	{
		if (m_description.format != Format::DEPTH_32_FLOAT && m_description.format != Format::DEPTH_24_STENCIL_8)
			return true;
		return false;
	}

	bool Texture::HasDepthFormat() const
	{
		if (m_description.format == Format::DEPTH_32_FLOAT)
			return true;
		return false;
	}

	bool Texture::HasDepthStencilFormat() const
	{
		if (m_description.format == Format::DEPTH_24_STENCIL_8)
			return true;
		return false;
	}

	uint32_t Texture::CalcMipMapLevels(uint32_t width, uint32_t height)
	{
		return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	}
}