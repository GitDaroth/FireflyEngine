#include "pch.h"
#include "Rendering/Vulkan/VulkanFrameBuffer.h"

#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanTexture.h"

namespace Firefly
{
	void VulkanFrameBuffer::Destroy()
	{
		for (vk::ImageView imageView : m_imageViewAttachments)
			m_device.destroyImageView(imageView);
		m_imageViewAttachments.clear();

		m_device.destroyRenderPass(m_compatibilityRenderPass);
		m_device.destroyFramebuffer(m_frameBuffer);
	}

	void VulkanFrameBuffer::Resolve()
	{
		// Is handled in the subpass of the render pass
	}

	vk::Framebuffer VulkanFrameBuffer::GetHandle()
	{
		return m_frameBuffer;
	}

	void VulkanFrameBuffer::OnInit()
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		m_device = vkContext->GetDevice()->GetDevice();

		CreateCompatibilityRenderPass();

		for (size_t i = 0; i < m_description.colorAttachments.size(); i++)
			m_imageViewAttachments.push_back(CreateImageViewFromAttachment(m_description.colorAttachments[i]));

		if(HasDepthStencilAttachment())
			m_imageViewAttachments.push_back(CreateImageViewFromAttachment(m_description.depthStencilAttachment));

		for (size_t i = 0; i < m_description.colorResolveAttachments.size(); i++)
			m_imageViewAttachments.push_back(CreateImageViewFromAttachment(m_description.colorResolveAttachments[i]));

		vk::FramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.renderPass = m_compatibilityRenderPass;
		framebufferCreateInfo.attachmentCount = m_imageViewAttachments.size();
		framebufferCreateInfo.pAttachments = m_imageViewAttachments.data();
		framebufferCreateInfo.width = m_description.width;
		framebufferCreateInfo.height = m_description.height;
		framebufferCreateInfo.layers = 1;

		vk::Result result = m_device.createFramebuffer(&framebufferCreateInfo, nullptr, &m_frameBuffer);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan framebuffer!");
	}

	void VulkanFrameBuffer::CreateCompatibilityRenderPass()
	{
		std::vector<vk::AttachmentDescription> attachmentDescriptions;
		std::vector<vk::AttachmentReference> colorAttachmentReferences;
		std::vector<vk::AttachmentReference> colorResolveAttachmentReferences;
		vk::AttachmentReference depthStencilAttachmentReference;

		size_t attachmentIndex = 0;
		for (size_t i = 0; i < m_description.colorAttachments.size(); i++)
		{
			attachmentDescriptions.push_back(CreateAttachmentDescription(m_description.colorAttachments[i]));
			colorAttachmentReferences.push_back(CreateAttachmentReference(m_description.colorAttachments[i], attachmentIndex));
			attachmentIndex++;
		}

		if (HasDepthStencilAttachment())
		{
			attachmentDescriptions.push_back(CreateAttachmentDescription(m_description.depthStencilAttachment));
			depthStencilAttachmentReference = CreateAttachmentReference(m_description.depthStencilAttachment, attachmentIndex);
			attachmentIndex++;
		}

		for (size_t i = 0; i < m_description.colorResolveAttachments.size(); i++)
		{
			attachmentDescriptions.push_back(CreateAttachmentDescription(m_description.colorResolveAttachments[i]));
			colorResolveAttachmentReferences.push_back(CreateAttachmentReference(m_description.colorResolveAttachments[i], attachmentIndex));
			attachmentIndex++;
		}

		vk::SubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDescription.flags = {};
		subpassDescription.colorAttachmentCount = colorAttachmentReferences.size();
		subpassDescription.pColorAttachments = colorAttachmentReferences.data();
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		if (HasDepthStencilAttachment())
			subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
		else
			subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = colorResolveAttachmentReferences.data();

		vk::RenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = {};
		renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
		renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 0;
		renderPassCreateInfo.pDependencies = nullptr;

		vk::Result result = m_device.createRenderPass(&renderPassCreateInfo, nullptr, &m_compatibilityRenderPass);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan render pass!");
	}

	vk::ImageView VulkanFrameBuffer::CreateImageViewFromAttachment(FrameBuffer::Attachment attachment)
	{
		std::shared_ptr<VulkanTexture> texture = std::dynamic_pointer_cast<VulkanTexture>(attachment.texture);
		vk::Format format = VulkanTexture::ConvertToVulkanFormat(texture->GetFormat());

		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.flags = {};
		imageViewCreateInfo.image = texture->GetImage();
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.subresourceRange.aspectMask = VulkanTexture::GetImageAspectFlags(format);
		imageViewCreateInfo.subresourceRange.baseMipLevel = attachment.mipMapLevel;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = attachment.arrayLayer;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		vk::ImageView imageView;
		vk::Result result = m_device.createImageView(&imageViewCreateInfo, nullptr, &imageView);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan image view!");

		return imageView;
	}

	vk::AttachmentDescription VulkanFrameBuffer::CreateAttachmentDescription(FrameBuffer::Attachment attachment)
	{
		vk::Format format = VulkanTexture::ConvertToVulkanFormat(attachment.texture->GetFormat());

		vk::AttachmentDescription attachmentDescription{};
		attachmentDescription.flags = {};
		attachmentDescription.format = format;
		attachmentDescription.samples = VulkanTexture::ConvertToVulkanSampleCount(attachment.texture->GetSampleCount());
		attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
		attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
		attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
		attachmentDescription.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		return attachmentDescription;
	}

	vk::AttachmentReference VulkanFrameBuffer::CreateAttachmentReference(FrameBuffer::Attachment attachment, size_t index)
	{
		std::shared_ptr<VulkanTexture> texture = std::dynamic_pointer_cast<VulkanTexture>(attachment.texture);

		vk::ImageLayout layout;
		if (texture->HasColorFormat())
			layout = vk::ImageLayout::eColorAttachmentOptimal;
		else if (texture->HasDepthFormat() || texture->HasDepthStencilFormat())
			layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference attachmentReference{};
		attachmentReference.attachment = index;
		attachmentReference.layout = layout;

		return attachmentReference;
	}
}