#include "pch.h"
#include "Rendering/OpenGL/OpenGLFrameBuffer.h"

#include "Rendering/OpenGL/OpenGLTexture.h"
#include <glad/glad.h>

namespace Firefly
{
    void OpenGLFrameBuffer::Destroy()
    {
        glDeleteTextures(m_textureViewAttachments.size(), m_textureViewAttachments.data());
        glDeleteFramebuffers(1, &m_resolveFrameBuffer);
        glDeleteFramebuffers(1, &m_frameBuffer);
    }

    void OpenGLFrameBuffer::Resolve()
    {
        if (m_description.colorResolveAttachments.size() == 0)
            return;

        for (size_t i = 0; i < m_description.colorAttachments.size(); i++)
        {
            glNamedFramebufferReadBuffer(m_frameBuffer, GL_COLOR_ATTACHMENT0 + i);
            glNamedFramebufferDrawBuffer(m_resolveFrameBuffer, GL_COLOR_ATTACHMENT0 + i);

            glBlitNamedFramebuffer(m_frameBuffer, m_resolveFrameBuffer,
                0, 0, m_description.width, m_description.height,
                0, 0, m_description.width, m_description.height,
                GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }

    void OpenGLFrameBuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    }

    void OpenGLFrameBuffer::OnInit()
    {
        glCreateFramebuffers(1, &m_frameBuffer);

        for (size_t i = 0; i < m_description.colorAttachments.size(); i++)
        {
            uint32_t textureView = CreateTextureViewFromAttachment(m_description.colorAttachments[i]);
            glNamedFramebufferTexture(m_frameBuffer, GL_COLOR_ATTACHMENT0 + i, textureView, 0);
            m_textureViewAttachments.push_back(textureView);
        }

        if (HasDepthStencilAttachment())
        {
            std::shared_ptr<OpenGLTexture> depthStencilTexture = std::dynamic_pointer_cast<OpenGLTexture>(m_description.depthStencilAttachment.texture);
            if (depthStencilTexture->GetType() == Texture::Type::TEXTURE_CUBE_MAP)
                return;

            GLenum attachment = GL_NONE;
            if (depthStencilTexture->HasDepthFormat())
                attachment = GL_DEPTH_ATTACHMENT;
            else if (depthStencilTexture->HasDepthStencilFormat())
                attachment = GL_DEPTH_STENCIL_ATTACHMENT;

            uint32_t textureView = CreateTextureViewFromAttachment(m_description.depthStencilAttachment);
            glNamedFramebufferTexture(m_frameBuffer, attachment, textureView, 0);
            m_textureViewAttachments.push_back(textureView);
        }

        FIREFLY_ASSERT(glCheckNamedFramebufferStatus(m_frameBuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "OpenGL framebuffer for color attachments and depth stencil attachment is not complete!");

        if (m_description.colorResolveAttachments.size() > 0)
        {
            glCreateFramebuffers(1, &m_resolveFrameBuffer);

            for (size_t i = 0; i < m_description.colorResolveAttachments.size(); i++)
            {
                uint32_t textureView = CreateTextureViewFromAttachment(m_description.colorResolveAttachments[i]);
                glNamedFramebufferTexture(m_resolveFrameBuffer, GL_COLOR_ATTACHMENT0 + i, textureView, 0);
                m_textureViewAttachments.push_back(textureView);
            }

            FIREFLY_ASSERT(glCheckNamedFramebufferStatus(m_resolveFrameBuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "OpenGL Framebuffer for resolve color attachments is not complete!");
        }
    }

    uint32_t OpenGLFrameBuffer::CreateTextureViewFromAttachment(Attachment attachment)
    {
        std::shared_ptr<OpenGLTexture> texture = std::dynamic_pointer_cast<OpenGLTexture>(attachment.texture);
        GLenum internalFormat = texture->ConvertToOpenGLInternalFormat(texture->GetFormat());
        GLenum textureType = GL_TEXTURE_2D;
        if (texture->GetSampleCount() != Texture::SampleCount::SAMPLE_1)
            textureType = GL_TEXTURE_2D_MULTISAMPLE;

        uint32_t textureView;
        glGenTextures(1, &textureView);
        glTextureView(textureView, textureType, texture->GetHandle(), internalFormat, attachment.mipMapLevel, 1, attachment.arrayLayer, 1);

        return textureView;
    }
}