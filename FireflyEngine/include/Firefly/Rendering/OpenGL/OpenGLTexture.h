#pragma once

#include "Rendering/Texture.h"

#include <glad/glad.h>

namespace Firefly
{
	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture();

		virtual void Destroy() override;

		void Bind(GLuint slot);

	protected:
		virtual void OnInit(void* pixelData) override;

	private:
		static GLenum ConvertToOpenGLBaseFormat(Format format);
		static GLenum ConvertToOpenGLInternalFormat(Format format);
		static GLenum GetOpenGLPixelDataType(Format format);
		static GLenum ConvertToOpenGLTextureType(Type type, uint32_t sampleCount);
		static GLenum ConvertToOpenGLWrapMode(WrapMode wrapMode);
		static GLenum ConvertToOpenGLMinificationFilterMode(FilterMode minfilterMode, FilterMode mipMapFilterMode);
		static GLenum ConvertToOpenGLFilterMode(FilterMode filterMode);
		static GLsizei ConvertToOpenGLSampleCount(SampleCount sampleCount);
		static uint32_t GetBytePerPixel(Format format);

		uint32_t m_texture;
	};
}