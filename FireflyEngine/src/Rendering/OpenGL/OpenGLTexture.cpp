#include "pch.h"
#include "Rendering/OpenGL/OpenGLTexture.h"

namespace Firefly
{
	OpenGLTexture::OpenGLTexture() :
		Texture(),
		m_texture(0)
	{
	}

	void OpenGLTexture::Destroy()
	{
		glDeleteTextures(1, &m_texture);
	}

	void OpenGLTexture::Bind(GLuint slot)
	{
		glBindTextureUnit(slot, m_texture);
	}

	uint32_t OpenGLTexture::GetHandle() const
	{
		return m_texture;
	}

	void OpenGLTexture::OnInit(void* pixelData)
	{
		GLenum baseFormat = ConvertToOpenGLBaseFormat(m_description.format);
		GLenum internalFormat = ConvertToOpenGLInternalFormat(m_description.format);
		GLenum pixelDataType = GetOpenGLPixelDataType(m_description.format);
		GLsizei sampleCount = ConvertToSampleCountNumber(m_description.sampleCount);
		GLenum textureType = ConvertToOpenGLTextureType(m_description.type, sampleCount);

		glGenTextures(1, &m_texture);
		glBindTexture(textureType, m_texture);

		switch (textureType)
		{
		case GL_TEXTURE_2D:
			glTexImage2D(textureType, 0, internalFormat, m_description.width, m_description.height, 0, baseFormat, pixelDataType, pixelData);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE:
			glTexImage2DMultisample(textureType, sampleCount, internalFormat, m_description.width, m_description.height, GL_TRUE);
			break;
		case GL_TEXTURE_CUBE_MAP:
			uint32_t textureSize = m_description.width * m_description.height * GetBytePerPixel(m_description.format);
			for (size_t i = 0; i < 6; i++)
			{
				void* offsetPixelData = nullptr;
				if (pixelData)
				{
					uint32_t offset = i * textureSize;
					offsetPixelData = (reinterpret_cast<unsigned char*>(pixelData) + offset);
				}

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, m_description.width, m_description.height, 0, baseFormat, pixelDataType, offsetPixelData);
			}
			break;
		}

		if (m_description.useSampler)
		{
			if (m_description.sampler.isAnisotropicFilteringEnabled)
			{
				GLfloat maxAnisotropy;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);

				if (m_description.sampler.maxAnisotropy > maxAnisotropy)
				{
					Logger::Warn("OpenGL", "Max. anisotropy ({0}) is bigger than the device limit ({1}).", m_description.sampler.maxAnisotropy, maxAnisotropy);
					m_description.sampler.maxAnisotropy = maxAnisotropy;
				}
				glTexParameterf(textureType, GL_TEXTURE_MAX_ANISOTROPY, m_description.sampler.maxAnisotropy);
			}

			GLenum wrapMode = ConvertToOpenGLWrapMode(m_description.sampler.wrapMode);
			GLenum minFilterMode = ConvertToOpenGLFilterMode(m_description.sampler.minificationFilterMode);
			GLenum maxFilterMode = ConvertToOpenGLFilterMode(m_description.sampler.magnificationFilterMode);

			if (m_description.sampler.isMipMappingEnabled)
			{
				glGenerateMipmap(textureType);

				glTexParameteri(textureType, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(textureType, GL_TEXTURE_MAX_LEVEL, m_mipMapLevels - 1);
				minFilterMode = ConvertToOpenGLMinificationFilterMode(m_description.sampler.minificationFilterMode, m_description.sampler.mipMapFilterMode);
			}

			glTexParameteri(textureType, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(textureType, GL_TEXTURE_WRAP_T, wrapMode);
			glTexParameteri(textureType, GL_TEXTURE_WRAP_R, wrapMode);
			glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, minFilterMode);
			glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, maxFilterMode);
		}
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
}