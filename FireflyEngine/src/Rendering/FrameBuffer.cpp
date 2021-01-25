#include "pch.h"
#include "Rendering/FrameBuffer.h"

namespace Firefly
{
	void FrameBuffer::Init(const FrameBuffer::Description& description)
	{
		m_description = description;

		FIREFLY_ASSERT(m_description.colorAttachments.size() > 0, "At least 1 color attachment is required!");
		FIREFLY_ASSERT(m_description.colorAttachments.size() <= 8, "Maximal 8 color attachments are supported!");
		FIREFLY_ASSERT((m_description.colorAttachments.size() == m_description.colorResolveAttachments.size()) || m_description.colorResolveAttachments.empty(), 
						"The amounts of color attachments and color resolve attachments need to match!");
		
		CheckColorAttachments();
		CheckColorResolveAttachments();
		CheckDepthStencilAttachment();

		CheckSampleCounts();

		OnInit();
	}

	uint32_t FrameBuffer::GetWidth() const
	{
		return m_description.width;
	}

	uint32_t FrameBuffer::GetHeight() const
	{
		return m_description.height;
	}

	std::vector<FrameBuffer::Attachment> FrameBuffer::GetColorAttachments()
	{
		return m_description.colorAttachments;
	}

	std::vector<FrameBuffer::Attachment> FrameBuffer::GetColorResolveAttachments()
	{
		return m_description.colorResolveAttachments;
	}

	FrameBuffer::Attachment FrameBuffer::GetDepthStencilAttachment()
	{
		return m_description.depthStencilAttachment;
	}

	bool FrameBuffer::HasDepthStencilAttachment() const
	{
		if(m_description.depthStencilAttachment.texture)
			return true;
		return false;
	}

	void FrameBuffer::CheckColorAttachments()
	{
		for (size_t i = 0; i < m_description.colorAttachments.size(); i++)
		{
			CheckAttachment(m_description.colorAttachments[i], "color attachment " + std::to_string(i));
			std::shared_ptr<Texture> colorTexture = m_description.colorAttachments[i].texture;
			FIREFLY_ASSERT(colorTexture->HasColorFormat(), "Color attachment {0} has no color texture format: {1}", i, colorTexture->GetFormatAsString());
		}
	}

	void FrameBuffer::CheckColorResolveAttachments()
	{
		for (size_t i = 0; i < m_description.colorResolveAttachments.size(); i++)
		{
			CheckAttachment(m_description.colorResolveAttachments[i], "color resolve attachment " + std::to_string(i));
			std::shared_ptr<Texture> colorTexture = m_description.colorAttachments[i].texture;
			std::shared_ptr<Texture> colorResolveTexture = m_description.colorResolveAttachments[i].texture;
			FIREFLY_ASSERT(colorTexture->GetFormat() == colorResolveTexture->GetFormat(),
							"Color resolve attachment {0} has not the same format as color attachment {0}. {1} != {2}", i, colorResolveTexture->GetFormatAsString(), colorTexture->GetFormatAsString());
		}
	}

	void FrameBuffer::CheckDepthStencilAttachment()
	{
		if (HasDepthStencilAttachment())
		{
			CheckAttachment(m_description.depthStencilAttachment, "depth stencil attachment");
			std::shared_ptr<Texture> depthStencilTexture = m_description.depthStencilAttachment.texture;
			FIREFLY_ASSERT(depthStencilTexture->HasDepthFormat() || depthStencilTexture->HasDepthStencilFormat(),
							"Depth stencil attachment has no depth/stencil texture format: {0}", depthStencilTexture->GetFormatAsString());
		}
	}

	void FrameBuffer::CheckAttachment(Attachment& attachment, const std::string& attachmentLabel)
	{
		std::shared_ptr<Texture> texture = attachment.texture;
		FIREFLY_ASSERT(texture, attachmentLabel + " has a nullptr texture!");

		if (texture->GetMipMapLevels() <= attachment.mipMapLevel)
		{
			Logger::Warn("FrameBuffer", "Specified mip map level ({0}) for " + attachmentLabel + " is not compatible with the texture.",
						attachment.mipMapLevel);
						attachment.mipMapLevel = texture->GetMipMapLevels() - 1;
		}

		if ((texture->GetType() != Texture::Type::TEXTURE_CUBE_MAP) && (attachment.arrayLayer > 0))
		{
			Logger::Warn("FrameBuffer", "Specified array layer ({0}) for " + attachmentLabel + " is not compatible with the texture.",
						attachment.arrayLayer);
			attachment.arrayLayer = 0;
		}
		else if ((texture->GetType() == Texture::Type::TEXTURE_CUBE_MAP) && (attachment.arrayLayer >= 6))
		{
			Logger::Warn("FrameBuffer", "Specified array layer ({0}) for " + attachmentLabel + " is not compatible with the cube map texture.",
						attachment.arrayLayer);
			attachment.arrayLayer = 5;
		}

		uint32_t mipMapTextureWidth = texture->GetWidth() * std::pow(0.5, attachment.mipMapLevel);
		uint32_t mipMapTextureHeight = texture->GetHeight() * std::pow(0.5, attachment.mipMapLevel);
		FIREFLY_ASSERT((m_description.width == mipMapTextureWidth) && (m_description.height == mipMapTextureHeight),
						attachmentLabel + " texture size ({0}, {1}) mismatches the frame buffer size ({2}, {3})",
						mipMapTextureWidth, mipMapTextureHeight, m_description.width, m_description.height);
	}

	void FrameBuffer::CheckSampleCounts()
	{
		Texture::SampleCount sampleCount = m_description.colorAttachments[0].texture->GetSampleCount();
		for (size_t i = 1; i < m_description.colorAttachments.size(); i++)
			FIREFLY_ASSERT(sampleCount == m_description.colorAttachments[i].texture->GetSampleCount(), "Color attachment {0} has not the same sample count as the other attachments!", i);
		
		if (HasDepthStencilAttachment())
			FIREFLY_ASSERT(sampleCount == m_description.depthStencilAttachment.texture->GetSampleCount(), "Depth stencil attachment has not the same sample count as the color attachments!");

		for (size_t i = 0; i < m_description.colorResolveAttachments.size(); i++)
			FIREFLY_ASSERT(m_description.colorResolveAttachments[i].texture->GetSampleCount() == Texture::SampleCount::SAMPLE_1, "Color resolve attachment has multiple samples. Only 1 sample is allowed");
	}
}