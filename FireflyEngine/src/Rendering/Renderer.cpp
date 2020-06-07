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
		m_camera = camera;
		m_viewProjectionMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::SubmitDraw(std::shared_ptr<Model> model)
	{
		model->Bind();
		model->GetShader()->SetUniformMatrix4("u_viewProjectionMat", m_viewProjectionMatrix);
		model->GetShader()->SetUniformFloat3("u_cameraPosition", m_camera->GetPosition());

		RenderingAPI::GetRenderFunctions()->DrawIndexed(model->GetMesh()->GetIndexCount());
	}
}