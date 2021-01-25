#pragma once

#include "Rendering/Texture.h"
#include "Rendering/FrameBuffer.h"

namespace Firefly
{
	//enum class CompareOperation
	//{
	//	LESS,
	//	LESS_OR_EQUAL,
	//	EQUAL,
	//	NOT_EQUAL,
	//	GREATER,
	//	GREATER_OR_EQUAL,
	//	ALWAYS,
	//	NEVER
	//};

	//struct FrameBufferAttachmentDescription
	//{
	//	Texture::Format format = Texture::Format::RGBA_8;
	//	Texture::SampleCount sampleCount = Texture::SampleCount::SAMPLE_1;
	//};

	//struct RenderPassDescription
	//{
	//	bool isMultisamplingEnabled = false;
	//	bool isSampleShadingEnabled = false;
	//	float minSampleShading = 0.0f;

	//	bool isDepthTestingEnabled = false;
	//	CompareOperation depthCompareOperation = CompareOperation::LESS;

	//	std::vector<FrameBufferAttachmentDescription> colorAttachmentDescriptions;
	//	std::vector<FrameBufferAttachmentDescription> colorResolveAttachmentDescriptions;
	//	FrameBufferAttachmentDescription depthStencilAttachmentDescription;
	//};

	class RenderPass
	{
	public:
		//void Init(const RenderPassDescription& description);
		//void Destroy();

		//void Begin(std::shared_ptr<FrameBuffer> framebuffer);
		//void End();

	private:
		//RenderPassDescription m_description;
	};
}