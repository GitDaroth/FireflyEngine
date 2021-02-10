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
		uint32_t GetHandle() const;

	protected:
		virtual void OnInit(void* pixelData) override;

	private:
		void CreateTexture(void* pixelData);
		void DestroyTexture();
		void CreateSampler();
		void DestroySampler();

		static GLenum ConvertToOpenGLBaseFormat(Format format);
		static GLenum ConvertToOpenGLInternalFormat(Format format);
		static GLenum GetOpenGLPixelDataType(Format format);
		static GLenum ConvertToOpenGLTextureType(Type type, uint32_t sampleCount);
		static GLenum ConvertToOpenGLWrapMode(WrapMode wrapMode);
		static GLenum ConvertToOpenGLMinificationFilterMode(FilterMode minfilterMode, FilterMode mipMapFilterMode);
		static GLenum ConvertToOpenGLFilterMode(FilterMode filterMode);

		uint32_t m_texture;
		uint32_t m_sampler;
	};
}