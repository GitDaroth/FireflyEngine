#pragma once

#include "Rendering/VertexArray.h"
#include <glm/glm.hpp>

namespace Firefly
{
	class RenderFunctions
	{
	public:
		virtual ~RenderFunctions() {}

		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) = 0;
	};
}