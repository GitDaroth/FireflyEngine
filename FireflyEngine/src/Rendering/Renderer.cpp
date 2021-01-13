#include "pch.h"
#include "Rendering/Renderer.h"

namespace Firefly
{
	Renderer::Renderer(std::shared_ptr<GraphicsContext> context) :
		m_context(context)
	{
	}
}