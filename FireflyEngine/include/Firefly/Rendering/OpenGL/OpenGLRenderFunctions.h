#pragma once

#include "Rendering/RenderFunctions.h"

namespace Firefly
{
	class OpenGLRenderFunctions : public RenderFunctions
	{
	public:
		virtual ~OpenGLRenderFunctions() override {};

		virtual void Init() override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void DrawIndexed(uint32_t indexCount) override;
	};
}