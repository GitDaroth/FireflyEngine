#include "pch.h"
#include "Rendering/OpenGL/OpenGLTexture.h"

namespace Firefly
{
	OpenGLTexture::OpenGLTexture() :
		Texture(),
		m_texture(0),
		m_textureView(0),
		m_sampler(0),
		m_baseFormat(GL_NONE),
		m_internalFormat(GL_NONE),
		m_pixelDataType(GL_NONE),
		m_textureType(GL_NONE),
		m_sampleCount(1)
	{
	}

	void OpenGLTexture::Destroy()
	{
		if (m_description.useSampler)
			DestroySampler();
		DestroyTextureView();
		DestroyTexture();
	}

	void OpenGLTexture::Bind(GLuint slot)
	{
		glBindTextureUnit(slot, m_textureView);
		if (m_description.useSampler)
			glBindSampler(slot, m_sampler);
	}

	uint32_t OpenGLTexture::GetHandle() const
	{
		return m_textureView;
	}

	void OpenGLTexture::OnInit(void* pixelData)
	{
		m_baseFormat = ConvertToOpenGLBaseFormat(m_description.format);
		m_internalFormat = ConvertToOpenGLInternalFormat(m_description.format);
		m_pixelDataType = GetOpenGLPixelDataType(m_description.format);
		m_sampleCount = ConvertToSampleCountNumber(m_description.sampleCount);
		m_textureType = ConvertToOpenGLTextureType(m_description.type, m_sampleCount);

		CreateTexture(pixelData);
		CreateTextureView();
		if (m_description.useSampler)
			CreateSampler();
	}

	void OpenGLTexture::CreateTexture(void* pixelData)
	{
		bool wasPixelDataInitiallyEmpty = pixelData == nullptr;
		if (wasPixelDataInitiallyEmpty)
			pixelData = malloc(GetTextureByteSize(m_description.width, m_description.height, m_description.type, m_description.format));

		glCreateTextures(m_textureType, 1, &m_texture);

		switch (m_textureType)
		{
		case GL_TEXTURE_2D:
			glTextureStorage2D(m_texture, m_mipMapLevels, m_internalFormat, m_description.width, m_description.height);
			glTextureSubImage2D(m_texture, 0, 0, 0, m_description.width, m_description.height, m_baseFormat, m_pixelDataType, pixelData);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE:
			glTextureStorage2DMultisample(m_texture, m_sampleCount, m_internalFormat, m_description.width, m_description.height, GL_TRUE);
			glTextureSubImage2D(m_texture, 0, 0, 0, m_description.width, m_description.height, m_baseFormat, m_pixelDataType, pixelData);
			break;
		case GL_TEXTURE_CUBE_MAP:
			glTextureStorage2D(m_texture, m_mipMapLevels, m_internalFormat, m_description.width, m_description.height);
			for (size_t i = 0; i < 6; i++)
			{
				uint32_t offset = i * m_description.width * m_description.height * GetBytePerPixel(m_description.format);
				void* offsetPixelData = (reinterpret_cast<unsigned char*>(pixelData) + offset);
				glTextureSubImage3D(m_texture, 0, 0, 0, i, m_description.width, m_description.height, 1, m_baseFormat, m_pixelDataType, offsetPixelData);
			}
			break;
		}

		glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, m_mipMapLevels - 1);

		if (m_description.useSampler && m_description.sampler.isMipMappingEnabled)
			glGenerateTextureMipmap(m_texture);

		if (wasPixelDataInitiallyEmpty)
			free(pixelData);
	}

	void OpenGLTexture::DestroyTexture()
	{
		glDeleteTextures(1, &m_texture);
	}

	void OpenGLTexture::CreateTextureView()
	{
		glGenTextures(1, &m_textureView);
		glTextureView(m_textureView, m_textureType, m_texture, m_internalFormat, 0, m_mipMapLevels, 0, m_arrayLayers);
	}

	void OpenGLTexture::DestroyTextureView()
	{
		glDeleteTextures(1, &m_textureView);
	}

	void OpenGLTexture::CreateSampler()
	{
		glCreateSamplers(1, &m_sampler);

		float maxAnisotropy = 1.0f;
		if (m_description.sampler.isAnisotropicFilteringEnabled)
		{
			GLfloat maxAnisotropyLimit;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropyLimit);

			if (m_description.sampler.maxAnisotropy > maxAnisotropyLimit)
			{
				Logger::Warn("OpenGL", "Max. anisotropy ({0}) is bigger than the device limit ({1}).", m_description.sampler.maxAnisotropy, maxAnisotropyLimit);
				m_description.sampler.maxAnisotropy = maxAnisotropyLimit;
			}
			maxAnisotropy = m_description.sampler.maxAnisotropy;
		}

		std::vector<float> borderColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		GLenum wrapMode = ConvertToOpenGLWrapMode(m_description.sampler.wrapMode);
		GLenum minFilterMode = ConvertToOpenGLFilterMode(m_description.sampler.minificationFilterMode);
		GLenum maxFilterMode = ConvertToOpenGLFilterMode(m_description.sampler.magnificationFilterMode);
		if (m_description.sampler.isMipMappingEnabled)
			minFilterMode = ConvertToOpenGLMinificationFilterMode(m_description.sampler.minificationFilterMode, m_description.sampler.mipMapFilterMode);

		glSamplerParameterf(m_sampler, GL_TEXTURE_MIN_LOD, 0.0f);
		glSamplerParameterf(m_sampler, GL_TEXTURE_MAX_LOD, (float)(m_mipMapLevels - 1));
		glSamplerParameterf(m_sampler, GL_TEXTURE_LOD_BIAS, 0.0f);
		glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_S, wrapMode);
		glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_T, wrapMode);
		glSamplerParameteri(m_sampler, GL_TEXTURE_WRAP_R, wrapMode);
		glSamplerParameterf(m_sampler, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
		glSamplerParameteri(m_sampler, GL_TEXTURE_MIN_FILTER, minFilterMode);
		glSamplerParameteri(m_sampler, GL_TEXTURE_MAG_FILTER, maxFilterMode);
		glSamplerParameteri(m_sampler, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_FALSE);
		glSamplerParameterfv(m_sampler, GL_TEXTURE_BORDER_COLOR, borderColor.data());
		glSamplerParameteri(m_sampler, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glSamplerParameteri(m_sampler, GL_TEXTURE_COMPARE_FUNC, GL_ALWAYS);
		if (m_description.type == Texture::Type::TEXTURE_CUBE_MAP)
			glSamplerParameteri(m_sampler, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_TRUE);
	}

	void OpenGLTexture::DestroySampler()
	{
		glDeleteSamplers(1, &m_sampler);
	}

	GLenum OpenGLTexture::ConvertToOpenGLBaseFormat(Format format)
	{
		switch (format)
		{
		case Format::R_8:
		case Format::R_8_NON_LINEAR:
		case Format::R_16_FLOAT:
		case Format::R_32_FLOAT:
			return GL_RED;
		case Format::RG_8:
		case Format::RG_8_NON_LINEAR:
		case Format::RG_16_FLOAT:
		case Format::RG_32_FLOAT:
			return GL_RG;
		case Format::RGB_8:
		case Format::RGB_8_NON_LINEAR:
		case Format::RGB_16_FLOAT:
		case Format::RGB_32_FLOAT:
			return GL_RGB;
		case Format::RGBA_8:
		case Format::RGBA_8_NON_LINEAR:
		case Format::RGBA_16_FLOAT:
		case Format::RGBA_32_FLOAT:
			return GL_RGBA;
		case Format::DEPTH_32_FLOAT:
			return GL_DEPTH_COMPONENT;
		case Format::DEPTH_24_STENCIL_8:
			return GL_DEPTH_STENCIL;
		}
	}

	GLenum OpenGLTexture::ConvertToOpenGLInternalFormat(Format format)
	{
		switch (format)
		{
		case Format::R_8:
			return GL_R8;
		case Format::R_8_NON_LINEAR:
			Logger::Warn("OpenGL", "Non linear format with only 1 component is not supported -> linear format GL_R8 is used instead");
			return GL_R8;
		case Format::R_16_FLOAT:
			return GL_R16F;
		case Format::R_32_FLOAT:
			return GL_R32F;
		case Format::RG_8:
			return GL_RG8;
		case Format::RG_8_NON_LINEAR:
			Logger::Warn("OpenGL", "Non linear format with only 2 components is not supported -> linear format GL_RG8 is used instead");
			return GL_RG8;
		case Format::RG_16_FLOAT:
			return GL_RG16F;
		case Format::RG_32_FLOAT:
			return GL_RG32F;
		case Format::RGB_8:
			return GL_RGB8;
		case Format::RGB_8_NON_LINEAR:
			return GL_SRGB8;
		case Format::RGB_16_FLOAT:
			return GL_RGB16F;
		case Format::RGB_32_FLOAT:
			return GL_RGB32F;
		case Format::RGBA_8:
			return GL_RGBA8;
		case Format::RGBA_8_NON_LINEAR:
			return GL_SRGB8_ALPHA8;
		case Format::RGBA_16_FLOAT:
			return GL_RGBA16F;
		case Format::RGBA_32_FLOAT:
			return GL_RGBA32F;
		case Format::DEPTH_32_FLOAT:
			return GL_DEPTH_COMPONENT32F;
		case Format::DEPTH_24_STENCIL_8:
			return GL_DEPTH24_STENCIL8;
		}
	}

	GLenum OpenGLTexture::GetOpenGLPixelDataType(Format format)
	{
		switch (format)
		{
		case Format::R_8:
		case Format::R_8_NON_LINEAR:
		case Format::RG_8:
		case Format::RG_8_NON_LINEAR:
		case Format::RGB_8:
		case Format::RGB_8_NON_LINEAR:
		case Format::RGBA_8:
		case Format::RGBA_8_NON_LINEAR:
			return GL_UNSIGNED_BYTE;
		case Format::R_16_FLOAT:
		case Format::RG_16_FLOAT:
		case Format::RGB_16_FLOAT:
		case Format::RGBA_16_FLOAT:
			return GL_HALF_FLOAT;
		case Format::R_32_FLOAT:
		case Format::RG_32_FLOAT:
		case Format::RGB_32_FLOAT:
		case Format::RGBA_32_FLOAT:
		case Format::DEPTH_32_FLOAT:
			return GL_FLOAT;
		case Format::DEPTH_24_STENCIL_8:
			return GL_UNSIGNED_INT_24_8;
		}
	}

	GLenum OpenGLTexture::ConvertToOpenGLTextureType(Type type, uint32_t sampleCount)
	{
		if (type != Type::TEXTURE_2D && sampleCount > 1)
			Logger::Warn("OpenGL", "Sample count > 1 is ignored for non TEXTURE_2D types");

		switch (type)
		{
		case Type::TEXTURE_2D:
			if (sampleCount == 1)
				return GL_TEXTURE_2D;
			else
				return GL_TEXTURE_2D_MULTISAMPLE;
		case Type::TEXTURE_CUBE_MAP:
			return GL_TEXTURE_CUBE_MAP;
		}
	}

	GLenum OpenGLTexture::ConvertToOpenGLWrapMode(WrapMode wrapMode)
	{
		switch (wrapMode)
		{
		case WrapMode::REPEAT:
			return GL_REPEAT;
		case WrapMode::MIRRORED_REPEAT:
			return GL_MIRRORED_REPEAT;
		case WrapMode::CLAMP_TO_EDGE:
			return GL_CLAMP_TO_EDGE;
		case WrapMode::MIRROR_CLAMP_TO_EDGE:
			return GL_MIRROR_CLAMP_TO_EDGE;
		case WrapMode::CLAMP_TO_BORDER:
			return GL_CLAMP_TO_BORDER;
		}
	}

	GLenum OpenGLTexture::ConvertToOpenGLMinificationFilterMode(FilterMode minFilterMode, FilterMode mipMapFilterMode)
	{
		switch (minFilterMode)
		{
		case FilterMode::NEAREST:
			switch (mipMapFilterMode)
			{
			case FilterMode::NEAREST:
				return GL_NEAREST_MIPMAP_NEAREST;
			case FilterMode::LINEAR:
				return GL_NEAREST_MIPMAP_LINEAR;
			}
		case FilterMode::LINEAR:
			switch (mipMapFilterMode)
			{
			case FilterMode::NEAREST:
				return GL_LINEAR_MIPMAP_NEAREST;
			case FilterMode::LINEAR:
				return GL_LINEAR_MIPMAP_LINEAR;
			}
		}
	}

	GLenum OpenGLTexture::ConvertToOpenGLFilterMode(FilterMode filterMode)
	{
		switch (filterMode)
		{
		case FilterMode::NEAREST:
			return GL_NEAREST;
		case FilterMode::LINEAR:
			return GL_LINEAR;
		}
	}

	size_t OpenGLTexture::GetTextureByteSize(uint32_t width, uint32_t height, Type type, Format format)
	{
		size_t textureSize = width * height * GetBytePerPixel(format);
		if (type == Texture::Type::TEXTURE_CUBE_MAP)
			textureSize *= 6;
		return textureSize;
	}
}