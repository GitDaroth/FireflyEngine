#pragma once

#include "Rendering/RenderPass.h"
#include <vulkan/vulkan.hpp>

namespace Firefly
{
	class VulkanRenderPass : public RenderPass
	{
	public:
		virtual void Destroy() override;

		vk::RenderPass GetHandle();

		static vk::CompareOp ConvertToVulkanCompareOperation(CompareOperation compareOperation);

	protected:
		virtual void OnInit() override;
		virtual void OnBegin() override;
		virtual void OnEnd() override;

	private:
		vk::AttachmentDescription CreateAttachmentDescription(RenderPass::AttachmentLayout attachmentLayout);
		vk::AttachmentReference CreateAttachmentReference(RenderPass::AttachmentLayout attachmentLayout, size_t index);

		vk::Device m_device;
		vk::RenderPass m_renderPass;
		std::vector<vk::ClearValue> m_clearValues;
	};
}