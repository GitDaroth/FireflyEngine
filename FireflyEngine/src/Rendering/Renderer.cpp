#include "pch.h"
#include "Rendering/Renderer.h"

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::BeginScene()
	{

	}

	void Renderer::EndScene()
	{

	}

	void Renderer::SubmitDraw(std::shared_ptr<VertexArray> vertexArray)
	{
		vertexArray->Bind();
		RenderingAPI::GetRenderFunctions()->DrawIndexed(vertexArray);
	}
}