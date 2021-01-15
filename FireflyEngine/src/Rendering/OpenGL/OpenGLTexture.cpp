#include "pch.h"
#include "Rendering/OpenGL/OpenGLTexture.h"

namespace Firefly
{
	OpenGLTexture::OpenGLTexture(std::shared_ptr<GraphicsContext> context) :
		Texture(context),
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

	void OpenGLTexture::OnInit(unsigned char* pixelData, ColorSpace colorSpace)
	{
		GLenum baseFormat = 0;
		GLenum internalFormat = 0;
		switch (colorSpace)
		{
		case ColorSpace::SRGB:
			baseFormat = GL_RGBA;
			internalFormat = GL_SRGB8_ALPHA8;
			break;
		case ColorSpace::RGB:
		default:
			baseFormat = GL_RGBA;
			internalFormat = GL_RGBA8;
			break;
		}

		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, baseFormat, GL_UNSIGNED_BYTE, pixelData);

		GLfloat maxAnisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}