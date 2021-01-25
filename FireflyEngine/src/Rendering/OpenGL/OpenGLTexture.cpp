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

	void OpenGLTexture::OnInit(void* pixelData)
	{
		GLenum baseFormat = ConvertToOpenGLBaseFormat(m_description.format);
		GLenum internalFormat = ConvertToOpenGLInternalFormat(m_description.format);
        GLenum pixelDataType = GetOpenGLPixelDataType(m_description.format);
        GLsizei sampleCount = ConvertToOpenGLSampleCount(m_description.sampleCount);
        GLenum textureType = ConvertToOpenGLTextureType(m_description.type, sampleCount);

		glGenTextures(1, &m_texture);
		glBindTexture(textureType, m_texture);

        switch (textureType)
        {
        case GL_TEXTURE_1D:
            glTexImage1D(textureType, 0, internalFormat, m_description.width, 0, baseFormat, pixelDataType, pixelData);
            break;
        case GL_TEXTURE_2D:
            glTexImage2D(textureType, 0, internalFormat, m_description.width, m_description.height, 0, baseFormat, pixelDataType, pixelData);
            break;
        case GL_TEXTURE_2D_MULTISAMPLE:
            glTexImage2DMultisample(textureType, sampleCount, internalFormat, m_description.width, m_description.height, GL_TRUE);
            break;
        case GL_TEXTURE_3D:
            glTexImage3D(textureType, 0, internalFormat, m_description.width, m_description.height, m_description.depth, 0, baseFormat, pixelDataType, pixelData);
            break;
        case GL_TEXTURE_CUBE_MAP:
            uint32_t textureSize = m_description.width * m_description.height * GetBytePerPixel(m_description.format);
            for (size_t i = 0; i < 6; i++)
            {
                uint32_t offset = i * textureSize;
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, m_description.width, m_description.height, 0, baseFormat, pixelDataType, (reinterpret_cast<unsigned char*>(pixelData) + offset));
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
		GLenum baseFormat = GL_NONE;

        switch (format)
        {
        case Format::R_8:
        case Format::R_8_NON_LINEAR:
        case Format::R_16_FLOAT:
        case Format::R_32_FLOAT:
            baseFormat = GL_RED;
            break;
        case Format::RG_8:
        case Format::RG_8_NON_LINEAR:
        case Format::RG_16_FLOAT:
        case Format::RG_32_FLOAT:
            baseFormat = GL_RG;
            break;
        case Format::RGB_8:
        case Format::RGB_8_NON_LINEAR:
        case Format::RGB_16_FLOAT:
        case Format::RGB_32_FLOAT:
            baseFormat = GL_RGB;
            break;
        case Format::RGBA_8:
        case Format::RGBA_8_NON_LINEAR:
        case Format::RGBA_16_FLOAT:
        case Format::RGBA_32_FLOAT:
            baseFormat = GL_RGBA;
            break;
        case Format::DEPTH_32_FLOAT:
            baseFormat = GL_DEPTH_COMPONENT;
            break;
        case Format::DEPTH_24_STENCIL_8:
            baseFormat = GL_DEPTH_STENCIL;
            break;
        }

		return baseFormat;
	}

	GLenum OpenGLTexture::ConvertToOpenGLInternalFormat(Format format)
	{
		GLenum internalFormat = GL_NONE;

        switch (format)
        {
        case Format::R_8:
            internalFormat = GL_R8;
            break;
        case Format::R_8_NON_LINEAR:
            internalFormat = GL_R8;
            Logger::Warn("OpenGL", "Non linear format with only 1 component is not supported -> linear format GL_R8 is used instead");
            break;
        case Format::R_16_FLOAT:
            internalFormat = GL_R16F;
            break;
        case Format::R_32_FLOAT:
            internalFormat = GL_R32F;
            break;
        case Format::RG_8:
            internalFormat = GL_RG8;
            break;
        case Format::RG_8_NON_LINEAR:
            internalFormat = GL_RG8;
            Logger::Warn("OpenGL", "Non linear format with only 2 components is not supported -> linear format GL_RG8 is used instead");
            break;
        case Format::RG_16_FLOAT:
            internalFormat = GL_RG16F;
            break;
        case Format::RG_32_FLOAT:
            internalFormat = GL_RG32F;
            break;
        case Format::RGB_8:
            internalFormat = GL_RGB8;
            break;
        case Format::RGB_8_NON_LINEAR:
            internalFormat = GL_SRGB8;
            break;
        case Format::RGB_16_FLOAT:
            internalFormat = GL_RGB16F;
            break;
        case Format::RGB_32_FLOAT:
            internalFormat = GL_RGB32F;
            break;
        case Format::RGBA_8:
            internalFormat = GL_RGBA8;
            break;
        case Format::RGBA_8_NON_LINEAR:
            internalFormat = GL_SRGB8_ALPHA8;
            break;
        case Format::RGBA_16_FLOAT:
            internalFormat = GL_RGBA16F;
            break;
        case Format::RGBA_32_FLOAT:
            internalFormat = GL_RGBA32F;
            break;
        case Format::DEPTH_32_FLOAT:
            internalFormat = GL_DEPTH_COMPONENT32F;
            break;
        case Format::DEPTH_24_STENCIL_8:
            internalFormat = GL_DEPTH24_STENCIL8;
            break;
        }

		return internalFormat;
	}

    GLenum OpenGLTexture::GetOpenGLPixelDataType(Format format)
    {
        GLenum pixelDataType = GL_NONE;

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
            pixelDataType = GL_UNSIGNED_BYTE;
            break;
        case Format::R_16_FLOAT:
        case Format::RG_16_FLOAT:
        case Format::RGB_16_FLOAT:
        case Format::RGBA_16_FLOAT:
            pixelDataType = GL_HALF_FLOAT;
            break;
        case Format::R_32_FLOAT:
        case Format::RG_32_FLOAT:
        case Format::RGB_32_FLOAT:
        case Format::RGBA_32_FLOAT:
        case Format::DEPTH_32_FLOAT:
            pixelDataType = GL_FLOAT;
            break;
        case Format::DEPTH_24_STENCIL_8:
            pixelDataType = GL_UNSIGNED_INT_24_8;
            break;
        }

        return pixelDataType;
    }

    GLenum OpenGLTexture::ConvertToOpenGLTextureType(Type type, uint32_t sampleCount)
    {
        GLenum textureType = GL_NONE;

        switch (type)
        {
        case Type::TEXTURE_1D:
            textureType = GL_TEXTURE_1D;
            break;
        case Type::TEXTURE_2D:
            if(sampleCount == 1)
                textureType = GL_TEXTURE_2D;
            else
                textureType = GL_TEXTURE_2D_MULTISAMPLE;
            break;
        case Type::TEXTURE_3D:
            textureType = GL_TEXTURE_3D;
            break;
        case Type::TEXTURE_CUBE_MAP:
            textureType = GL_TEXTURE_CUBE_MAP;
            break;
        }

        if (type != Type::TEXTURE_2D && sampleCount > 1)
            Logger::Warn("OpenGL", "Sample count > 1 is ignored for non TEXTURE_2D types");

        return textureType;
    }

    GLenum OpenGLTexture::ConvertToOpenGLWrapMode(WrapMode wrapMode)
    {
        GLenum textureWrapMode = GL_NONE;

        switch (wrapMode)
        {
        case WrapMode::REPEAT:
            textureWrapMode = GL_REPEAT;
            break;
        case WrapMode::MIRRORED_REPEAT:
            textureWrapMode = GL_MIRRORED_REPEAT;
            break;
        case WrapMode::CLAMP_TO_EDGE:
            textureWrapMode = GL_CLAMP_TO_EDGE;
            break;
        case WrapMode::MIRROR_CLAMP_TO_EDGE:
            textureWrapMode = GL_MIRROR_CLAMP_TO_EDGE;
            break;
        case WrapMode::CLAMP_TO_BORDER:
            textureWrapMode = GL_CLAMP_TO_BORDER;
            break;
        }

        return textureWrapMode;
    }

    GLenum OpenGLTexture::ConvertToOpenGLMinificationFilterMode(FilterMode minFilterMode, FilterMode mipMapFilterMode)
    {
        GLenum minificationFilterMode = GL_NONE;

        switch (minFilterMode)
        {
        case FilterMode::NEAREST:
            switch (mipMapFilterMode)
            {
            case FilterMode::NEAREST:
                minificationFilterMode = GL_NEAREST_MIPMAP_NEAREST;
                break;
            case FilterMode::LINEAR:
                minificationFilterMode = GL_NEAREST_MIPMAP_LINEAR;
                break;
            }
            break;
        case FilterMode::LINEAR:
            switch (mipMapFilterMode)
            {
            case FilterMode::NEAREST:
                minificationFilterMode = GL_LINEAR_MIPMAP_NEAREST;
                break;
            case FilterMode::LINEAR:
                minificationFilterMode = GL_LINEAR_MIPMAP_LINEAR;
                break;
            }
            break;
        }

        return minificationFilterMode;
    }

    GLenum OpenGLTexture::ConvertToOpenGLFilterMode(FilterMode filterMode)
    {
        GLenum textureFilterMode = GL_NONE;

        switch (filterMode)
        {
        case FilterMode::NEAREST:
            textureFilterMode = GL_NEAREST;
            break;
        case FilterMode::LINEAR:
            textureFilterMode = GL_LINEAR;
            break;
        }

        return textureFilterMode;
    }

    GLsizei OpenGLTexture::ConvertToOpenGLSampleCount(SampleCount sampleCount)
    {
        GLsizei textureSampleCount = 0;

        switch (sampleCount)
        {
        case SampleCount::SAMPLE_1:
            textureSampleCount = 1;
            break;
        case SampleCount::SAMPLE_2:
            textureSampleCount = 2;
            break;
        case SampleCount::SAMPLE_4:
            textureSampleCount = 4;
            break;
        case SampleCount::SAMPLE_8:
            textureSampleCount = 8;
            break;
        case SampleCount::SAMPLE_16:
            textureSampleCount = 16;
            break;
        case SampleCount::SAMPLE_32:
            textureSampleCount = 32;
            break;
        case SampleCount::SAMPLE_64:
            textureSampleCount = 64;
            break;
        }

        return textureSampleCount;
    }

    uint32_t OpenGLTexture::GetBytePerPixel(Format format)
    {
        uint32_t bytePerPixel = 0;

        switch (format)
        {
        case Format::R_8:
        case Format::R_8_NON_LINEAR:
            bytePerPixel = 1;
            break;
        case Format::RG_8:
        case Format::RG_8_NON_LINEAR:
        case Format::R_16_FLOAT:
            bytePerPixel = 2;
            break;
        case Format::RGB_8:
        case Format::RGB_8_NON_LINEAR:
            bytePerPixel = 3;
            break;
        case Format::R_32_FLOAT:
        case Format::RG_16_FLOAT:
        case Format::RGBA_8:
        case Format::RGBA_8_NON_LINEAR:
        case Format::DEPTH_32_FLOAT:
        case Format::DEPTH_24_STENCIL_8:
            bytePerPixel = 4;
            break;
        case Format::RGB_16_FLOAT:
            bytePerPixel = 6;
            break;
        case Format::RG_32_FLOAT:
        case Format::RGBA_16_FLOAT:
            bytePerPixel = 8;
            break;
        case Format::RGB_32_FLOAT:
            bytePerPixel = 12;
            break;
        case Format::RGBA_32_FLOAT:
            bytePerPixel = 16;
            break;
        }

        return bytePerPixel;
    }
}