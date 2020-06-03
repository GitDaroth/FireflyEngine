#include "pch.h"
#include "Rendering/OpenGL/OpenGLTexture.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Firefly
{
	OpenGLTexture2D::OpenGLTexture2D()
	{
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_texture);
	}

	void OpenGLTexture2D::Init(const std::string& path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (stbi_failure_reason())
			Logger::Error("FireflyEngine", "Failed to load image({0}): {1}", path, stbi_failure_reason());

		m_width = width;
		m_height = height;

		GLenum internalFormat = 0;
		GLenum dataFormat = 0;
		if (channels = 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (channels = 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else
		{
			Logger::Error("FireflyEngine", "Wrong image format with {0} channels!", channels);
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
		glTextureStorage2D(m_texture, 1, internalFormat, m_width, m_height);
		glTexParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureSubImage2D(m_texture, 0, 0, 0, m_width, m_height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot)
	{
		glBindTextureUnit(slot, m_texture);
	}
}