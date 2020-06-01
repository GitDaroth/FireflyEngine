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

	void Renderer::BeginScene(std::shared_ptr<Camera> camera)
	{
		m_viewProjectionMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix();
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::SubmitDraw(std::shared_ptr<Shader> shader, std::shared_ptr<VertexArray> vertexArray, const glm::mat4& modelMatrix)
	{
		shader->Bind();
		shader->SetUniformMatrix4("viewProjectionMat", m_viewProjectionMatrix);
		shader->SetUniformMatrix4("modelMat", modelMatrix);

		vertexArray->Bind();
		RenderingAPI::GetRenderFunctions()->DrawIndexed(vertexArray);
	}
}