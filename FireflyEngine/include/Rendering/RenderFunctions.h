#pragma once

#include "Rendering/VertexArray.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class RenderFunctions
	{
	public:
		virtual ~RenderFunctions() {}

		virtual void Init() = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) = 0;
	};
}