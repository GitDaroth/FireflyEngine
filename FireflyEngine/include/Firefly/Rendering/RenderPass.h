#pragma once

#include "Rendering/FrameBuffer.h"

namespace Firefly
{
    enum class CompareOperation
    {
        LESS,
        LESS_OR_EQUAL,
        EQUAL,
        NOT_EQUAL,
        GREATER,
        GREATER_OR_EQUAL,
        ALWAYS,
        NEVER
    };

    class RenderPass
    {
    public:
        struct AttachmentLayout
        {
            Texture::Format format = Texture::Format::NONE;
            Texture::SampleCount sampleCount = Texture::SampleCount::SAMPLE_1;
        };

        struct Description
        {
            bool isMultisamplingEnabled = false;
            bool isSampleShadingEnabled = false;
            float minSampleShading = 1.0f;

            bool isDepthTestingEnabled = false;
            CompareOperation depthCompareOperation = CompareOperation::LESS;

            std::vector<AttachmentLayout> colorAttachmentLayouts;
            std::vector<AttachmentLayout> colorResolveAttachmentLayouts;
            AttachmentLayout depthStencilAttachmentLayout;
        };

        void Init(const Description& description);
        virtual void Destroy() = 0;

        void Begin(std::shared_ptr<FrameBuffer> frameBuffer);
        void End();

        bool isDepthTestingEnabled() const;
        CompareOperation GetDepthCompareOperation() const;
        bool IsMultisamplingEnabled() const;
        bool IsSampleShadingEnabled() const;
        float GetMinSampleShading() const;
        Texture::SampleCount GetSampleCount() const;

    protected:
        virtual void OnInit() = 0;
        virtual void OnBegin() = 0;
        virtual void OnEnd() = 0;
        bool IsFrameBufferCompatible(std::shared_ptr<FrameBuffer> frameBuffer);
        static bool IsAttachmentCompatibleWithLayout(FrameBuffer::Attachment attachment, AttachmentLayout attachmentLayout);
        bool HasDepthStencilAttachmentLayout() const;

        Description m_description;
        std::shared_ptr<FrameBuffer> m_currentFrameBuffer = nullptr;

    private:
        void CheckColorAttachmentLayouts();
        void CheckColorResolveAttachmentLayouts();
        void CheckDepthStencilAttachmentLayout();
        void CheckSampleCounts();
    };
}