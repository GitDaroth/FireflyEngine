#pragma once

#include "Rendering/RenderFunctions.h"

namespace Firefly
{
	class OpenGLRenderFunctions : public RenderFunctions
	{
	public:
		virtual ~OpenGLRenderFunctions() override {};

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;
		virtual void DrawIndexed(std::shared_ptr<VertexArray> vertexArray) override;
	};
}