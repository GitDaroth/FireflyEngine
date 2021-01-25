#pragma once

#include "Rendering/Texture.h"
#include <vector>

namespace Firefly
{
	class FrameBuffer
	{
	public:
		struct Attachment
		{
			std::shared_ptr<Texture> texture = nullptr;
			uint32_t mipMapLevel = 0;
			uint32_t arrayLayer = 0;
		};

		struct Description
		{
			uint32_t width = 0;
			uint32_t height = 0;
			std::vector<Attachment> colorAttachments;
			std::vector<Attachment> colorResolveAttachments;
			Attachment depthStencilAttachment;
		};

		void Init(const Description& description);
		virtual void Destroy() = 0;
		virtual void Resolve() = 0;
	
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		std::vector<Attachment> GetColorAttachments();
		std::vector<Attachment> GetColorResolveAttachments();
		Attachment GetDepthStencilAttachment();
		bool HasDepthStencilAttachment() const;

	protected:
		virtual void OnInit() = 0;

		Description m_description;

	private:
		void CheckColorAttachments();
		void CheckColorResolveAttachments();
		void CheckDepthStencilAttachment();
		void CheckAttachment(Attachment& attachment, const std::string& attachmentLabel);
		void CheckSampleCounts();
	};
}