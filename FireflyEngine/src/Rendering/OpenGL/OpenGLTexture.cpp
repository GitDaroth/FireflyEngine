//#include "pch.h"
//#include "Rendering/OpenGL/OpenGLTexture.h"
//
//#include <glad/glad.h>
//#include <stb_image.h>
//
//namespace Firefly
//{
//	OpenGLTexture2D::OpenGLTexture2D()
//	{
//	}
//
//	OpenGLTexture2D::~OpenGLTexture2D()
//	{
//		glDeleteTextures(1, &m_texture);
//	}
//
//	void OpenGLTexture2D::Init(const std::string& path)
//	{
//		int width, height, channels;
//		stbi_set_flip_vertically_on_load(1);
//		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
//		if (!data)
//			Logger::Error("FireflyEngine", "Failed to load image({0}): {1}", path);
//
//		m_width = width;
//		m_height = height;
//
//		GLenum dataFormat = 0;
//		switch (channels)
//		{
//		case 1:
//			dataFormat = GL_RED;
//			break;
//		case 3:
//			dataFormat = GL_RGB;
//			break;
//		case 4:
//			dataFormat = GL_RGBA;
//			break;
//		default:
//			Logger::Error("FireflyEngine", "Wrong image format with {0} channels!", channels);
//			break;
//		}
//
//		glGenTextures(1, &m_texture);
//		glBindTexture(GL_TEXTURE_2D, m_texture);
//		glTexImage2D(GL_TEXTURE_2D, 0, dataFormat, m_width, m_height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
//
//		GLfloat maxAnisotropy;
//		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
//		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);
//
//		glGenerateMipmap(GL_TEXTURE_2D);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//		stbi_image_free(data);
//	}
//
//	void OpenGLTexture2D::Bind(uint32_t slot)
//	{
//		glBindTextureUnit(slot, m_texture);
//	}
//}