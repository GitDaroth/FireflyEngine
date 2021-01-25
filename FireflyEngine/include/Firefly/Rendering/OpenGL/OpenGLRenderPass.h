#pragma once

#include "Rendering/RenderPass.h"
#include <glad/glad.h>

namespace Firefly
{
	class OpenGLRenderPass : public RenderPass
	{
	public:
		virtual void Destroy() override;

		virtual void OnBegin() override;
		virtual void OnEnd() override;

	protected:
		virtual void OnInit() override;

	private:
		GLenum ConvertToOpenGLDepthFunction(CompareOperation compareOperation);
	};
}