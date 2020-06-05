#include "pch.h"
#include "Rendering/Renderer.h"

#include "Rendering/RenderingAPI.h"

namespace Firefly
{
	Renderer::Renderer()
	{
		RenderingAPI::GetRenderFunctions()->Init();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::BeginScene(std::shared_ptr<Camera> camera)
	{
		m_viewProjectionMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::SubmitDraw(std::shared_ptr<Model> model)
	{
		model->Bind();
		model->GetShader()->SetUniformMatrix4("viewProjectionMat", m_viewProjectionMatrix);

		RenderingAPI::GetRenderFunctions()->DrawIndexed(model->GetMesh()->GetIndexCount());
	}
}