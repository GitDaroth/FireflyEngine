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
		bool wasPixelDataInitiallyEmpty = pixelData == nullptr;

		GLenum baseFormat = ConvertToOpenGLBaseFormat(m_description.format);
		GLenum internalFormat = ConvertToOpenGLInternalFormat(m_description.format);
		GLenum pixelDataType = GetOpenGLPixelDataType(m_description.format);
		GLsizei sampleCount = ConvertToSampleCountNumber(m_description.sampleCount);
		GLenum textureType = ConvertToOpenGLTextureType(m_description.type, sampleCount);
		size_t textureSize = m_description.width * m_description.height * GetBytePerPixel(m_description.format);

		glCreateTextures(textureType, 1, &m_texture);

		switch (textureType)
		{
		case GL_TEXTURE_2D:
			if (wasPixelDataInitiallyEmpty)
				pixelData = malloc(textureSize);

			glTextureStorage2D(m_texture, m_mipMapLevels, internalFormat, m_description.width, m_description.height);
			glTextureSubImage2D(m_texture, 0, 0, 0, m_description.width, m_description.height, baseFormat, pixelDataType, pixelData);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE:
			if (wasPixelDataInitiallyEmpty)
				pixelData = malloc(textureSize);

			glTextureStorage2DMultisample(m_texture, sampleCount, internalFormat, m_description.width, m_description.height, GL_TRUE);
			glTextureSubImage2D(m_texture, 0, 0, 0, m_description.width, m_description.height, baseFormat, pixelDataType, pixelData);
			break;
		case GL_TEXTURE_CUBE_MAP:
			if (wasPixelDataInitiallyEmpty)
				pixelData = malloc(textureSize * 6);

			glTextureStorage2D(m_texture, m_mipMapLevels, internalFormat, m_description.width, m_description.height);
			for (size_t i = 0; i < 6; i++)
			{
				uint32_t offset = i * textureSize;
				void* offsetPixelData = (reinterpret_cast<unsigned char*>(pixelData) + offset);
				glTextureSubImage3D(m_texture, 0, 0, 0, i, m_description.width, m_description.height, 1, baseFormat, pixelDataType, offsetPixelData);
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
				glTextureParameterf(m_texture, GL_TEXTURE_MAX_ANISOTROPY, m_description.sampler.maxAnisotropy);
			}

			GLenum wrapMode = ConvertToOpenGLWrapMode(m_description.sampler.wrapMode);
			GLenum minFilterMode = ConvertToOpenGLFilterMode(m_description.sampler.minificationFilterMode);
			GLenum maxFilterMode = ConvertToOpenGLFilterMode(m_description.sampler.magnificationFilterMode);

			if (m_description.sampler.isMipMappingEnabled)
			{
				glGenerateTextureMipmap(m_texture);

				glTextureParameteri(m_texture, GL_TEXTURE_BASE_LEVEL, 0);
				glTextureParameteri(m_texture, GL_TEXTURE_MAX_LEVEL, m_mipMapLevels - 1);
				glTextureParameterf(m_texture, GL_TEXTURE_MIN_LOD, 0.0f);
				glTextureParameterf(m_texture, GL_TEXTURE_MAX_LOD, (float)(m_mipMapLevels - 1));
				glTextureParameterf(m_texture, GL_TEXTURE_LOD_BIAS, 0.0f);
				minFilterMode = ConvertToOpenGLMinificationFilterMode(m_description.sampler.minificationFilterMode, m_description.sampler.mipMapFilterMode);
			}

			glTextureParameteri(m_texture, GL_TEXTURE_WRAP_S, wrapMode);
			glTextureParameteri(m_texture, GL_TEXTURE_WRAP_T, wrapMode);
			glTextureParameteri(m_texture, GL_TEXTURE_WRAP_R, wrapMode);
			glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, minFilterMode);
			glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, maxFilterMode);
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