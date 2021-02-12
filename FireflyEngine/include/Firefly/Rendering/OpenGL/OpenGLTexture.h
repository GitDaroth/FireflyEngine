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

        static GLenum ConvertToOpenGLBaseFormat(Format format);
        static GLenum ConvertToOpenGLInternalFormat(Format format);
        static GLenum GetOpenGLPixelDataType(Format format);
        static GLenum ConvertToOpenGLTextureType(Type type, uint32_t sampleCount);
        static GLenum ConvertToOpenGLWrapMode(WrapMode wrapMode);
        static GLenum ConvertToOpenGLMinificationFilterMode(FilterMode minfilterMode, FilterMode mipMapFilterMode);
        static GLenum ConvertToOpenGLFilterMode(FilterMode filterMode);
        static size_t GetTextureByteSize(uint32_t width, uint32_t height, Type type, Format format);

    protected:
        virtual void OnInit(void* pixelData) override;

    private:
        void CreateTexture(void* pixelData);
        void DestroyTexture();
        void CreateTextureView();
        void DestroyTextureView();
        void CreateSampler();
        void DestroySampler();

        uint32_t m_texture;
        uint32_t m_textureView;
        uint32_t m_sampler;

        GLenum m_baseFormat;
        GLenum m_internalFormat;
        GLenum m_pixelDataType;
        GLsizei m_sampleCount;
        GLenum m_textureType;
    };
}