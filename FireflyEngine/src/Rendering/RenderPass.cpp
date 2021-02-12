#include "pch.h"
#include "Rendering/RenderPass.h"

namespace Firefly
{
    void RenderPass::Init(const RenderPass::Description& description)
    {
        m_description = description;

        FIREFLY_ASSERT(m_description.colorAttachmentLayouts.size() > 0, "At least 1 color attachment layout is required!");
        FIREFLY_ASSERT(m_description.colorAttachmentLayouts.size() <= 8, "Maximal 8 color attachment layouts are supported!");
        FIREFLY_ASSERT((m_description.colorAttachmentLayouts.size() == m_description.colorResolveAttachmentLayouts.size()) || m_description.colorResolveAttachmentLayouts.empty(),
            "The amounts of color attachment layouts and color resolve attachment layouts need to match!");

        CheckColorAttachmentLayouts();
        CheckColorResolveAttachmentLayouts();
        CheckDepthStencilAttachmentLayout();

        CheckSampleCounts();

        OnInit();
    }

    void RenderPass::Begin(std::shared_ptr<FrameBuffer> frameBuffer)
    {
        m_currentFrameBuffer = frameBuffer;
        OnBegin();
    }

    void RenderPass::End()
    {
        if (m_description.isMultisamplingEnabled)
            m_currentFrameBuffer->Resolve();

        OnEnd();

        m_currentFrameBuffer = nullptr;
    }

    bool RenderPass::isDepthTestingEnabled() const
    {
        return m_description.isDepthTestingEnabled;
    }

    CompareOperation RenderPass::GetDepthCompareOperation() const
    {
        return m_description.depthCompareOperation;
    }

    bool RenderPass::IsMultisamplingEnabled() const
    {
        return m_description.isMultisamplingEnabled;
    }

    bool RenderPass::IsSampleShadingEnabled() const
    {
        return m_description.isSampleShadingEnabled;
    }

    float RenderPass::GetMinSampleShading() const
    {
        return m_description.minSampleShading;
    }

    Texture::SampleCount RenderPass::GetSampleCount() const
    {
        if (m_description.colorAttachmentLayouts.size() > 0)
            return m_description.colorAttachmentLayouts[0].sampleCount;
        return Texture::SampleCount::SAMPLE_1;
    }

    void RenderPass::CheckColorAttachmentLayouts()
    {
        for (size_t i = 0; i < m_description.colorAttachmentLayouts.size(); i++)
        {
            Texture::Format colorAttachmentFormat = m_description.colorAttachmentLayouts[i].format;
            FIREFLY_ASSERT((colorAttachmentFormat != Texture::Format::DEPTH_32_FLOAT) &&
                (colorAttachmentFormat != Texture::Format::DEPTH_24_STENCIL_8),
                "Color attachment layout {0} has no color texture format: {1}", i, Texture::ConvertFormatToString(colorAttachmentFormat));
        }
    }

    void RenderPass::CheckColorResolveAttachmentLayouts()
    {
        for (size_t i = 0; i < m_description.colorResolveAttachmentLayouts.size(); i++)
        {
            Texture::Format colorAttachmentFormat = m_description.colorAttachmentLayouts[i].format;
            Texture::Format colorResolveAttachmentFormat = m_description.colorResolveAttachmentLayouts[i].format;
            FIREFLY_ASSERT(colorAttachmentFormat == colorResolveAttachmentFormat,
                "Color resolve attachment layout {0} has not the same format as color attachment layout {0}. {1} != {2}",
                i, Texture::ConvertFormatToString(colorResolveAttachmentFormat), Texture::ConvertFormatToString(colorAttachmentFormat));
        }
    }

    void RenderPass::CheckDepthStencilAttachmentLayout()
    {
        if (HasDepthStencilAttachmentLayout())
        {
            Texture::Format depthStencilAttachmentFormat = m_description.depthStencilAttachmentLayout.format;
            FIREFLY_ASSERT((depthStencilAttachmentFormat == Texture::Format::DEPTH_32_FLOAT) ||
                (depthStencilAttachmentFormat == Texture::Format::DEPTH_24_STENCIL_8),
                "Depth stencil attachment has no depth/stencil texture format: {0}", Texture::ConvertFormatToString(depthStencilAttachmentFormat));
        }
    }

    void RenderPass::CheckSampleCounts()
    {
        Texture::SampleCount sampleCount = m_description.colorAttachmentLayouts[0].sampleCount;
        for (size_t i = 1; i < m_description.colorAttachmentLayouts.size(); i++)
            FIREFLY_ASSERT(sampleCount == m_description.colorAttachmentLayouts[i].sampleCount, "Color attachment layout {0} has not the same sample count as the other attachment layouts!", i);

        if (HasDepthStencilAttachmentLayout())
            FIREFLY_ASSERT(sampleCount == m_description.depthStencilAttachmentLayout.sampleCount, "Depth stencil attachment layout has not the same sample count as the color attachment layouts!");

        for (size_t i = 0; i < m_description.colorResolveAttachmentLayouts.size(); i++)
            FIREFLY_ASSERT(m_description.colorResolveAttachmentLayouts[i].sampleCount == Texture::SampleCount::SAMPLE_1, "Color resolve attachment layout has multiple samples. Only 1 sample is allowed");
    }

    bool RenderPass::IsFrameBufferCompatible(std::shared_ptr<FrameBuffer> frameBuffer)
    {
        std::vector<AttachmentLayout> renderPassAttachmentLayouts;
        for (auto attachment : m_description.colorAttachmentLayouts)
            renderPassAttachmentLayouts.push_back(attachment);
        for (auto attachment : m_description.colorResolveAttachmentLayouts)
            renderPassAttachmentLayouts.push_back(attachment);
        if (HasDepthStencilAttachmentLayout())
            renderPassAttachmentLayouts.push_back(m_description.depthStencilAttachmentLayout);

        std::vector<FrameBuffer::Attachment> frameBufferAttachments;
        for (auto attachment : frameBuffer->GetColorAttachments())
            frameBufferAttachments.push_back(attachment);
        for (auto attachment : frameBuffer->GetColorResolveAttachments())
            frameBufferAttachments.push_back(attachment);
        if (frameBuffer->HasDepthStencilAttachment())
            frameBufferAttachments.push_back(frameBuffer->GetDepthStencilAttachment());

        if (renderPassAttachmentLayouts.size() != frameBufferAttachments.size())
            return false;

        for (size_t i = 0; i < renderPassAttachmentLayouts.size(); i++)
            if (!IsAttachmentCompatibleWithLayout(frameBufferAttachments[i], renderPassAttachmentLayouts[i]))
                return false;

        return true;
    }

    bool RenderPass::IsAttachmentCompatibleWithLayout(FrameBuffer::Attachment attachment, RenderPass::AttachmentLayout attachmentLayout)
    {
        if (!attachment.texture)
            return false;

        if (attachmentLayout.format != attachment.texture->GetFormat())
            return false;
        if (attachmentLayout.sampleCount != attachment.texture->GetSampleCount())
            return false;

        return true;
    }

    bool RenderPass::HasDepthStencilAttachmentLayout() const
    {
        if (m_description.depthStencilAttachmentLayout.format != Texture::Format::NONE)
            return true;
        return false;
    }

}