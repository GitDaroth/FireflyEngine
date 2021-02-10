#include "pch.h"
#include "Rendering/OpenGL/OpenGLFrameBuffer.h"

#include "Rendering/OpenGL/OpenGLTexture.h"
#include <glad/glad.h>

namespace Firefly
{
	void OpenGLFrameBuffer::Destroy()
	{
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
			Attachment colorAttachment = m_description.colorAttachments[i];
			std::shared_ptr<OpenGLTexture> texture = std::dynamic_pointer_cast<OpenGLTexture>(colorAttachment.texture);

			if (texture->GetType() == Texture::Type::TEXTURE_2D)
				glNamedFramebufferTexture(m_frameBuffer, GL_COLOR_ATTACHMENT0 + i, texture->GetHandle(), colorAttachment.mipMapLevel);
			else if (texture->GetType() == Texture::Type::TEXTURE_CUBE_MAP)
				glNamedFramebufferTextureLayer(m_frameBuffer, GL_COLOR_ATTACHMENT0 + i, texture->GetHandle(), colorAttachment.mipMapLevel, colorAttachment.arrayLayer);
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

			glNamedFramebufferTexture(m_frameBuffer, attachment, depthStencilTexture->GetHandle(), m_description.depthStencilAttachment.mipMapLevel);
		}

		FIREFLY_ASSERT(glCheckNamedFramebufferStatus(m_frameBuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "OpenGL framebuffer for color attachments and depth stencil attachment is not complete!");

		if (m_description.colorResolveAttachments.size() > 0)
		{
			glCreateFramebuffers(1, &m_resolveFrameBuffer);

			for (size_t i = 0; i < m_description.colorResolveAttachments.size(); i++)
			{
				Attachment colorResolveAttachment = m_description.colorResolveAttachments[i];
				std::shared_ptr<OpenGLTexture> texture = std::dynamic_pointer_cast<OpenGLTexture>(colorResolveAttachment.texture);

				if (texture->GetType() == Texture::Type::TEXTURE_2D)
					glNamedFramebufferTexture(m_resolveFrameBuffer, GL_COLOR_ATTACHMENT0 + i, texture->GetHandle(), colorResolveAttachment.mipMapLevel);
				else if (texture->GetType() == Texture::Type::TEXTURE_CUBE_MAP)
					glNamedFramebufferTextureLayer(m_resolveFrameBuffer, GL_COLOR_ATTACHMENT0 + i, texture->GetHandle(), colorResolveAttachment.mipMapLevel, colorResolveAttachment.arrayLayer);
			}

			FIREFLY_ASSERT(glCheckNamedFramebufferStatus(m_resolveFrameBuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "OpenGL Framebuffer for resolve color attachments is not complete!");
		}
	}
}