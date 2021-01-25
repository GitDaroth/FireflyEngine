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

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resolveFrameBuffer);
		for (size_t i = 0; i < m_description.colorAttachments.size(); i++)
		{
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
			glBlitFramebuffer(0, 0, m_description.width, m_description.height, 0, 0, m_description.width, m_description.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}

	void OpenGLFrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	}

	void OpenGLFrameBuffer::OnInit()
	{
		glGenFramebuffers(1, &m_frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

		for (size_t i = 0; i < m_description.colorAttachments.size(); i++)
		{
			Attachment colorAttachment = m_description.colorAttachments[i];
			std::shared_ptr<OpenGLTexture> texture = std::dynamic_pointer_cast<OpenGLTexture>(colorAttachment.texture);

			GLenum texTarget = GL_NONE;
			if (texture->GetType() == Texture::Type::TEXTURE_2D)
			{
				if (texture->GetSampleCount() == Texture::SampleCount::SAMPLE_1)
					texTarget = GL_TEXTURE_2D;
				else
					texTarget = GL_TEXTURE_2D_MULTISAMPLE;
			}
			else if (texture->GetType() == Texture::Type::TEXTURE_CUBE_MAP)
			{
				texTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + colorAttachment.arrayLayer;
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, texTarget, texture->GetHandle(), colorAttachment.mipMapLevel);
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

			GLenum texTarget = GL_NONE;
			if (depthStencilTexture->GetSampleCount() == Texture::SampleCount::SAMPLE_1)
				texTarget = GL_TEXTURE_2D;
			else
				texTarget = GL_TEXTURE_2D_MULTISAMPLE;

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texTarget, depthStencilTexture->GetHandle(), m_description.depthStencilAttachment.mipMapLevel);
		}

		FIREFLY_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "OpenGL framebuffer for color attachments and depth stencil attachment is not complete!");

		if (m_description.colorResolveAttachments.size() > 0)
		{
			glGenFramebuffers(1, &m_resolveFrameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, m_resolveFrameBuffer);

			for (size_t i = 0; i < m_description.colorResolveAttachments.size(); i++)
			{
				Attachment colorResolveAttachment = m_description.colorResolveAttachments[i];
				std::shared_ptr<OpenGLTexture> texture = std::dynamic_pointer_cast<OpenGLTexture>(colorResolveAttachment.texture);

				GLenum texTarget = GL_NONE;
				if (texture->GetType() == Texture::Type::TEXTURE_2D)
					texTarget = GL_TEXTURE_2D;
				else if (texture->GetType() == Texture::Type::TEXTURE_CUBE_MAP)
					texTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + colorResolveAttachment.arrayLayer;

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, texTarget, texture->GetHandle(), colorResolveAttachment.mipMapLevel);
			}

			FIREFLY_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "OpenGL Framebuffer for resolve color attachments is not complete!");
		}
	}
}