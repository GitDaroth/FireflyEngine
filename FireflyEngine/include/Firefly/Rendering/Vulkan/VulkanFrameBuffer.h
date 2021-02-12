#pragma once

#include "Rendering/FrameBuffer.h"
#include <vulkan/vulkan.hpp>

namespace Firefly
{
    class VulkanFrameBuffer : public FrameBuffer
    {
    public:
        virtual void Destroy() override;
        virtual void Resolve() override;

        vk::Framebuffer GetHandle();

    protected:
        virtual void OnInit() override;

    private:
        void CreateCompatibilityRenderPass();
        vk::ImageView CreateImageViewFromAttachment(FrameBuffer::Attachment attachment);
        vk::AttachmentDescription CreateAttachmentDescription(FrameBuffer::Attachment attachment);
        vk::AttachmentReference CreateAttachmentReference(FrameBuffer::Attachment attachment, size_t index);

        vk::Device m_device;

        std::vector<vk::ImageView> m_imageViewAttachments;
        vk::RenderPass m_compatibilityRenderPass;
        vk::Framebuffer m_frameBuffer;
    };
}