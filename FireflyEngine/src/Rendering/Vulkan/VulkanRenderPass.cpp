#include "pch.h"
#include "Rendering/Vulkan/VulkanRenderPass.h"

#include "Rendering/RenderingAPI.h"
#include "Rendering/Vulkan/VulkanContext.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanFrameBuffer.h"

namespace Firefly
{
	void VulkanRenderPass::OnInit()
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		m_device = vkContext->GetDevice()->GetHandle();

		std::vector<vk::AttachmentDescription> attachmentDescriptions;
		std::vector<vk::AttachmentReference> colorAttachmentReferences;
		std::vector<vk::AttachmentReference> colorResolveAttachmentReferences;
		vk::AttachmentReference depthStencilAttachmentReference;

		size_t attachmentIndex = 0;
		for (size_t i = 0; i < m_description.colorAttachmentLayouts.size(); i++)
		{
			attachmentDescriptions.push_back(CreateAttachmentDescription(m_description.colorAttachmentLayouts[i]));
			colorAttachmentReferences.push_back(CreateAttachmentReference(m_description.colorAttachmentLayouts[i], attachmentIndex));
			attachmentIndex++;
		}

		if (HasDepthStencilAttachmentLayout())
		{
			attachmentDescriptions.push_back(CreateAttachmentDescription(m_description.depthStencilAttachmentLayout));
			depthStencilAttachmentReference = CreateAttachmentReference(m_description.depthStencilAttachmentLayout, attachmentIndex);
			attachmentIndex++;
		}

		for (size_t i = 0; i < m_description.colorResolveAttachmentLayouts.size(); i++)
		{
			attachmentDescriptions.push_back(CreateAttachmentDescription(m_description.colorResolveAttachmentLayouts[i]));
			colorResolveAttachmentReferences.push_back(CreateAttachmentReference(m_description.colorResolveAttachmentLayouts[i], attachmentIndex));
			attachmentIndex++;
		}

		vk::SubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpassDescription.flags = {};
		subpassDescription.colorAttachmentCount = colorAttachmentReferences.size();
		subpassDescription.pColorAttachments = colorAttachmentReferences.data();
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		if (HasDepthStencilAttachmentLayout())
			subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
		else
			subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;
		subpassDescription.pResolveAttachments = colorResolveAttachmentReferences.data();

		vk::SubpassDependency subpassDependency{};
		subpassDependency.dependencyFlags = {};
		subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass = 0;
		subpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		subpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		subpassDependency.srcAccessMask = {};
		if(HasDepthStencilAttachmentLayout())
			subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		else
			subpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		vk::RenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = {};
		renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
		renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1;
		renderPassCreateInfo.pDependencies = &subpassDependency;

		vk::Result result = m_device.createRenderPass(&renderPassCreateInfo, nullptr, &m_renderPass);
		FIREFLY_ASSERT(result == vk::Result::eSuccess, "Unable to create Vulkan render pass!");


		for (size_t i = 0; i < m_description.colorAttachmentLayouts.size(); i++)
		{
			vk::ClearValue clearValue;
			clearValue.color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };
			m_clearValues.push_back(clearValue);
		}
		if (HasDepthStencilAttachmentLayout())
		{
			vk::ClearValue clearValue;
			clearValue.depthStencil = { 1.0f, 0 };
			m_clearValues.push_back(clearValue);
		}
		for (size_t i = 0; i < m_description.colorResolveAttachmentLayouts.size(); i++)
		{
			vk::ClearValue clearValue;
			clearValue.color = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f };
			m_clearValues.push_back(clearValue);
		}
	}

	void VulkanRenderPass::Destroy()
	{
		m_device.destroyRenderPass(m_renderPass);
	}

	vk::RenderPass VulkanRenderPass::GetHandle()
	{
		return m_renderPass;
	}

	vk::CompareOp VulkanRenderPass::ConvertToVulkanCompareOperation(CompareOperation compareOperation)
	{
		switch (compareOperation)
		{
		case CompareOperation::LESS:
			return vk::CompareOp::eLess;
		case CompareOperation::LESS_OR_EQUAL:
			return vk::CompareOp::eLessOrEqual;
		case CompareOperation::NOT_EQUAL:
			return vk::CompareOp::eNotEqual;
		case CompareOperation::EQUAL:
			return vk::CompareOp::eEqual;
		case CompareOperation::GREATER:
			return vk::CompareOp::eGreater;
		case CompareOperation::GREATER_OR_EQUAL:
			return vk::CompareOp::eGreaterOrEqual;
		case CompareOperation::ALWAYS:
			return vk::CompareOp::eAlways;
		case CompareOperation::NEVER:
			return vk::CompareOp::eNever;
		}
	}

	void VulkanRenderPass::OnBegin()
	{
		vk::Extent2D extent;
		extent.width = m_currentFrameBuffer->GetWidth();
		extent.height = m_currentFrameBuffer->GetHeight();

		vk::RenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = std::dynamic_pointer_cast<VulkanFrameBuffer>(m_currentFrameBuffer)->GetHandle();
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = extent;
		renderPassBeginInfo.clearValueCount = m_clearValues.size();
		renderPassBeginInfo.pClearValues = m_clearValues.data();

		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		vkContext->GetCurrentCommandBuffer().beginRenderPass(&renderPassBeginInfo, vk::SubpassContents::eInline);

		vk::Viewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkContext->GetCurrentCommandBuffer().setViewport(0, 1, &viewport);

		vk::Rect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkContext->GetCurrentCommandBuffer().setScissor(0, 1, &scissor);
	}

	void VulkanRenderPass::OnEnd()
	{
		std::shared_ptr<VulkanContext> vkContext = std::dynamic_pointer_cast<VulkanContext>(RenderingAPI::GetContext());
		vkContext->GetCurrentCommandBuffer().endRenderPass();
	}

	vk::AttachmentDescription VulkanRenderPass::CreateAttachmentDescription(RenderPass::AttachmentLayout attachmentLayout)
	{
		vk::Format format = VulkanTexture::ConvertToVulkanFormat(attachmentLayout.format);

		vk::AttachmentDescription attachmentDescription{};
		attachmentDescription.flags = {};
		attachmentDescription.format = format;
		attachmentDescription.samples = VulkanTexture::ConvertToVulkanSampleCount(attachmentLayout.sampleCount);
		attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
		attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
		attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
		attachmentDescription.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		return attachmentDescription;
	}

	vk::AttachmentReference VulkanRenderPass::CreateAttachmentReference(RenderPass::AttachmentLayout attachmentLayout, size_t index)
	{
		vk::ImageLayout layout;
		switch (attachmentLayout.format)
		{
		case Texture::Format::DEPTH_32_FLOAT:
		case Texture::Format::DEPTH_24_STENCIL_8:
			layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			break;
		default:
			layout = vk::ImageLayout::eColorAttachmentOptimal;
			break;
		}

		vk::AttachmentReference attachmentReference{};
		attachmentReference.attachment = index;
		attachmentReference.layout = layout;

		return attachmentReference;
	}
}