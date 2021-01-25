#pragma once

#include "Rendering/FrameBuffer.h"

namespace Firefly
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		virtual void Destroy() override;
		virtual void Resolve() override;

		void Bind();

	protected:
		virtual void OnInit() override;

	private:
		uint32_t m_frameBuffer;
		uint32_t m_resolveFrameBuffer;
	};
}